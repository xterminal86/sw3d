#include <cstdio>

#include "sw3d.h"
#include "instant-font.h"

using namespace SW3D;

const size_t QualityReductionFactor = 8;

size_t SelectionIndex = 0;

bool SubpixelDrawing = false;

const TriangleSimple FlatTop =
{
  {
    { 50, 50, 0 }, { 100, 50, 0 }, { 75, 100, 0 }
  }
};

const TriangleSimple FlatBottom =
{
  {
    { 75, 0, 0 }, { 50, 50, 0 }, { 100, 50, 0 }
  }
};

const TriangleSimple Composite =
{
  {
    { 50, 25, 0 }, { 75, 75, 0 }, { 150, 100, 0 }
  }
};

std::vector<TriangleSimple> Triangles =
{
  FlatTop,
  FlatBottom,
  Composite
};

TriangleSimple CurrentTriangle = FlatTop;

double dx = 0.0;
double dy = 0.0;

// =============================================================================

class CTF : public DrawWrapper
{
  public:
    void DrawTriangleContour(const TriangleSimple& t)
    {
      if (SubpixelDrawing)
      {
        SDL_RenderDrawLineF(_renderer,
                            t.Points[0].X, t.Points[0].Y,
                            t.Points[1].X, t.Points[1].Y);

        SDL_RenderDrawLineF(_renderer,
                           t.Points[1].X, t.Points[1].Y,
                           t.Points[2].X, t.Points[2].Y);

        SDL_RenderDrawLineF(_renderer,
                           t.Points[2].X, t.Points[2].Y,
                           t.Points[0].X, t.Points[0].Y);
      }
      else
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
    }

    // -------------------------------------------------------------------------

    void FillTriangleC(const TriangleSimple& t)
    {
    }

    // -------------------------------------------------------------------------

    void DrawToFrameBuffer() override
    {
      SaveColor();

      SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);

      SDL_RenderDrawPointF(_renderer, dx, dy);

      SDL_SetRenderDrawColor(_renderer, 255, 0, 0, 255);

      SDL_RenderDrawPoint(_renderer, dx, dy + 6);

      SDL_SetRenderDrawColor(_renderer, 0, 255, 255, 255);

      DrawTriangleContour(CurrentTriangle);

      SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);

      FillTriangleC(CurrentTriangle);

      RestoreColor();

    }

    // -------------------------------------------------------------------------

    void DrawToScreen() override
    {
      IF::Instance().Printf((int)(dx * QualityReductionFactor),
                            std::ceil(dy * QualityReductionFactor) + 20,
                            IF::TextParams::Set(0xFFFFFF,
                                                IF::TextAlignment::LEFT,
                                                2.0),
                            "dx = %.2f dy = %.2f (%d %d)",
                            dx, dy,
                            (int)std::round(dx),
                            (int)std::round(dy));

      IF::Instance().Printf(0, 680,
                            IF::TextParams::Set(0xFFFFFF,
                                                IF::TextAlignment::LEFT,
                                                2.0),
                            "subpixel mode %s",
                            SubpixelDrawing ? "ON" : "OFF");
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

            case SDLK_a:
              dx -= 0.1;
              break;

            case SDLK_d:
              dx += 0.1;
              break;

            case SDLK_w:
              dy -= 0.1;
              break;

            case SDLK_s:
              dy += 0.1;
              break;

            case SDLK_SPACE:
              SubpixelDrawing = not SubpixelDrawing;
              break;

            case SDLK_TAB:
              SelectionIndex++;
              SelectionIndex %= Triangles.size();
              CurrentTriangle = Triangles[SelectionIndex];
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

  if (c.Init(700, 700, QualityReductionFactor))
  {
    IF::Instance().Init(c.GetRenderer());
    c.Run(true);
  }

  return 0;
}
