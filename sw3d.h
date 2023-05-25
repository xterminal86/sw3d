#ifndef SW3D_H
#define SW3D_H

#include <string>
#include <vector>

#include <SDL2/SDL.h>

namespace SW3D
{
  enum class TriangleType
  {
    FLAT_TOP = 0,
    FLAT_BOTTOM,
    COMPOSITE
  };

  struct Vec3
  {
    double X = 0.0;
    double Y = 0.0;
    double Z = 0.0;
  };

  struct Triangle
  {
    Vec3 Points[3];
  };

  struct Mesh
  {
    std::vector<Triangle> Triangles;
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

        _window = SDL_CreateWindow(_windowName.data(),
                                   0,
                                   0,
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

      void Run(bool drawGrid = false)
      {
        if (not _initialized)
        {
          SDL_Log("Renderer is not initialized - call Init() first!");
          return;
        }

        SDL_Event evt;

        while (_running)
        {
          while (SDL_PollEvent(&evt))
          {
            HandleEvent(evt);
          }

          SDL_RenderClear(_renderer);

          if (drawGrid)
          {
            DrawGrid();
          }

          Draw();

          SDL_RenderPresent(_renderer);
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

      void FillTriangle(int x1, int y1,
                        int x2, int y2,
                        int x3, int y3,
                        uint32_t colorMask)
      {
        //
        // For any type of winding one invSlope will be negative, one positive.
        //
        // You can do the same with "classic" slope as well, but then
        // you'll have to scanline vertically instead. I left the original
        // algorithm I found in the Internet, since it's more natural to think
        // of scanlines in terms of a line going from left to right,
        // top to bottom.
        //
        // Since we're controlling y here, we have to find out next X coordinate
        // when Y has increased by one. That's why we use inverted slope.
        // Because k*x is the same as (k + k + k + k) x times, we can see that
        // by solving for x we'll get x = y/k. So we can calculate 1/k
        // beforehand and then just add it in every iteration of the loop.
        // Since k = dy / dx, 1/k = 1 / dy / dx = dx / dy.
        //

        TriangleType tt = GetTriangleType(x1, y1, x2, y2, x3, y3);

        SwapCoords(x1, y1, x2, y2, x3, y3, tt);

        switch (tt)
        {
          case TriangleType::FLAT_BOTTOM:
          {
            DrawFlatBottomTriangle(x1, y1, x2, y2, x3, y3, colorMask);
          }
          break;

          case TriangleType::FLAT_TOP:
          {
            DrawFlatTopTriangle(x1, y1, x2, y2, x3, y3, colorMask);
          }
          break;

          case TriangleType::COMPOSITE:
          {
            int x4;
            int y4;

            if (y3 > y2)
            {
              x4 = x1 + ( (double)(y2 - y1) / (double)(y3 - y1) ) * (x3 - x1);
              y4 = y2;

              DrawFlatBottomTriangle(x1, y1, x2, y2, x4, y4, colorMask);
              DrawFlatTopTriangle(x3, y3, x4, y4, x2, y2, colorMask);
            }
            else if (y2 > y3)
            {
              x4 = x1 - ( (double)(y3 - y1) / (double)(y2 - y1) ) * (x1 - x2);
              y4 = y3;

              DrawFlatBottomTriangle(x1, y1, x4, y4, x3, y3, colorMask);
              DrawFlatTopTriangle(x2, y2, x3, y3, x4, y4, colorMask);
            }
          }
          break;
        }
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

      void DrawFlatBottomTriangle(int x1, int y1,
                                  int x2, int y2,
                                  int x3, int y3,
                                  uint32_t colorMask)
      {
        double invSlope1 = (double)(x2 - x1) / (double)(y2 - y1);
        double invSlope2 = (double)(x3 - x1) / (double)(y3 - y1);

        double curx1 = x1;
        double curx2 = x1;

        for (int scanline = y1; scanline <= y3; scanline++)
        {
          for (int lineX = (int)curx1; lineX <= (int)curx2; lineX++)
          {
            DrawPoint(lineX, scanline, colorMask);
          }

          //
          // Ladder-like movement to left and right respectively,
          // depending on chosen winding, CCW in this project.
          //
          curx1 += invSlope1;
          curx2 += invSlope2;
        }
      }

      // -----------------------------------------------------------------------

      void DrawFlatTopTriangle(int x1, int y1,
                               int x2, int y2,
                               int x3, int y3,
                               uint32_t colorMask)
      {
        double invSlope1 = (double)(x2 - x1) / (double)(y2 - y1);
        double invSlope2 = (double)(x3 - x1) / (double)(y3 - y1);

        double curx1 = x1;
        double curx2 = x1;

        //
        // Bottom-up from lowest point.
        //
        for (int scanline = y1; scanline >= y3; scanline--)
        {
          for (int lineX = (int)curx2; lineX <= (int)curx1; lineX++)
          {
            DrawPoint(lineX, scanline, colorMask);
          }

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
        switch (tt)
        {
          //
          // Proper winding:
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
          // Proper winding:
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

          case TriangleType::COMPOSITE:
          {
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

} // namespace sw3d

#endif // SW3D_H
