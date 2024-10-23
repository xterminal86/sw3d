#include <cstdio>

#include "sw3d.h"
#include "instant-font.h"
#include "scanline-rasterizer.h"

using namespace SW3D;

const size_t QualityReductionFactor = 6;

bool Wireframe = false;

bool ShowTriangle1 = true;
bool ShowTriangle2 = true;
bool ShowTriangle3 = true;
bool ShowTriangle4 = true;

ScanlineRasterizer Rasterizer;

TriangleSimple Triangle1 =
{
  {
    { 20, 20, 0 }, { 100, 20, 0 }, { 60, 60, 0 }
  }
};

TriangleSimple Triangle2 =
{
  {
    { 100, 20, 0 }, { 60, 60, 0 }, { 100, 100, 0 }
  }
};

TriangleSimple Triangle3 =
{
  {
    { 100, 100, 0 }, { 60, 60, 0 }, { 20, 100, 0 }
  }
};

TriangleSimple Triangle4 =
{
  {
    { 20, 20, 0 }, { 60, 60, 0 }, { 20, 100, 0 }
  }
};

// =============================================================================

class TLR : public DrawWrapper
{
  public:

    // -------------------------------------------------------------------------

    void DrawToFrameBuffer() override
    {
      SaveColor();

      if (ShowTriangle1)
      {
        SDL_SetRenderDrawColor(_renderer, 255, 0, 0, 255);
        Rasterizer.Rasterize(Triangle1, Wireframe);
      }

      if (ShowTriangle2)
      {
        SDL_SetRenderDrawColor(_renderer, 0, 255, 0, 255);
        Rasterizer.Rasterize(Triangle2, Wireframe);
      }

      if (ShowTriangle3)
      {
        SDL_SetRenderDrawColor(_renderer, 0, 255, 255, 255);
        Rasterizer.Rasterize(Triangle3, Wireframe);
      }

      if (ShowTriangle4)
      {
        SDL_SetRenderDrawColor(_renderer, 255, 0, 255, 255);
        Rasterizer.Rasterize(Triangle4, Wireframe);
      }

      RestoreColor();
    }

    // -------------------------------------------------------------------------

    void DrawToScreen() override
    {
      IF::Instance().Printf(0, 0,
                            IF::TextParams::Set(0xFFFFFF,
                                                IF::TextAlignment::LEFT,
                                                2.0),
                            "'1', '2', '3', '4' to toggle triangles");

      IF::Instance().Printf(0, 20,
                            IF::TextParams::Set(0xFFFFFF,
                                                IF::TextAlignment::LEFT,
                                                2.0),
                            "'TAB' - toggle wireframe rendering");
    }

    // -------------------------------------------------------------------------

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
              Wireframe = not Wireframe;
              break;

            case SDLK_1:
              ShowTriangle1 = not ShowTriangle1;
              break;

            case SDLK_2:
              ShowTriangle2 = not ShowTriangle2;
              break;

            case SDLK_3:
              ShowTriangle3 = not ShowTriangle3;
              break;

            case SDLK_4:
              ShowTriangle4 = not ShowTriangle4;
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
  TLR c;

  if (c.Init(800, 800, QualityReductionFactor))
  {
    IF::Instance().Init(c.GetRenderer());
    Rasterizer.Init(c.GetRenderer());

    c.Run(true);
  }

  return 0;
}