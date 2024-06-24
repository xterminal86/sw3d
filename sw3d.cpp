#include "sw3d.h"

namespace SW3D
{
  DrawWrapper::~DrawWrapper()
  {
    for (auto& kvp : _texturesByHandle)
    {
      FreeTexture(kvp.first);
    }

    SDL_Quit();
  }

  // ---------------------------------------------------------------------------

  bool DrawWrapper::Init(uint16_t windowWidth,
                         uint16_t windowHeight,
                         uint16_t qualityReductionFactor)
  {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
      SDL_Log("SDL_Init() error: %s", SDL_GetError());
      return false;
    }

    if (qualityReductionFactor == 0)
    {
      SDL_Log("Resolution factor cannot be zero!");
      return false;
    }

    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

    uint16_t canvasSize = std::min(windowWidth, windowHeight);

    //
    // Cannot add extra 1 to account for pretty debug grid size because
    // it will actually downscale texture into screen by 1 pixel and thus
    // introduce artifacts when in full resolution
    // (frameBufferSize == windowWidth == windowHeight).
    //
    _frameBufferSize = canvasSize / qualityReductionFactor;

    if (_frameBufferSize == 0)
    {
      SDL_Log("Canvas size is zero - increase quality!");
      return false;
    }

    _windowWidth  = windowWidth;
    _windowHeight = windowHeight;

    SDL_DisplayMode dm;
    if (SDL_GetCurrentDisplayMode(0, &dm) < 0)
    {
      SDL_Log("SDL_GetCurrentDisplayMode() error: %s", SDL_GetError());
      return false;
    }

    _window = SDL_CreateWindow(_windowName.data(),
                               dm.w / 2 - _windowWidth / 2,
                               dm.h / 2 - _windowHeight / 2,
                               _windowWidth,
                               _windowHeight,
                               SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (_window == nullptr)
    {
      SDL_Log("SDL_CreateWindow() error: %s", SDL_GetError());
      return false;
    }

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
    if (_renderer == nullptr)
    {
      SDL_Log("Failed to create renderer with SDL_RENDERER_ACCELERATED"
              " - falling back to software");

      _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_SOFTWARE);
      if (_renderer == nullptr)
      {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        return false;
      }
    }

    _framebuffer = SDL_CreateTexture(_renderer,
                                     SDL_PIXELFORMAT_RGBA32,
                                     SDL_TEXTUREACCESS_TARGET,
                                     _frameBufferSize,
                                     _frameBufferSize);
    if (_framebuffer == nullptr)
    {
      SDL_Log("Failed to create framebuffer: %s", SDL_GetError());
      return false;
    }

    _aspectRatio = (double)_windowHeight / (double)_windowWidth;

    SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);

    _projectionMatrix.SetIdentity();
    _modelViewMatrix.SetIdentity();

    //
    // Both stacks contain identity matrices OpenGL style, and since identity
    // matrix is basically orthographic projection save corresponding mode as
    // well.
    //
    _projectionStack.push({ _projectionMatrix, ProjectionMode::ORTHOGRAPHIC });
    _modelViewStack.push(_modelViewMatrix);

    _initialized = true;

    PostInit();

    return true;
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::Run(bool debugMode)
  {
    INIT_CHECK();

    SDL_Event evt;

    Clock::time_point measureStart;
    Clock::time_point measureEnd;

    ns dt = ns{0};

    while (_running)
    {
      _drawCalls = 0;

      measureStart = Clock::now();
      measureEnd = measureStart;

      while (SDL_PollEvent(&evt))
      {
        HandleEvent(evt);
      }

      SDL_SetRenderTarget(_renderer, _framebuffer);
      SDL_RenderClear(_renderer);

      if (debugMode)
      {
        DrawGrid();
      }

      DrawToFrameBuffer();

      SDL_SetRenderTarget(_renderer, nullptr);
      SDL_RenderClear(_renderer);

      int ok = SDL_RenderCopy(_renderer, _framebuffer, nullptr, nullptr);
      if (ok < 0)
      {
        SDL_Log("%s", SDL_GetError());
      }

      DrawToScreen();

      SDL_RenderPresent(_renderer);

      _fps++;

      measureEnd = Clock::now();
      dt = measureEnd - measureStart;

      _deltaTime = std::chrono::duration<double>(dt).count();
      _dtAcc += _deltaTime;

      if (_dtAcc > 1.0)
      {
        static char buf[128];

        ::snprintf(buf, sizeof(buf), "FPS: %u", _fps);
        SDL_SetWindowTitle(_window, buf);

        _dtAcc = 0.0;
        _fps = 0;
      }
    }

    SDL_Log("Goodbye!");
  }

  // ---------------------------------------------------------------------------

  int DrawWrapper::LoadTexture(const std::string& fname)
  {
    if (_textureHandleByFname[fname] == 1)
    {
      SDL_Log("Texture '%s' already loaded (handle %d) - reloading",
              fname.data(), _textureHandleByFname[fname]);
      FreeTexture(_textureHandleByFname[fname]);
    }

    SDL_Surface* surf = SDL_LoadBMP(fname.data());
    if (surf == nullptr)
    {
      SDL_Log("SDL_LoadBMP() fail - %s", SDL_GetError());
      return -1;
    }

    //
    // No transparency for now.
    //
    SDL_Texture* tex = SDL_CreateTextureFromSurface(_renderer, surf);
    if (tex == nullptr)
    {
      SDL_FreeSurface(surf);
      SDL_Log("SDL_CreateTextureFromSurface() fail - %s", SDL_GetError());
      return -1;
    }

    _textureHandleCounter++;

    _texturesByHandle[_textureHandleCounter] = { fname, surf, tex };

    _textureHandleByFname[fname] = _textureHandleCounter;

    return _textureHandleCounter;
  }

  // ---------------------------------------------------------------------------

  uint32_t DrawWrapper::ReadTexel(int handle, int x, int y, bool wrap)
  {
    if (_texturesByHandle.count(handle) == 0)
    {
      SDL_Log("Texture handle %d not found!", handle);
      return -1;
    }

    const TextureData& td = _texturesByHandle[handle];

    uint32_t* pix = (uint32_t*)td.Surface->pixels;

    int nx = x;
    int ny = y;

    if (wrap)
    {
      nx %= td.Surface->w;
      ny %= td.Surface->h;
    }

    //
    // Check if it should be (x + y * w) or (y + x * w)
    //
    return pix[nx + (ny * td.Surface->w)];
  }

  // ---------------------------------------------------------------------------

  DrawWrapper::TextureData* DrawWrapper::GetTexture(int handle)
  {
    return (_texturesByHandle.count(handle) == 1)
          ? &_texturesByHandle[handle]
          : nullptr;
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::Stop()
  {
    _running = false;
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::DrawPoint(const SDL_Point &p, uint32_t colorMask)
  {
    INIT_CHECK();

    _drawCalls++;

    SaveColor();

    if (( colorMask & _maskA) != 0)
    {
      SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
    }
    else
    {
      SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_NONE);
    }

    HTML2RGBA(colorMask);

    SDL_SetRenderDrawColor(_renderer,
                            _drawColor.r,
                            _drawColor.g,
                            _drawColor.b,
                            _drawColor.a);

    SDL_RenderDrawPoint(_renderer, p.x, p.y);

    RestoreColor();
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::DrawLine(const SDL_Point& p1,
                             const SDL_Point& p2,
                             uint32_t colorMask)
  {
    INIT_CHECK();

    _drawCalls++;

    SaveColor();

    if (( colorMask & _maskA ) != 0)
    {
      SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
    }
    else
    {
      SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_NONE);
    }

    HTML2RGBA(colorMask);

    SDL_SetRenderDrawColor(_renderer,
                            _drawColor.r,
                            _drawColor.g,
                            _drawColor.b,
                            _drawColor.a);

    SDL_RenderDrawLine(_renderer, p1.x, p1.y, p2.x, p2.y);

    RestoreColor();
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::DrawTriangle(const SDL_Point& p1,
                                 const SDL_Point& p2,
                                 const SDL_Point& p3,
                                 uint32_t colorMask,
                                 RenderMode mode)
  {
    INIT_CHECK();

    switch (mode)
    {
      case RenderMode::SOLID:
        FillTriangle(p1, p2, p3, colorMask);
        break;

      case RenderMode::WIREFRAME:
        DrawLine(p1, p2, colorMask);
        DrawLine(p2, p3, colorMask);
        DrawLine(p1, p3, colorMask);
        break;

      case RenderMode::MIXED:
        FillTriangle(p1, p2, p3, colorMask);
        DrawLine(p1, p2, 0);
        DrawLine(p2, p3, 0);
        DrawLine(p1, p3, 0);
        break;

      default:
        SDL_Log("Unexpected mode %d!", (int)mode);
        break;
    }
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::DrawTriangle(const Vec3& p1,
                                 const Vec3& p2,
                                 const Vec3& p3,
                                 uint32_t colorMask,
                                 RenderMode mode)
  {
    DrawTriangle(SDL_Point{ (int32_t)p1.X, (int32_t)p1.Y },
                 SDL_Point{ (int32_t)p2.X, (int32_t)p2.Y },
                 SDL_Point{ (int32_t)p3.X, (int32_t)p3.Y },
                 colorMask,
                 mode);
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::FillTriangle(const SDL_Point& p1,
                                 const SDL_Point& p2,
                                 const SDL_Point& p3,
                                 uint32_t colorMask)
  {
    INIT_CHECK();

    int xMin = std::min( std::min(p1.x, p2.x), p3.x);
    int yMin = std::min( std::min(p1.y, p2.y), p3.y);
    int xMax = std::max( std::max(p1.x, p2.x), p3.x);
    int yMax = std::max( std::max(p1.y, p2.y), p3.y);

    for (int x = xMin; x <= xMax; x++)
    {
      for (int y = yMin; y <= yMax; y++)
      {
        SDL_Point p = { x, y };

        //
        // This way it works a little bit faster it seems.
        //
        int w0 = (p2.x - p1.x) * (p.y - p1.y) - (p2.y - p1.y) * (p.x - p1.x);
        int w1 = (p3.x - p2.x) * (p.y - p2.y) - (p3.y - p2.y) * (p.x - p2.x);
        int w2 = (p1.x - p3.x) * (p.y - p3.y) - (p1.y - p3.y) * (p.x - p3.x);

        bool inside = (w0 <= 0 and w1 <= 0 and w2 <= 0)
                   or (w0 >= 0 and w1 >= 0 and w2 >= 0);

        if (inside)
        {
          DrawPoint(p, colorMask);
        }
      }
    }
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::SetCullFaceMode(CullFaceMode modeToSet)
  {
    _cullFaceMode = modeToSet;
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::SetMatrixMode(MatrixMode modeToSet)
  {
    _matrixMode = modeToSet;
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::SetRenderMode(RenderMode modeToSet)
  {
    _renderMode = modeToSet;
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::SetShadingMode(ShadingMode modeToSet)
  {
    _shadingMode = modeToSet;
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::PushMatrix()
  {
    switch (_matrixMode)
    {
      case MatrixMode::PROJECTION:
      {
        if (_projectionStack.size() < SW3D::Constants::kMatrixStackLimit)
        {
          _projectionStack.push({ _projectionMatrix, _projectionMode });
        }
        else
        {
          SW3D::Error = EngineError::STACK_OVERFLOW;
        }
      }
      break;

      case MatrixMode::MODELVIEW:
      {
        if (_modelViewStack.size() < SW3D::Constants::kMatrixStackLimit)
        {
          _modelViewStack.push(_modelViewMatrix);
        }
        else
        {
          SW3D::Error = EngineError::STACK_OVERFLOW;
        }
      }
      break;

      default:
        SW3D::Error = EngineError::INVALID_MODE;
        break;
    }
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::PopMatrix()
  {
    switch (_matrixMode)
    {
      case MatrixMode::PROJECTION:
      {
        if (_projectionStack.size() > 1)
        {
          //
          // After we save current projection matrix with PushMatrix() it's
          // placed on top of the stack. So after that any changes to projection
          // will modify current projection matrix. When we're done with it and
          // want to go back to previous projection, it's *on top* of the stack,
          // so we need to assign it to the current projection matrix variable
          // first and pop the stack afterwards.
          //
          _projectionMatrix = _projectionStack.top().first;
          _projectionMode   = _projectionStack.top().second;

          _projectionStack.pop();
        }
        else
        {
          SW3D::Error = EngineError::STACK_UNDERFLOW;
        }
      }
      break;

      case MatrixMode::MODELVIEW:
      {
        //
        // Both stacks contain identity matrix from the start, but because
        // identity modelview matrix corresponds to "no change" it doesn't
        // matter what order to reassign modelview matrix variable is used, but
        // for consistency's sake let's rewrite it as with projection matrix
        // case.
        //
        if (_modelViewStack.size() > 1)
        {
          _modelViewMatrix = _modelViewStack.top();
          _modelViewStack.pop();
        }
        else
        {
          SW3D::Error = EngineError::STACK_UNDERFLOW;
        }
      }
      break;

      default:
        SW3D::Error = EngineError::INVALID_MODE;
        break;
    }
  }

  // ---------------------------------------------------------------------------

  SDL_Renderer* DrawWrapper::GetRenderer() const
  {
    return _renderer;
  }

  // ---------------------------------------------------------------------------

  const double& DrawWrapper::DeltaTime() const
  {
    return _deltaTime;
  }

  // ---------------------------------------------------------------------------

  const double& DrawWrapper::DrawTime() const
  {
    return _drawTime;
  }

  // ---------------------------------------------------------------------------

  const size_t& DrawWrapper::FrameBufferSize() const
  {
    return _frameBufferSize;
  }

  // ---------------------------------------------------------------------------

  const size_t& DrawWrapper::DrawCalls() const
  {
    return _drawCalls;
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::SetWeakPerspective()
  {
    _projectionMatrix = Matrix::WeakPerspective();
    _projectionMode = ProjectionMode::WEAK_PERSPECTIVE;
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::SetPerspective(double fov,
                                   double aspectRatio,
                                   double zNear,
                                   double zFar)
  {
    _projectionMatrix = Matrix::Perspective(fov,
                                            aspectRatio,
                                            zNear,
                                            zFar);
    _projectionMode = ProjectionMode::PERSPECTIVE;
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::SetOrthographic(double left, double right,
                                    double top, double bottom,
                                    double near, double far)
  {
    _projectionMatrix = Matrix::Orthographic(left, right,
                                             top, bottom,
                                             near, far);
    _projectionMode = ProjectionMode::ORTHOGRAPHIC;
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::ApplyShading(const Vec3& lookVector, Triangle& face)
  {
    static Vec3 v1, v2, n, fv;
    static double dp;

    v1 = face.Points[1].Position - face.Points[0].Position;
    v2 = face.Points[2].Position - face.Points[0].Position;
    n  = SW3D::CrossProduct(v1, v2);

    //
    // For backface culling this is not necessary, but for shading it is.
    //
    n.Normalize();

    fv = (_projectionMode == ProjectionMode::ORTHOGRAPHIC)
         ? Vec3::In()
         : face.Points[0].Position; // - camera

    fv.Normalize();

    dp = SW3D::DotProduct(fv, n);

    if (dp < 0.0)
    {
      dp = -dp;
    }

    uint8_t grayscalePart = (uint8_t)(255.0 * dp);

    if (face.ShadingMode_ == ShadingMode::NONE)
    {
      grayscalePart = 255;
    }

    //
    // Everything is one color for now.
    //
    face.Points[0].Color[0] = grayscalePart;
    face.Points[0].Color[1] = grayscalePart;
    face.Points[0].Color[2] = grayscalePart;
    face.Points[0].Color[3] = 255;

    face.Points[1].Color[0] = grayscalePart;
    face.Points[1].Color[1] = grayscalePart;
    face.Points[1].Color[2] = grayscalePart;
    face.Points[1].Color[3] = 255;

    face.Points[2].Color[0] = grayscalePart;
    face.Points[2].Color[1] = grayscalePart;
    face.Points[2].Color[2] = grayscalePart;
    face.Points[2].Color[3] = 255;
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::ShouldCullFace(const Vec3& lookVector, Triangle& face)
  {
    static Vec3 v1, v2, n, fv;
    static double dp;

    v1 = face.Points[1].Position - face.Points[0].Position;
    v2 = face.Points[2].Position - face.Points[0].Position;
    n  = SW3D::CrossProduct(v1, v2);

    //
    // Because in perspective projection we have vanishing point, in order to
    // properly cull faces we must compute dot product between face normal and
    // direction vector from camera to that face.
    // But in orthographic projection there is no vanishing point (or one can
    // think that in this type of projection camera is moved at infinity so all
    // vectors from camera to object are parallel), so our vector from camera to
    // object actually doesn't make sense and thus produces wrong result.
    // So in order to cull faces properly for orthographic projection we must
    // use camera's direction vector towards the object.
    //
    // FIXME: assuming camera is at (0, 0, 0), should be fixed in the future.
    //
    // Can be any point since they're all lying on the same plane.
    //
    fv = (_projectionMode == ProjectionMode::ORTHOGRAPHIC)
         ? Vec3::In()
         : face.Points[0].Position; // - camera

    dp = SW3D::DotProduct(fv, n);

    face.CullFlag = (_cullFaceMode == CullFaceMode::BACK)
                    ? (dp >= 0.0)
                    : (dp < 0.0);
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::Enqueue(const Triangle& t)
  {
    static Triangle tri;

    tri.Points[0].Position = (_modelViewMatrix * t.Points[0].Position);
    tri.Points[1].Position = (_modelViewMatrix * t.Points[1].Position);
    tri.Points[2].Position = (_modelViewMatrix * t.Points[2].Position);

    tri.ShadingMode_ = _shadingMode;
    tri.RenderMode_  = _renderMode;

    ApplyShading(Vec3::Zero(), tri);

    if (_cullFaceMode != CullFaceMode::NONE)
    {
      //
      // Smart people say backface culling should be performed in world space.
      //
      ShouldCullFace(Vec3::Zero(), tri);

      if (not tri.CullFlag)
      {
        //
        // Apply projection only if triangle will be visible.
        //
        for (size_t i = 0; i < 3; i++)
        {
          tri.Points[i].Position = (_projectionMatrix * tri.Points[i].Position);

          tri.Points[i].Position.X += 1;
          tri.Points[i].Position.Y += 1;

          tri.Points[i].Position.X /= 2.0;
          tri.Points[i].Position.Y /= 2.0;

          //
          // Scale into view.
          //
          tri.Points[i].Position.X *= (double)FrameBufferSize();
          tri.Points[i].Position.Y *= (double)FrameBufferSize();
        }

        _pipeline.push_back(tri);
      }
    }
    else
    {
      //
      // Not doing shit.
      //
      tri.CullFlag = false;

      for (size_t i = 0; i < 3; i++)
      {
        tri.Points[i].Position = (_projectionMatrix * tri.Points[i].Position);

        tri.Points[i].Position.X += 1;
        tri.Points[i].Position.Y += 1;

        tri.Points[i].Position.X /= 2.0;
        tri.Points[i].Position.Y /= 2.0;

        tri.Points[i].Position.X *= (double)FrameBufferSize();
        tri.Points[i].Position.Y *= (double)FrameBufferSize();
      }

      _pipeline.push_back(tri);
    }
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::CommenceDraw()
  {
    static Clock::time_point tp;

    tp = Clock::now();

    while (not _pipeline.empty())
    {
      const Triangle& tri = _pipeline.front();

      DrawTriangle(tri.Points[0].Position,
                   tri.Points[1].Position,
                   tri.Points[2].Position,
                   Array2Mask(tri.Points[0].Color),
                   tri.RenderMode_);

      _pipeline.pop_front();
    }

    _drawTime = std::chrono::duration<double>(Clock::now() - tp ).count();
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::FreeTexture(int handle)
  {
    if (_texturesByHandle.count(handle) == 0)
    {
      SDL_Log("Texture handle %d not found!", handle);
      return;
    }

    bool ok = true;

    const TextureData& td = _texturesByHandle[handle];

    SDL_Log("Freeing '%s'...", td.Filename.data());

    if (td.Surface != nullptr)
    {
      SDL_Log("  freeing surface...");
      SDL_FreeSurface(td.Surface);
    }
    else
    {
      ok = false;
      SDL_Log("Surface is nullptr!");
    }

    if (td.Texture != nullptr)
    {
      SDL_Log("  destroying texture...");
      SDL_DestroyTexture(td.Texture);
    }
    else
    {
      ok = false;
      SDL_Log("Texture is nullptr!");
    }

    if (ok)
    {
      SDL_Log("OK");
    }
  }

  // ---------------------------------------------------------------------------

  const SDL_Color& DrawWrapper::HTML2RGBA(const uint32_t& colorMask)
  {
    if (colorMask <= 0xFFFFFF)
    {
      _drawColor.r = (colorMask & _maskR) >> 16;
      _drawColor.g = (colorMask & _maskG) >> 8;
      _drawColor.b = (colorMask & _maskB);
      _drawColor.a = 0xFF;
    }
    else
    {
      _drawColor.a = (colorMask & _maskA) >> 24;
      _drawColor.r = (colorMask & _maskR) >> 16;
      _drawColor.g = (colorMask & _maskG) >> 8;
      _drawColor.b = (colorMask & _maskB);
    }

    return _drawColor;
  }

  // ---------------------------------------------------------------------------

  uint32_t DrawWrapper::Array2Mask(const uint8_t (&color)[4])
  {
    uint32_t res = 0;

    res |= (color[0] << 16);
    res |= (color[1] << 8);
    res |= color[2];
    res |= (color[3] << 24);

    return res;
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::SaveColor()
  {
    SDL_GetRenderDrawColor(_renderer,
                           &_oldColor.r,
                           &_oldColor.g,
                           &_oldColor.b,
                           &_oldColor.a);
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::RestoreColor()
  {
    SDL_SetRenderDrawColor(_renderer,
                           _oldColor.r,
                           _oldColor.g,
                           _oldColor.b,
                           _oldColor.a);
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::DrawGrid()
  {
    SaveColor();

    SDL_SetRenderDrawColor(_renderer, 64, 64, 64, 255);

    for (int x = 0; x <= _frameBufferSize; x += 10)
    {
      for (int y = 0; y <= _frameBufferSize; y += 10)
      {
        SDL_RenderDrawPoint(_renderer, x, y);
      }
    }

    RestoreColor();
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::RotateX(double angle)
  {
    static Matrix r(4, 4);

    r.SetIdentity();

    r[0][0] = 1.0;
    r[0][1] = 0.0;
    r[0][2] = 0.0;

    r[1][0] = 0.0;
    r[1][1] = std::cos(angle * Constants::DEG2RAD);
    r[1][2] = -std::sin(angle * Constants::DEG2RAD);

    r[2][0] = 0.0;
    r[2][1] = std::sin(angle * Constants::DEG2RAD);
    r[2][2] = std::cos(angle * Constants::DEG2RAD);

    _modelViewMatrix = _modelViewMatrix * r;
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::RotateY(double angle)
  {
    static Matrix r(4, 4);

    r.SetIdentity();

    r[0][0] = std::cos(angle * Constants::DEG2RAD);
    r[0][1] = 0.0;
    r[0][2] = std::sin(angle * Constants::DEG2RAD);

    r[1][0] = 0.0;
    r[1][1] = 1.0;
    r[1][2] = 0.0;

    r[2][0] = -std::sin(angle * Constants::DEG2RAD);
    r[2][1] = 0.0;
    r[2][2] = std::cos(angle * Constants::DEG2RAD);

    _modelViewMatrix = _modelViewMatrix * r;
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::RotateZ(double angle)
  {
    static Matrix r(4, 4);

    r.SetIdentity();

    r[0][0] = std::cos(angle * Constants::DEG2RAD);
    r[0][1] = -std::sin(angle * Constants::DEG2RAD);
    r[0][2] = 0.0;

    r[1][0] = std::sin(angle * Constants::DEG2RAD);
    r[1][1] = std::cos(angle * Constants::DEG2RAD);
    r[1][2] = 0.0;

    r[2][0] = 0.0;
    r[2][1] = 0.0;
    r[2][2] = 1.0;

    _modelViewMatrix = _modelViewMatrix * r;
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::Translate(double dx, double dy, double dz)
  {
    static Matrix r(4, 4);

    r.SetIdentity();

    r[3][0] = dx;
    r[3][1] = dy;
    r[3][2] = dz;

    _modelViewMatrix = _modelViewMatrix * r;
  }

  // ***************************************************************************
  //
  //                           HELPER FUNCTIONS
  //
  // ***************************************************************************

  Vec3 RotateX(const Vec3& p, double angle)
  {
    static Matrix r(3, 3);

    r[0][0] = 1.0;
    r[0][1] = 0.0;
    r[0][2] = 0.0;

    r[1][0] = 0.0;
    r[1][1] = std::cos(angle * Constants::DEG2RAD);
    r[1][2] = -std::sin(angle * Constants::DEG2RAD);

    r[2][0] = 0.0;
    r[2][1] = std::sin(angle * Constants::DEG2RAD);
    r[2][2] = std::cos(angle * Constants::DEG2RAD);

    return r * p;
  }

  // ---------------------------------------------------------------------------

  Vec3 RotateY(const Vec3& p, double angle)
  {
    static Matrix r(3, 3);

    r[0][0] = std::cos(angle * Constants::DEG2RAD);
    r[0][1] = 0.0;
    r[0][2] = std::sin(angle * Constants::DEG2RAD);

    r[1][0] = 0.0;
    r[1][1] = 1.0;
    r[1][2] = 0.0;

    r[2][0] = -std::sin(angle * Constants::DEG2RAD);
    r[2][1] = 0.0;
    r[2][2] = std::cos(angle * Constants::DEG2RAD);

    return r * p;
  }

  // ---------------------------------------------------------------------------

  Vec3 RotateZ(const Vec3& p, double angle)
  {
    static Matrix r(3, 3);

    r[0][0] = std::cos(angle * Constants::DEG2RAD);
    r[0][1] = -std::sin(angle * Constants::DEG2RAD);
    r[0][2] = 0.0;

    r[1][0] = std::sin(angle * Constants::DEG2RAD);
    r[1][1] = std::cos(angle * Constants::DEG2RAD);
    r[1][2] = 0.0;

    r[2][0] = 0.0;
    r[2][1] = 0.0;
    r[2][2] = 1.0;

    return r * p;
  }

  // ---------------------------------------------------------------------------

  Vec3 Rotate(const Vec3& p, const Vec3& around, double angleDeg)
  {
    Vec3 res;

    Vec3 n = around;

    n.Normalize();

    double x = p.X;
    double y = p.Y;
    double z = p.Z;

    double x2 = p.X * p.X;
    double y2 = p.Y * p.Y;
    double z2 = p.Z * p.Z;

    double c = std::cos(angleDeg * Constants::DEG2RAD);
    double s = std::sin(angleDeg * Constants::DEG2RAD);

    double omc = 1.0 - c;

    double xs = p.X * s;
    double ys = p.Y * s;
    double zs = p.Z * s;

    Matrix r(3, 3);

    r[0][0] = x2 * omc + c;
    r[0][1] = x * y * omc - zs;
    r[0][2] = x * z * omc + ys;

    r[1][0] = y * x * omc + zs;
    r[1][1] = y2 * omc + c;
    r[1][2] = y * z * omc - xs;

    r[2][0] = x * z * omc - ys;
    r[2][1] = y * z * omc + xs;
    r[2][2] = z2 * omc + c;

    res = r * p;

    return res;
  }

  // ---------------------------------------------------------------------------

  std::string ToString(const Matrix& m)
  {
    static std::stringstream ss;

    ss.str(std::string());

    static std::unordered_map<uint32_t, size_t> maxColumnLengthByColumn;

    maxColumnLengthByColumn.clear();

    size_t maxLength = 0;

    for (uint32_t y = 0; y < m.Columns(); y++)
    {
      maxLength = 0;

      for (uint32_t x = 0; x < m.Rows(); x++)
      {
        std::stringstream lss;

        lss << std::fixed << std::setprecision(4);

        lss << m[x][y];

        size_t ln = lss.str().length();
        if (ln > maxLength)
        {
          maxLength = ln;
        }
      }

      maxColumnLengthByColumn[y] = maxLength;
    }

    ss << std::fixed << std::setprecision(4);

    ss << "\n";

    for (uint32_t x = 0; x < m.Rows(); x++)
    {
      ss << "[ ";

      for (uint32_t y = 0; y < m.Columns(); y++)
      {
        std::stringstream lss;

        lss << std::fixed << std::setprecision(4);

        lss << m[x][y];

        for (uint32_t spaces = 0;
             spaces < (maxColumnLengthByColumn[y] - lss.str().length());
             spaces++)
        {
          ss << " ";
        }

        ss << m[x][y] << " ";
      }

      ss << "]\n";
    }

    return ss.str();
  }

  // ---------------------------------------------------------------------------

  std::string ToString(const Vec2& v)
  {
    static std::stringstream ss;

    ss.str(std::string());

    ss << std::fixed << std::setprecision(4);

    ss << "< " << v.X << ", " << v.Y << " >";

    return ss.str();
  }

  // ---------------------------------------------------------------------------

  std::string ToString(const Vec3& v)
  {
    static std::stringstream ss;

    ss.str(std::string());

    ss << std::fixed << std::setprecision(4);

    ss << "< " << v.X << ", " << v.Y << ", " << v.Z << " >";

    return ss.str();
  }

  // ---------------------------------------------------------------------------

  std::string ToString(const Vec4& v)
  {
    static std::stringstream ss;

    ss.str(std::string());

    ss << std::fixed << std::setprecision(4);

    ss << "< " << v.X << ", " << v.Y << ", " << v.Z << ", " << v.W << " >\n";

    return ss.str();
  }

  // ---------------------------------------------------------------------------

  //
  // Outputs as valid JSON (well, according to online validators anyway).
  //
  std::string ToString(const ModelLoader::Scene& s)
  {
    static std::stringstream ss;

    ss.str(std::string());

    ss << std::fixed << std::setprecision(4);

    ss << "{\n"
       << R"(  "vertices" : [)" << "\n";

    for (size_t i = 0; i < s.Vertices.size(); i++)
    {
      const auto& v = s.Vertices[i];
      ss << "    [ " << v.X << ", " << v.Y << ", " << v.Z << "]";

      if (i != s.Vertices.size() - 1)
      {
        ss << ",";
      }

      ss << "\n";
    }

    ss << "  ],\n";

    ss << R"(  "normals" : [)" << "\n";

    for (size_t i = 0; i < s.Normals.size(); i++)
    {
      const auto& n = s.Normals[i];
      ss << "    [ " << n.X << ", " << n.Y << ", " << n.Z << "]";

      if (i != s.Normals.size() - 1)
      {
        ss << ",";
      }

      ss << "\n";
    }

    ss << "  ],\n";

    ss << R"(  "uv" : [)" << "\n";

    for (size_t i = 0; i < s.UV.size(); i++)
    {
      const auto& uv = s.UV[i];
      ss << "    [ " << uv.X << ", " << uv.Y << "]";

      if (i != s.UV.size() - 1)
      {
        ss << ",";
      }

      ss << "\n";
    }

    ss << "  ],\n";

    ss << R"(  "objects" : [)" << "\n";

    for (size_t i = 0; i < s.Objects.size(); i++)
    {
      const auto& obj = s.Objects[i];

      ss << "    {\n";
      ss << R"(      "name" : ")" << obj.Name << R"(",)" << "\n";
      ss << R"(      "faces" : [)" << "\n";

      for (size_t j = 0; j < obj.Faces.size(); j++)
      {
        const auto& face = obj.Faces[j];

        ss << "        [ ";

        for (int x = 0; x < 3; x++)
        {
          ss << "[ ";

          for (int y = 0; y < 3; y++)
          {
            ss << face.Indices[x][y];

            if (y != 2)
            {
              ss << ", ";
            }
          }

          ss << " ]";

          if (x != 2)
          {
            ss << ", ";
          }
        }

        ss << " ]";

        if (j != obj.Faces.size() - 1)
        {
          ss << ",";
        }

        ss << "\n";
      }

      ss << "      ]\n";

      ss << "    }";

      if (i != s.Objects.size() - 1)
      {
        ss << ",";
      }

      ss << "\n";
    }

    ss << "  ]\n";

    ss << "}\n";

    return ss.str();
  }

  // ---------------------------------------------------------------------------

  Vec3 CrossProduct(const Vec3& v1, const Vec3& v2)
  {
    Vec3 res;

    res.X = (v1.Y * v2.Z - v1.Z * v2.Y);
    res.Y = (v1.Z * v2.X - v1.X * v2.Z);
    res.Z = (v1.X * v2.Y - v1.Y * v2.X);

    return res;
  }

  // ---------------------------------------------------------------------------

  double DotProduct(const Vec3& v1, const Vec3& v2)
  {
    return (v1.X * v2.X + v1.Y * v2.Y + v1.Z * v2.Z);
  }
}
