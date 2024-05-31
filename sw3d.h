#ifndef SW3D_H
#define SW3D_H

#include <string>
#include <vector>
#include <chrono>
#include <cmath>
#include <sstream>
#include <unordered_map>

#include <SDL2/SDL.h>

#define INIT_CHECK()                                           \
  if (not _initialized)                                        \
  {                                                            \
    SDL_Log("Drawer is not initialized - call Init() first!"); \
    return;                                                    \
  }

namespace SW3D
{
  enum class FrontFaceWinding
  {
    CW = 0,
    CCW
  };

  enum class FaceCullMode
  {
    FRONT = 0,
    BACK,
    BOTH
  };

  constexpr double DEG2RAD = M_PI / 180.0;

  using Clock = std::chrono::steady_clock;
  using ns    = std::chrono::nanoseconds;

  // ===========================================================================

  struct Vec2
  {
    double X = 0.0;
    double Y = 0.0;

    void operator*=(double value)
    {
      X *= value;
      Y *= value;
    }

    void operator+=(double value)
    {
      X += value;
      Y += value;
    }

    double Length()
    {
      return std::sqrt(X * X + Y * Y);
    }

    void Normalize()
    {
      double l = Length();

      if (l == 0)
      {
        return;
      }

      X /= l;
      Y /= l;
    }
  };

  // ===========================================================================

  struct Vec3
  {
    double X = 0.0;
    double Y = 0.0;
    double Z = 0.0;

    void operator*=(double value)
    {
      X *= value;
      Y *= value;
      Z *= value;
    }

    void operator+=(double value)
    {
      X += value;
      Y += value;
      Z += value;
    }

    double Length()
    {
      return std::sqrt(X * X + Y * Y + Z * Z);
    }

    void Normalize()
    {
      double l = Length();

      if (l == 0)
      {
        return;
      }

      X /= l;
      Y /= l;
      Z /= l;
    }
  };

  // ===========================================================================

  struct Vec4
  {
    double X = 0.0;
    double Y = 0.0;
    double Z = 0.0;
    double W = 1.0;

    void operator*=(double value)
    {
      X *= value;
      Y *= value;
      Z *= value;
      W *= value;
    }

    void operator+=(double value)
    {
      X += value;
      Y += value;
      Z += value;
      W += value;
    }

    double Length()
    {
      return std::sqrt(X * X + Y * Y + Z * Z + W * W);
    }

    void Normalize()
    {
      double l = Length();

      if (l == 0)
      {
        return;
      }

      X /= l;
      Y /= l;
      Z /= l;
      W /= l;
    }
  };

  namespace Directions
  {
    const Vec3 UP    = {  0.0,  1.0,  0.0 };
    const Vec3 DOWN  = {  0.0, -1.0,  0.0 };
    const Vec3 RIGHT = {  1.0,  0.0,  0.0 };
    const Vec3 LEFT  = { -1.0,  0.0,  0.0 };
    const Vec3 IN    = {  0.0,  0.0, -1.0 };
    const Vec3 OUT   = {  0.0,  0.0,  1.0 };
  }

  // ===========================================================================

  struct Triangle
  {
    Vec3 Points[3];
  };

  // ===========================================================================

  struct Mesh
  {
    std::vector<Triangle> Triangles;
  };

  // ===========================================================================

  using VV = std::vector<std::vector<double>>;

  struct Matrix
  {
    public:
      Matrix() = default;

      // -----------------------------------------------------------------------

      Matrix(const Matrix& copy) :
        _rows(copy._rows), _cols(copy._cols), _matrix(copy._matrix)
      {
      }

      // -----------------------------------------------------------------------

      Matrix(const VV& data)
        : _matrix(data), _rows(data.size())
      {
        _cols = (data.size() != 0) ? data[0].size() : 0;
      }

      // -----------------------------------------------------------------------

      Matrix(uint32_t rows, uint32_t cols) : _rows(rows), _cols(cols)
      {
        _matrix.resize(_rows);

        for (uint32_t i = 0; i < _rows; i++)
        {
          _matrix[i].resize(_cols);
        }

        Clear();
      }

      // -----------------------------------------------------------------------

      void Clear()
      {
        for (uint32_t x = 0; x < _rows; x++)
        {
          for (uint32_t y = 0; y < _cols; y++)
          {
            _matrix[x][y] = 0.0;
          }
        }
      }

      // -----------------------------------------------------------------------

      void Identity()
      {
        if (_rows != _cols)
        {
          return;
        }

        for (uint32_t x = 0; x < _rows; x++)
        {
          for (uint32_t y = 0; y < _cols; y++)
          {
            _matrix[x][y] = (x == y) ? 1.0 : 0.0;
          }
        }
      }

      // -----------------------------------------------------------------------

      std::string ToString()
      {
        size_t maxLength = 0;

        for (uint32_t y = 0; y < _cols; y++)
        {
          maxLength = 0;

          for (uint32_t x = 0; x < _rows; x++)
          {
            ::snprintf(_buf, sizeof(_buf), "%.4f", _matrix[x][y]);

            size_t ln = ::strlen(_buf);
            if (ln > maxLength)
            {
              maxLength = ln;
            }
          }

          _maxColumnLengthByColumn[y] = maxLength;
        }

        _ss.str(std::string());

        for (uint32_t x = 0; x < _rows; x++)
        {
          _ss << "[ ";

          for (uint32_t y = 0; y < _cols; y++)
          {
            ::snprintf(_buf, sizeof(_buf), "%.4f", _matrix[x][y]);

            for (int spaces = 0;
                 spaces < (_maxColumnLengthByColumn[y] - ::strlen(_buf));
                 spaces++)
            {
              _ss << " ";
            }

            _ss << _buf << " ";
          }

          _ss << "]\n";
        }

        return _ss.str();
      }

      // -----------------------------------------------------------------------

      void Print()
      {
        if (_rows == 0 or _cols == 0)
        {
          SDL_Log("<rows = 0, cols = 0>");
        }
        else
        {
          SDL_Log("\n%s\n", ToString().data());
        }
      }

      // -----------------------------------------------------------------------

      const std::vector<double>& operator[](uint32_t row) const
      {
        return _matrix[row];
      }

      // -----------------------------------------------------------------------

      std::vector<double>& operator[](uint32_t row)
      {
        return _matrix[row];
      }

      // -----------------------------------------------------------------------

      const uint32_t& Rows() const
      {
        return _rows;
      }

      // -----------------------------------------------------------------------

      const uint32_t& Columns() const
      {
        return _cols;
      }

      // -----------------------------------------------------------------------

      Matrix operator*(const Matrix& rhs)
      {
        if (_cols != rhs._rows)
        {
          return *this;
        }

        Matrix res(_rows, rhs._cols);

        for (uint32_t x = 0; x < res._rows; x++)
        {
          for (uint32_t y = 0; y < res._cols; y++)
          {
            for (uint32_t z = 0; z < _cols; z++) // or z < rhs._rows
            {
              res[x][y] += (_matrix[x][z] * rhs._matrix[z][y]);
            }
          }
        }

        return res;
      }

      // -----------------------------------------------------------------------

      Vec3 operator*(const Vec3& in)
      {
        Vec3 res;

        if (_cols != 4)
        {
          return res;
        }

        res.X = _matrix[0][0] * in.X +
                _matrix[0][1] * in.Y +
                _matrix[0][2] * in.Z +
                _matrix[0][3];

        res.Y = _matrix[1][0] * in.X +
                _matrix[1][1] * in.Y +
                _matrix[1][2] * in.Z +
                _matrix[1][3];

        res.Z = _matrix[2][0] * in.X +
                _matrix[2][1] * in.Y +
                _matrix[2][2] * in.Z +
                _matrix[2][3];

        double w = _matrix[3][0] * in.X +
                   _matrix[3][1] * in.Y +
                   _matrix[3][2] * in.Z +
                   _matrix[3][3];

        /*
        res.X = _matrix[0][0] * in.X +
                _matrix[1][0] * in.Y +
                _matrix[2][0] * in.Z +
                _matrix[3][0];

        res.Y = _matrix[0][1] * in.X +
                _matrix[1][1] * in.Y +
                _matrix[2][1] * in.Z +
                _matrix[3][1];

        res.Z = _matrix[0][2] * in.X +
                _matrix[1][2] * in.Y +
                _matrix[2][2] * in.Z +
                _matrix[3][2];

        double w = _matrix[0][3] * in.X +
                   _matrix[1][3] * in.Y +
                   _matrix[2][3] * in.Z +
                   _matrix[3][3];
        */

        if (w != 0.0)
        {
          res.X /= w;
          res.Y /= w;
          res.Z /= w;
        }

        return res;
      }

      // -----------------------------------------------------------------------

      Matrix& operator*=(double value)
      {
        for (uint32_t x = 0; x < _rows; x++)
        {
          for (uint32_t y = 0; y < _cols; y++)
          {
            _matrix[x][y] *= value;
          }
        }

        return *this;
      }

      // -----------------------------------------------------------------------

      Matrix operator+(const Matrix& rhs)
      {
        if ( (_rows != rhs._rows) or (_cols != rhs._cols) )
        {
          return *this;
        }

        Matrix res(_rows, _cols);

        for (int x = 0; x < _rows; x++)
        {
          for (int y = 0; y < _cols; y++)
          {
            res[x][y] = _matrix[x][y] + rhs._matrix[x][y];
          }
        }

        return res;
      }

      // -----------------------------------------------------------------------

      void operator=(const Matrix& rhs)
      {
        _matrix = rhs._matrix;
        _rows   = rhs._rows;
        _cols   = rhs._cols;
      }

    private:
      VV _matrix;

      uint32_t _rows;
      uint32_t _cols;

      // -----------------------------------------------------------------------
      //
      // For printf debug logs.
      //
      std::unordered_map<uint32_t, size_t> _maxColumnLengthByColumn;

      char _buf[32];

      std::stringstream _ss;
      // -----------------------------------------------------------------------
  };

  // ===========================================================================

  class DrawWrapper
  {
    public:
      virtual ~DrawWrapper()
      {
        SDL_Quit();
      }

      // -----------------------------------------------------------------------

      bool Init(uint16_t windowWidth,
                uint16_t windowHeight,
                uint32_t frameBufferSize = 100)
      {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
        {
          SDL_Log("SDL_Init() error: %s", SDL_GetError());
          return false;
        }

        SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

        //
        // Extra 1 to account for last screen column in DrawGrid()
        // since we go from 0 to frameBufferSize.
        //
        _frameBufferSize = frameBufferSize + 1;

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
                                   SDL_WINDOW_SHOWN);

        if (_window == nullptr)
        {
          SDL_Log("SDL_CreateWindow() error: %s", SDL_GetError());
          return false;
        }

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

        _initialized = true;

        return true;
      }

      // -----------------------------------------------------------------------

      void Run(bool debugMode = false)
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

          Draw();

          SDL_SetRenderTarget(_renderer, nullptr);
          SDL_RenderCopy(_renderer, _framebuffer, nullptr, nullptr);

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

      // -----------------------------------------------------------------------

      void Stop()
      {
        _running = false;
      }

      // -----------------------------------------------------------------------

      virtual void Draw() = 0;
      virtual void HandleEvent(const SDL_Event& evt) = 0;

      // -----------------------------------------------------------------------

      void DrawPoint(const SDL_Point& p, uint32_t colorMask)
      {
        INIT_CHECK();

        SaveColor();

        if (HasAlpha(colorMask))
        {
          SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
          HTML2RGBA(colorMask);
        }
        else
        {
          SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_NONE);
          HTML2RGB(colorMask);
        }

        SDL_SetRenderDrawColor(_renderer,
                                _drawColor.r,
                                _drawColor.g,
                                _drawColor.b,
                                _drawColor.a);

        SDL_RenderDrawPoint(_renderer, p.x, p.y);

        RestoreColor();
      }

      // -----------------------------------------------------------------------

      void DrawLine(const SDL_Point& p1,
                    const SDL_Point& p2,
                    uint32_t colorMask)
      {
        INIT_CHECK();

        SaveColor();

        if (HasAlpha(colorMask))
        {
          SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
          HTML2RGBA(colorMask);
        }
        else
        {
          SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_NONE);
          HTML2RGB(colorMask);
        }

        SDL_SetRenderDrawColor(_renderer,
                                _drawColor.r,
                                _drawColor.g,
                                _drawColor.b,
                                _drawColor.a);

        SDL_RenderDrawLine(_renderer, p1.x, p1.y, p2.x, p2.y);

        RestoreColor();
      }

      // -----------------------------------------------------------------------

      void DrawTriangle(const SDL_Point& p1,
                        const SDL_Point& p2,
                        const SDL_Point& p3,
                        uint32_t colorMask,
                        bool wireframe = false)
      {
        INIT_CHECK();

        if (wireframe)
        {
          DrawLine(p1, p2, colorMask);
          DrawLine(p2, p3, colorMask);
          DrawLine(p1, p3, colorMask);
        }
        else
        {
          FillTriangle(p1, p2, p3, colorMask);
        }
      }

      // -----------------------------------------------------------------------

      void DrawTriangle(const Vec3& p1,
                        const Vec3& p2,
                        const Vec3& p3,
                        uint32_t colorMask,
                        bool wireframe = false)
      {
        DrawTriangle(SDL_Point{ (int32_t)p1.X, (int32_t)p1.Y },
                     SDL_Point{ (int32_t)p2.X, (int32_t)p2.Y },
                     SDL_Point{ (int32_t)p3.X, (int32_t)p3.Y },
                     colorMask,
                     wireframe);
      }

      // -----------------------------------------------------------------------

      //
      // Simple rasterizer based on point inside triangle test.
      //
      void FillTriangle(const SDL_Point& p1,
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

      // -----------------------------------------------------------------------

      const double& DeltaTime()
      {
        return _deltaTime;
      }

      // -----------------------------------------------------------------------

      const uint32_t& FrameBufferSize()
      {
        return _frameBufferSize;
      }

    // *************************************************************************
    //
    //                               PROTECTED
    //
    // *************************************************************************

    protected:
      std::string _windowName = "DrawService window";

    // *************************************************************************
    //
    //                               PRIVATE
    //
    // *************************************************************************
    private:
      const SDL_Color& HTML2RGB(const uint32_t& colorMask)
      {
        _drawColor.r = (colorMask & _maskR) >> 16;
        _drawColor.g = (colorMask & _maskG) >> 8;
        _drawColor.b = (colorMask & _maskB);
        _drawColor.a = 0xFF;

        return _drawColor;
      }

      // -----------------------------------------------------------------------

      const SDL_Color& HTML2RGBA(const uint32_t& colorMask)
      {
        _drawColor.a = (colorMask & _maskA) >> 24;
        _drawColor.r = (colorMask & _maskR) >> 16;
        _drawColor.g = (colorMask & _maskG) >> 8;
        _drawColor.b = (colorMask & _maskB);

        return _drawColor;
      }

      // -----------------------------------------------------------------------

      int32_t CrossProduct(const SDL_Point& p1,
                           const SDL_Point& p2,
                           const SDL_Point& p)
      {
        static SDL_Point v1, v2;
        v1 = { p2.x - p1.x, p2.y - p1.y };
        v2 = { p.x  - p1.x, p.y  - p1.y };
        return (v1.x * v2.y - v1.y * v2.x);
      }

      // -----------------------------------------------------------------------

      void SaveColor()
      {
        SDL_GetRenderDrawColor(_renderer,
                               &_oldColor.r,
                               &_oldColor.g,
                               &_oldColor.b,
                               &_oldColor.a);
      }

      // -----------------------------------------------------------------------

      void RestoreColor()
      {
        SDL_SetRenderDrawColor(_renderer,
                               _oldColor.r,
                               _oldColor.g,
                               _oldColor.b,
                               _oldColor.a);
      }

      // -----------------------------------------------------------------------

      void DrawGrid()
      {
        SaveColor();

        SDL_SetRenderDrawColor(_renderer, 64, 64, 64, 255);

        for (int x = 0; x <= _windowWidth; x += 10)
        {
          for (int y = 0; y <= _windowHeight; y += 10)
          {
            SDL_RenderDrawPoint(_renderer, x, y);
          }
        }

        RestoreColor();
      }

      // -----------------------------------------------------------------------

      bool HasAlpha(uint32_t colorMask)
      {
        return (colorMask & _maskA) != 0x0;
      }

      // -----------------------------------------------------------------------

      SDL_Renderer* _renderer = nullptr;
      SDL_Window* _window     = nullptr;

      SDL_Texture* _framebuffer = nullptr;

      uint32_t _frameBufferSize = 0;

      uint16_t _windowWidth  = 0;
      uint16_t _windowHeight = 0;

      uint32_t _fps = 0;

      double _deltaTime = 0.0;
      double _dtAcc = 0.0;

      double _aspectRatio = 0.0;

      bool _initialized        = false;
      bool _faceCullingEnabled = true;

      const uint32_t _maskR = 0x00FF0000;
      const uint32_t _maskG = 0x0000FF00;
      const uint32_t _maskB = 0x000000FF;
      const uint32_t _maskA = 0xFF000000;

      SDL_Color _drawColor;
      SDL_Color _oldColor;

      SDL_Rect _rect;

      bool _running = true;

      FrontFaceWinding _frontFaceWinding = FrontFaceWinding::CCW;

      FaceCullMode _faceCullMode = FaceCullMode::BACK;
  };

  // ***************************************************************************
  //
  //                           HELPER FUNCTIONS
  //
  // ***************************************************************************


  Matrix GetProjection(double fov,
                       double aspectRatio,
                       double zNear,
                       double zFar)
  {
    double f = 1.0 / tan( (fov / 2) * DEG2RAD );
    double q = zFar / (zFar - zNear);

    Matrix proj(4, 4);

    proj[0][0] = (aspectRatio > 1.0) ? (f / aspectRatio) : (f * aspectRatio);
    proj[1][1] = f;
    proj[2][2] = q;
    proj[3][2] = -zNear * q;
    proj[2][3] = 1.0;

    return proj;
  }

  // ===========================================================================

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

    double c = std::cos(angleDeg * DEG2RAD);
    double s = std::sin(angleDeg * DEG2RAD);

    double omc = 1.0 - c;

    double xs = p.X * s;
    double ys = p.Y * s;
    double zs = p.Z * s;

    Matrix r(4, 4);

    r[0][0] = x2 * omc + c;
    r[0][1] = x * y * omc - zs;
    r[0][2] = x * z * omc + ys;
    r[0][3] = 0.0;

    r[1][0] = y * x * omc + zs;
    r[1][1] = y2 * omc + c;
    r[1][2] = y * z * omc + xs;
    r[1][3] = 0.0;

    r[2][0] = x * z * omc - ys;
    r[2][1] = y * z * omc + xs;
    r[2][2] = z2 * omc + c;
    r[2][3] = 0.0;

    r[3][0] = 0.0;
    r[3][1] = 0.0;
    r[3][2] = 0.0;
    r[3][3] = 1.0;

    res = r * p;

    return res;
  }
} // namespace sw3d

#endif // SW3D_H
