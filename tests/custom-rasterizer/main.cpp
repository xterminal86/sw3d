#include <cstdio>

#include "sw3d.h"
#include "instant-font.h"

using namespace SW3D;

size_t SelectionIndex = 0;

float dx = 0.0;

TriangleSimple FlatTop =
{
  {
    { 50, 50, 0 }, { 150, 50, 0 }, { 100, 100, 0 }
  }
};

TriangleSimple FlatBottom =
{
  {
    { 200, 50, 0 }, { 150, 100, 0 }, { 250, 100, 0 }
  }
};

TriangleSimple Composite =
{
  {
    { 100, 150, 0 }, { 150, 250, 0 }, { 300, 300, 0 }
  }
};

// =============================================================================

class CTF : public DrawWrapper
{
  public:
    void DrawTriangleContour(const TriangleSimple& t)
    {
      SDL_RenderDrawLine(_renderer,
                          t.Points[0].X, t.Points[0].Y,
                          t.Points[1].X, t.Points[1].Y);

      SDL_RenderDrawLine(_renderer,
                         t.Points[1].X, t.Points[1].Y,
                         t.Points[2].X, t.Points[2].Y);

      SDL_RenderDrawLine(_renderer,
                         t.Points[2].X, t.Points[2].Y,
                         t.Points[0].X, t.Points[0].Y);
    }

    // -------------------------------------------------------------------------

    void DrawToFrameBuffer() override
    {
      SaveColor();

      if (SelectionIndex == 0)
      {
        SDL_SetRenderDrawColor(_renderer, 0, 255, 255, 255);
      }
      else
      {
        SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
      }

      DrawTriangleContour(FlatTop);

      if (SelectionIndex == 1)
      {
        SDL_SetRenderDrawColor(_renderer, 0, 255, 255, 255);
      }
      else
      {
        SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
      }

      DrawTriangleContour(FlatBottom);

      if (SelectionIndex == 2)
      {
        SDL_SetRenderDrawColor(_renderer, 0, 255, 255, 255);
      }
      else
      {
        SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
      }

      DrawTriangleContour(Composite);

      SDL_RenderDrawPointF(_renderer, dx, 1.0);

      dx += (0.2 * DeltaTime());

      RestoreColor();

    }

    // -------------------------------------------------------------------------

    void DrawToScreen() override
    {
      IF::Instance().Printf(0, 10,
                            IF::TextParams::Set(0xFFFFFF,
                                                IF::TextAlignment::LEFT,
                                                2.0),
                            "dx = %.2f (%d)",
                            dx, (int)std::round(dx));
    }

    void HandleEvent(const SDL_Event& evt) override
    {
      switch (evt.type)
      {
        case SDL_KEYDOWN:
        {
          switch (evt.key.keysym.sym)
          {
            case SDLK_ESCAPE:
              Stop();
              break;

            case SDLK_TAB:
              SelectionIndex++;
              SelectionIndex %= 3;
              break;

            default:
              break;
          }
        }
      }
    }
};

// =============================================================================

int main(int argc, char* argv[])
{
  CTF c;

  if (c.Init(700, 700, 2))
  {
    IF::Instance().Init(c.GetRenderer());
    c.Run(true);
  }

  return 0;
}
