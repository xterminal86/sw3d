#include "sw3d.h"

using namespace SW3D;

uint16_t x = 0;
uint16_t y = 0;

class Drawer : public DrawWrapper
{
  public:
    Drawer()
    {
      _windowName = "Software 3D renderer";
    }

    void HandleEvent(const SDL_Event& evt) override
    {
      const uint8_t* kb = SDL_GetKeyboardState(nullptr);
      if (kb[SDL_SCANCODE_ESCAPE])
      {
        Stop();
      }

      if (kb[SDL_SCANCODE_RIGHT])
      {
        x++;
      }

      if (kb[SDL_SCANCODE_LEFT])
      {
        x--;
      }

      if (kb[SDL_SCANCODE_UP])
      {
        y--;
      }

      if (kb[SDL_SCANCODE_DOWN])
      {
        y++;
      }
    }

    void Draw() override
    {
      DrawPoint(10, 10, 0xFFFFFF);
    }
};

int main()
{
  Drawer d;

  d.Init(800, 600, 12);
  d.Run();

  return 0;
}
