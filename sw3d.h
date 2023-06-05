#ifndef SW3D_H
#define SW3D_H

#include <string>
#include <vector>
#include <chrono>
#include <cmath>
#include <sstream>
#include <map>

#include <SDL2/SDL.h>

namespace SW3D
{
  constexpr double DEG2RAD = std::acos(-1) / 180.0;

  using Clock = std::chrono::high_resolution_clock;
  using ns = std::chrono::nanoseconds;

  enum class TriangleType
  {
    FLAT_TOP = 0,
    FLAT_BOTTOM,
    COMPOSITE
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
  };

  // ===========================================================================

  struct Vec4
  {
    double X = 0.0;
    double Y = 0.0;
    double Z = 0.0;
    double W = 1.0;
  };

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
        _rows = rhs._rows;
        _cols = rhs._cols;
      }

    private:
      VV _matrix;

      uint32_t _rows;
      uint32_t _cols;

      std::map<uint32_t, size_t> _maxColumnLengthByColumn;

      char _buf[32];

      std::stringstream _ss;
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
                uint8_t pixelSize = 1)
      {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
        {
          SDL_Log("SDL_Init() error: %s", SDL_GetError());
          return false;
        }

        SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

        _pixelSize = pixelSize;

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

        _aspectRatio = (double)_windowHeight / (double)_windowWidth;

        SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);

        _initialized = true;

        return true;
      }

      // -----------------------------------------------------------------------

      void Run(bool debugMode = false)
      {
        if (not _initialized)
        {
          SDL_Log("Renderer is not initialized - call Init() first!");
          return;
        }

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

          SDL_RenderClear(_renderer);

          if (debugMode)
          {
            DrawGrid();
            char buf[128];
            ::snprintf(buf, sizeof(buf), "%f", _deltaTime);
            SDL_SetWindowTitle(_window, buf);
          }

          Draw();

          SDL_RenderPresent(_renderer);

          measureEnd = Clock::now();
          dt = measureEnd - measureStart;

          _deltaTime = std::chrono::duration<double>(dt).count();
        }
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

      void DrawPoint(int x, int y, uint32_t colorMask)
      {
        _rect.x = x * _pixelSize;
        _rect.y = y * _pixelSize;
        _rect.w = _pixelSize;
        _rect.h = _pixelSize;

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

        SDL_RenderFillRect(_renderer, &_rect);

        RestoreColor();
      }

      // -----------------------------------------------------------------------

      void DrawLine(int x1, int y1, int x2, int y2, uint32_t colorMask)
      {
        bool steep = fabs(y2 - y1) > fabs(x2 - x1);

        if (steep)
        {
          std::swap(x1, y1);
          std::swap(x2, y2);
        }

        if (x1 > x2)
        {
          std::swap(x1, x2);
          std::swap(y1, y2);
        }

        double dx = x2 - x1;
        double dy = fabs(y2 - y1);

        double error = dx / 2.0;

        int yStep = (y1 < y2) ? 1 : -1;

        int y = y1;

        for (int x = x1; x <= x2; x++)
        {
          if (steep)
          {
            DrawPoint(y, x, colorMask);
          }
          else
          {
            DrawPoint(x, y, colorMask);
          }

          error -= dy;

          if (error < 0)
          {
            y += yStep;
            error += dx;
          }
        }
      }

      // -----------------------------------------------------------------------

      void DrawTriangle(int x1, int y1,
                        int x2, int y2,
                        int x3, int y3,
                        uint32_t colorMask)
      {
        DrawLine(x1, y1, x2, y2, colorMask);
        DrawLine(x2, y2, x3, y3, colorMask);
        DrawLine(x1, y1, x3, y3, colorMask);
      }

      // -----------------------------------------------------------------------

      void FillTriangle(int x1, int y1,
                        int x2, int y2,
                        int x3, int y3,
                        uint32_t colorMask)
      {
        TriangleType tt = GetTriangleType(x1, y1, x2, y2, x3, y3);

        SwapCoords(x1, y1, x2, y2, x3, y3, tt);

        switch (tt)
        {
          case TriangleType::FLAT_BOTTOM:
            FillFlatBottomTriangle(x1, y1, x2, y2, x3, y3, colorMask);
            break;

          case TriangleType::FLAT_TOP:
            FillFlatTopTriangle(x1, y1, x2, y2, x3, y3, colorMask);
            break;

          case TriangleType::COMPOSITE:
          {
            int x4;
            int y4;

            if (y3 > y2)
            {
              //
              // In order to split composite triangle in two we need to find out
              // the x coordinate of the new point that lies on the longest side
              // (it's written as 4 here).
              //
              // x4 is derived from Intercept Theorem:
              //
              // "The ratio of the two segments on the same ray starting at S
              // equals the ratio of the segments on the parallels:"
              //
              // So, given this:
              //
              // +---------------> x
              // |
              // |        1
              // |
              // |     2  z  4
              // |
              // |        w    3
              // ▼
              // y
              //
              // the following holds true:
              //
              // [1 to z]   [z to 4]
              // -------- = --------
              // [1 to w]   [w to 3]
              //
              // which gives us:
              //
              // (y2 - y1)   (x4 - x1)
              // --------- = ---------
              // (y3 - y1)   (x3 - x1)
              //
              //
              // Solving for x4 yields:
              //
              // (y2 - y1)
              // --------- * (x3 - x1) = (x4 - x1)
              // (y3 - y1)
              //
              //           (y2 - y1)
              // x4 = x1 + --------- * (x3 - x1)
              //           (y3 - y1)
              //

              x4 = x1 + ( (double)(y2 - y1) / (double)(y3 - y1) ) * (x3 - x1);
              y4 = y2;

              FillFlatBottomTriangle(x1, y1, x2, y2, x4, y4, colorMask, false);
              FillFlatTopTriangle(x3, y3, x4, y4, x2, y2, colorMask);
            }
            else if (y2 > y3)
            {
              //
              // For mirror-image situation the process is the same.
              //
              // +------------> x
              // |
              // |        1
              // |
              // |     4  z  3
              // |
              // |   2    w
              // |
              // ▼
              // y
              //
              // [1 to z]   [z to 4]
              // -------- = --------
              // [1 to w]   [w to 3]
              //
              // (y3 - y1)   (x1 - x4)
              // --------- = ---------
              // (y2 - y1)   (x1 - x2)
              //
              //             (y3 - y1)
              // (x1 - x2) * --------- = (x1 - x4)
              //             (y2 - y1)
              //
              //           (y3 - y1)
              // x4 = x1 - --------- * (x1 - x2)
              //           (y2 - y1)
              //

              x4 = x1 - ( (double)(y3 - y1) / (double)(y2 - y1) ) * (x1 - x2);
              y4 = y3;

              FillFlatBottomTriangle(x1, y1, x4, y4, x3, y3, colorMask, false);
              FillFlatTopTriangle(x2, y2, x3, y3, x4, y4, colorMask);
            }
          }
          break;
        }
      }

      // -----------------------------------------------------------------------

      const double& DeltaTime()
      {
        return _deltaTime;
      }

      // -----------------------------------------------------------------------

      const uint8_t& PixelSize()
      {
        return _pixelSize;
      }

    protected:
      std::string _windowName = "DrawService window";

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
        if (_pixelSize < 4)
        {
          return;
        }

        SaveColor();

        SDL_SetRenderDrawColor(_renderer, 128, 128, 128, 255);

        for (int x = 0; x < _windowWidth; x += _pixelSize)
        {
          for (int y = 0; y < _windowHeight; y += _pixelSize)
          {
            SDL_RenderDrawPoint(_renderer, x, y);
          }
        }

        RestoreColor();
      }

      // -----------------------------------------------------------------------

      void FillFlatBottomTriangle(int x1, int y1,
                                  int x2, int y2,
                                  int x3, int y3,
                                  uint32_t colorMask,
                                  bool includeLastPoint = true)
      {
        //
        // For any type of winding one invSlope will be negative, one positive.
        //
        // You can do the same with "classic" slope as well, but then
        // you'll have to scanline vertically instead. I left the original
        // algorithm I found in the Internet, since it's more natural to think
        // of scanlines in terms of a line going from left to right,
        // top to bottom, like in CRT television.
        //
        // Since we're controlling y here, we have to find out next X coordinate
        // when Y has increased by one. That's why we use inverted slope.
        // Because k*x is the same as (k + k + k + k) x times, we can see that
        // by solving for x we'll get x = y/k. So we can calculate 1/k
        // beforehand and then just add it in every iteration of the loop.
        //
        // Since:
        //
        // y
        //
        // ^
        // |     * p2
        // |    /
        // |   /
        // |  /
        // | * p1
        // |
        // +----------------------> x
        //
        // p1: (x1, y1), p2: (x2, y2)
        //
        // dy = (y2 - y1)
        // dx = (x2 - x1)
        //
        //      dy      1     1       1     dx
        // k = ----,   --- = ----,   --- = ----
        //      dx      k     dy      k     dy
        //                   ----
        //                    dx
        //
        // In case of our triangle on the screen:
        //
        // +-----------------------> x
        // |
        // |
        // |            p1 (x1, y1)
        // |            / \
        // |           /   \
        // |          /     \
        // | (x2, y2)p2-----p3 (x3, y3)
        // |
        // |
        // ▼
        // y
        //
        // It's easy to see, that one invSlope will be negative, thus no need
        // to bother with + or - during increment, it will be handled
        // automatically in += depending on the sign of the term.
        //
        double invSlope1 = (double)(x2 - x1) / (double)(y2 - y1);
        double invSlope2 = (double)(x3 - x1) / (double)(y3 - y1);

        double curx1 = x1;
        double curx2 = x1;

        for (int scanline = y1;
             (includeLastPoint ? scanline <= y3 : scanline < y3);
             scanline++)
        {
          for (int lineX = (int)curx1; lineX <= (int)curx2; lineX++)
          {
            DrawPoint(lineX, scanline, colorMask);
          }

          curx1 += invSlope1;
          curx2 += invSlope2;
        }
      }

      // -----------------------------------------------------------------------

      void FillFlatTopTriangle(int x1, int y1,
                               int x2, int y2,
                               int x3, int y3,
                               uint32_t colorMask,
                               bool includeLastPoint = true)
      {
        //
        // The idea is exactly the same as above,
        // just in this case we go from bottom to top.
        //
        double invSlope1 = (double)(x2 - x1) / (double)(y2 - y1);
        double invSlope2 = (double)(x3 - x1) / (double)(y3 - y1);

        double curx1 = x1;
        double curx2 = x1;

        //
        // Bottom-up from lowest point.
        //
        for (int scanline = y1;
             (includeLastPoint ? scanline >= y3 : scanline > y3);
             scanline--)
        {
          for (int lineX = (int)curx2; lineX <= (int)curx1; lineX++)
          {
            DrawPoint(lineX, scanline, colorMask);
          }

          //
          // And since we go up, we need to decrement y.
          // Again, the sign will be handled automatically since this is
          // basically the mirror image operation of the flat bottom triangle.
          //
          curx1 -= invSlope1;
          curx2 -= invSlope2;
        }
      }

      // -----------------------------------------------------------------------

      bool HasAlpha(uint32_t colorMask)
      {
        return (colorMask & _maskA) != 0x0;
      }

      // -----------------------------------------------------------------------

      TriangleType GetTriangleType(int x1, int y1,
                                   int x2, int y2,
                                   int x3, int y3)
      {
        //
        // There are 3 cases of 2D triangles.
        //
        // BASIC:
        // ------
        //
        // 1) Flat-top
        //
        //       1
        //
        //     2   3
        //
        // 2) Flat-bottom:
        //
        //     3   2
        //
        //       1
        //
        // SPECIAL:
        // --------
        //
        // 3) Composite, which comes in two forms:
        //
        //       1              1
        //
        //     2        ->    2   3
        //                      +
        //          3         3     2
        //
        //                            1
        //
        // Its mirror image:
        //
        //     1                1
        //
        //        3     ->    2   3
        //                      +
        //  2               3     2
        //
        //
        //                1
        //
        //
        // and which can be split into two basic ones.
        //
        bool isFlatBottom = ( (y2 == y3) and (y1 < y2) ) or
                            ( (y1 == y2) and (y3 < y2) ) or
                            ( (y3 == y1) and (y2 < y3) );

        bool isFlatTop = ( (y1 == y3) and (y2 > y1) ) or
                         ( (y3 == y2) and (y1 > y3) ) or
                         ( (y2 == y1) and (y3 > y2) );

        return isFlatTop ? TriangleType::FLAT_TOP
                         : isFlatBottom ? TriangleType::FLAT_BOTTOM
                         : TriangleType::COMPOSITE;
      }

      // -----------------------------------------------------------------------

      void SwapCoords(int& x1, int& y1,
                      int& x2, int& y2,
                      int& x3, int& y3,
                      TriangleType tt)
      {
        //
        // There are two main methods of specifying the order of vertices:
        // ClockWise (CW) and Counter ClockWise (CCW). Its main purpose
        // is to determine which faces are considered "front" faces,
        // i.e. the ones that should be drawn.
        //
        // For a simple drawing of a 2D filled triangle on the screen
        // winding doesn't matter, but it will matter if you plan to do
        // shading and back face culling for example.
        // At design phase you can choose CW or CCW winding order,
        // it won't make a difference for the end result, as long as you're
        // consistent with it.
        //
        // We'll be using CCW in this project.
        //
        // So we'll be assuming that the user is responsible for supplying
        // vertices in the correct winding order for the object.
        // But it may be possible to specify points in the correct winding order
        // but different relative order. Our rasterization algorithms assume
        // that triangle points have *specific* relative order, so if user
        // specified points in the wrong relative order, rasterization will fail.
        // So we need to account for that by swapping the corresponding coordinates.
        // Finding out if relative order of points was wrong is just a matter of
        // bruteforcing the coordinates' location against one another.
        //
        switch (tt)
        {
          //
          // Proper order:
          //
          //       1
          //
          //     2   3
          //
          case TriangleType::FLAT_BOTTOM:
          {
            //
            //     3          1
            //          ->
            //   1   2      2   3
            //
            if ( (x1 < x2) and (y1 > y3) )
            {
              std::swap(x1, x2);
              std::swap(y1, y2);
              std::swap(x1, x3);
              std::swap(y1, y3);
            }
            //
            //     2          1
            //          ->
            //   3   1      2   3
            //
            else if ( (x3 < x1) and (y3 > y2) )
            {
              std::swap(x1, x3);
              std::swap(y1, y3);
              std::swap(x1, x2);
              std::swap(y1, y2);
            }
          }
          break;

          //
          // Proper order:
          //
          //    3   2
          //
          //      1
          //
          case TriangleType::FLAT_TOP:
          {
            //
            //   2   1     3   2
            //          ->
            //     3         1
            //
            if ( (x1 > x2) and (y1 < y3) )
            {
              std::swap(x1, x2);
              std::swap(y1, y2);
              std::swap(x1, x3);
              std::swap(y1, y3);
            }
            //
            //   1   3     3   2
            //          ->
            //     2         1
            //
            else if ( (x3 > x1) and (y2 > y3) )
            {
              std::swap(x1, x2);
              std::swap(y1, y2);
              std::swap(x2, x3);
              std::swap(y2, y3);
            }
          }
          break;

          //
          // Since we'll be splitting this triangle in two later on there is
          // no proper relative order for this one, but we can prepare certain
          // relative order beforehand just to make everything sort of standard.
          // So when we'll be splitting this in two later we can be sure that
          // point 1 is always at the top, point 2 is to the left and point 3 is
          // to the right.
          //
          case TriangleType::COMPOSITE:
          {
            //
            //
            //      3           1
            //
            //    1      ->   2
            //
            //         2           3
            //
            if ( (y1 > y3) and (y1 < y2) )
            {
              std::swap(x1, x3);
              std::swap(y1, y3);
              std::swap(x2, x3);
              std::swap(y2, y3);
            }
            //
            //      2           1
            //
            //    3      ->   2
            //
            //         1           3
            //
            else if ( (y3 > y2) and (y3 < y1) )
            {
              std::swap(x1, x2);
              std::swap(y1, y2);
              std::swap(x2, x3);
              std::swap(y2, y3);
            }
            //
            //      3           1
            //
            //        2  ->       3
            //
            //   1           2
            //
            else if ( (y1 > y2) and (y2 > y3) )
            {
              std::swap(x1, x3);
              std::swap(y1, y3);
              std::swap(x2, x3);
              std::swap(y2, y3);
            }
            //
            //      2           1
            //
            //        1  ->       3
            //
            //   3           2
            //
            else if ( (y3 > y1) and (y1 > y2) )
            {
              std::swap(x1, x2);
              std::swap(y1, y2);
              std::swap(x2, x3);
              std::swap(y2, y3);
            }
          }
          break;
        }
      }

      // -----------------------------------------------------------------------

      SDL_Renderer* _renderer = nullptr;
      SDL_Window* _window     = nullptr;

      uint8_t _pixelSize = 1;

      uint16_t _windowWidth  = 0;
      uint16_t _windowHeight = 0;

      double _deltaTime = 0.0;

      double _aspectRatio = 0.0;

      bool _initialized = false;

      const uint32_t _maskR = 0x00FF0000;
      const uint32_t _maskG = 0x0000FF00;
      const uint32_t _maskB = 0x000000FF;
      const uint32_t _maskA = 0xFF000000;

      SDL_Color _drawColor;
      SDL_Color _oldColor;

      SDL_Rect _rect;

      bool _running = true;
  };

  // ===========================================================================

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

} // namespace sw3d

#endif // SW3D_H
