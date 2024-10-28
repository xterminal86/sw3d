#include <cstdio>

#include "sw3d.h"
#include "instant-font.h"
#include "pit-rasterizer.h"

using namespace SW3D;

const size_t QualityReductionFactor = 4;

size_t SelectionIndex    = 0;
size_t CurrentPointIndex = 0;

bool Wireframe  = false;
bool HideGizmos = false;

const TriangleSimple FlatTop =
{
  {
    { 50, 100, 0 }, { 150, 100, 0 }, { 100, 150, 0 }
  }
};

const TriangleSimple FlatBottom =
{
  {
    { 50, 100, 0 }, { 150, 100, 0 }, { 100, 50, 0 }
  }
};

const TriangleSimple CompositeMR =
{
  {
    { 100, 50, 0 }, { 50, 100, 0 }, { 150, 150, 0 }
  }
};

const TriangleSimple CompositeML =
{
  {
    { 100, 50, 0 }, { 150, 100, 0 }, { 50, 150, 0 }
  }
};

std::vector<TriangleSimple> Triangles =
{
  FlatTop,
  FlatBottom,
  CompositeMR,
  CompositeML
};

TriangleSimple CurrentTriangle = FlatTop;

Vec3* CurrentPoint = &CurrentTriangle.Points[CurrentPointIndex];

PitRasterizer Rasterizer;

// =============================================================================

class CTF : public DrawWrapper
{
  public:
    void HighlightPoint()
    {
      //
      // For some reason SDL_RenderDrawRect() draws like this:
      //
      // |
      // |-----
      // |     |
      // |     |
      // |     |
      //  -----
      //
      SDL_SetRenderDrawColor(_renderer, 0, 255, 255, 255);
      SDL_RenderDrawLine(_renderer,
                         CurrentPoint->X - 5,
                         CurrentPoint->Y - 5,
                         CurrentPoint->X + 5,
                         CurrentPoint->Y - 5);
      SDL_RenderDrawLine(_renderer,
                         CurrentPoint->X + 5,
                         CurrentPoint->Y - 5,
                         CurrentPoint->X + 5,
                         CurrentPoint->Y + 5);
      SDL_RenderDrawLine(_renderer,
                         CurrentPoint->X - 5,
                         CurrentPoint->Y - 5,
                         CurrentPoint->X - 5,
                         CurrentPoint->Y + 5);
      SDL_RenderDrawLine(_renderer,
                         CurrentPoint->X - 5,
                         CurrentPoint->Y + 5,
                         CurrentPoint->X + 5,
                         CurrentPoint->Y + 5);
    }

    // -------------------------------------------------------------------------

    void DrawToFrameBuffer() override
    {
      SaveColor();

      if (not HideGizmos)
      {
        HighlightPoint();
      }

      SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
      Rasterizer.Rasterize(CurrentTriangle, Wireframe);

      RestoreColor();
    }

    // -------------------------------------------------------------------------

    void DrawToScreen() override
    {
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
              CurrentPoint->X--;
              break;

            case SDLK_d:
              CurrentPoint->X++;
              break;

            case SDLK_w:
              CurrentPoint->Y--;
              break;

            case SDLK_s:
              CurrentPoint->Y++;
              break;

            case SDLK_q:
              CurrentPoint->X--;
              CurrentPoint->Y--;
              break;

            case SDLK_e:
              CurrentPoint->X++;
              CurrentPoint->Y--;
              break;

            case SDLK_c:
              CurrentPoint->X++;
              CurrentPoint->Y++;
              break;

            case SDLK_z:
              CurrentPoint->X--;
              CurrentPoint->Y++;
              break;

            case SDLK_f:
              Wireframe = not Wireframe;
              break;

            case SDLK_h:
              HideGizmos = not HideGizmos;
              break;

            case SDLK_TAB:
              SelectionIndex++;
              SelectionIndex %= Triangles.size();
              CurrentTriangle = Triangles[SelectionIndex];
              break;

            case SDLK_1:
              CurrentPointIndex = 0;
              CurrentPoint = &CurrentTriangle.Points[CurrentPointIndex];
              break;

            case SDLK_2:
              CurrentPointIndex = 1;
              CurrentPoint = &CurrentTriangle.Points[CurrentPointIndex];
              break;

            case SDLK_3:
              CurrentPointIndex = 2;
              CurrentPoint = &CurrentTriangle.Points[CurrentPointIndex];
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

  if (c.Init(800, 800, QualityReductionFactor))
  {
    IF::Instance().Init(c.GetRenderer());
    Rasterizer.Init(c.GetRenderer());
    c.Run(true);
  }

  return 0;
}
