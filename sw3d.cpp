#include "sw3d.h"

namespace SW3D
{
  DrawWrapper::~DrawWrapper()
  {
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

    _projectionStack.push(_projectionMatrix);
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

  void DrawWrapper::Stop()
  {
    _running = false;
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::DrawPoint(const SDL_Point &p, uint32_t colorMask)
  {
    INIT_CHECK();

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

        int w0 = CrossProduct(p1, p2, p);
        int w1 = CrossProduct(p2, p3, p);
        int w2 = CrossProduct(p3, p1, p);

        //
        // TODO: add user configurable front face vertex winding order into
        // account.
        //
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

  void DrawWrapper::SetMatrixMode(MatrixMode modeToSet)
  {
    _matrixMode = modeToSet;
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
          _projectionStack.push(_projectionMatrix);
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
          _projectionStack.pop();
        }
        else
        {
          SW3D::Error = EngineError::STACK_UNDERFLOW;
        }

        _projectionMatrix = _projectionStack.top();
      }
      break;

      case MatrixMode::MODELVIEW:
      {
        if (_modelViewStack.size() > 1)
        {
          _modelViewStack.pop();
        }
        else
        {
          SW3D::Error = EngineError::STACK_UNDERFLOW;
        }

        _modelViewMatrix = _modelViewStack.top();
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

  const uint32_t& DrawWrapper::FrameBufferSize() const
  {
    return _frameBufferSize;
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::SetWeakPerspective()
  {
    _projectionMatrix = Matrix::WeakPerspective();
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
  }

  // ---------------------------------------------------------------------------

  void DrawWrapper::SetOrthographic(double left, double right,
                                    double top, double bottom,
                                    double near, double far)
  {
    _projectionMatrix = Matrix::Orthographic(left, right,
                                             top, bottom,
                                             near, far);
  }

  // ---------------------------------------------------------------------------

  bool DrawWrapper::ShouldCullFace(const Vec3& lookVector,
                                    Triangle& face)
  {
    static Vec3 v1, v2, n;
    static double dp;

    v1 = face.Points[1] - face.Points[0];
    v2 = face.Points[2] - face.Points[0];
    n  = SW3D::CrossProduct(v1, v2);
    dp = SW3D::DotProduct(lookVector, n);

    return (dp > 0.0);
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

  int32_t DrawWrapper::CrossProduct(const SDL_Point& p1,
                                    const SDL_Point& p2,
                                    const SDL_Point& p)
  {
    static SDL_Point v1, v2;
    v1 = { p2.x - p1.x, p2.y - p1.y };
    v2 = { p.x  - p1.x, p.y  - p1.y };
    return (v1.x * v2.y - v1.y * v2.x);
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
