#ifndef SW3D_H
#define SW3D_H

#include <string>

#include <SDL2/SDL.h>

namespace SW3D
{
  class DrawWrapper
  {
    public:
      virtual ~DrawWrapper()
      {
        SDL_Quit();
      }

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

        SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);

        _initialized = true;

        return true;
      }

      void Run()
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

          Draw();

          SDL_RenderPresent(_renderer);
        }
      }

      void Stop()
      {
        _running = false;
      }

      virtual void Draw() = 0;
      virtual void HandleEvent(const SDL_Event& evt) = 0;

      void DrawPoint(uint16_t x, uint16_t y, uint32_t colorMask, bool useAlpha = false)
      {
        _rect.x = x * _pixelSize;
        _rect.y = y * _pixelSize;
        _rect.w = _pixelSize;
        _rect.h = _pixelSize;

        SaveColor();

        if (useAlpha)
        {
          HTML2RGBA(colorMask);
        }
        else
        {
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

      const SDL_Color& HTML2RGBA(const uint32_t& colorMask)
      {
        _drawColor.a = (colorMask & _maskA) >> 24;
        _drawColor.r = (colorMask & _maskR) >> 16;
        _drawColor.g = (colorMask & _maskG) >> 8;
        _drawColor.b = (colorMask & _maskB);

        return _drawColor;
      }

      void SaveColor()
      {
        SDL_GetRenderDrawColor(_renderer,
                               &_oldColor.r,
                               &_oldColor.g,
                               &_oldColor.b,
                               &_oldColor.a);
      }

      void RestoreColor()
      {
        SDL_SetRenderDrawColor(_renderer,
                               _oldColor.r,
                               _oldColor.g,
                               _oldColor.b,
                               _oldColor.a);
      }

      SDL_Renderer* _renderer = nullptr;
      SDL_Window* _window     = nullptr;

      uint8_t _pixelSize = 1;

      uint16_t _windowWidth  = 0;
      uint16_t _windowHeight = 0;

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
