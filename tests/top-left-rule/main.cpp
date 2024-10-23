#include <cstdio>

#include "sw3d.h"
#include "instant-font.h"
#include "srtl.h"

using namespace SW3D;

const size_t QualityReductionFactor = 6;

bool Wireframe = false;

bool ShowTriangle1 = true;
bool ShowTriangle2 = true;
bool ShowTriangle3 = true;
bool ShowTriangle4 = true;

SRTL Rasterizer;

using GroupData = std::pair<TriangleSimple, SDL_Color>;

// -----------------------------------------------------------------------------

std::vector<GroupData> Group1 =
{
  {
    {
      {
        { 20, 20, 0 }, { 100, 20, 0 }, { 60, 60, 0 }
      }
    },
    { 255, 0, 0, 255 }
  },
  {
    {
      {
        { 100, 20, 0 }, { 60, 60, 0 }, { 100, 100, 0 }
      }
    },
    { 0, 255, 0, 255 }
  },
  {
    {
      {
        { 100, 100, 0 }, { 60, 60, 0 }, { 20, 100, 0 }
      }
    },
    { 0, 128, 128, 255 }
  },
  {
    {
      {
        { 20, 20, 0 }, { 60, 60, 0 }, { 20, 100, 0 }
      }
    },
    { 128, 0, 128, 255 }
  }
};

// -----------------------------------------------------------------------------

std::vector<GroupData> Group2 =
{
  {
    {
      {
        { 20, 20, 0 }, { 100, 60, 0 }, { 60, 60, 0 }
      }
    },
    { 255, 0, 0, 255 }
  },
  {
    {
      {
        { 20, 20, 0 }, { 100, 60, 0 }, { 120, 60, 0 }
      }
    },
    { 0, 255, 0, 255 }
  },
  {
    {
      {
        { 20, 20, 0 }, { 120, 20, 0 }, { 120, 60, 0 }
      }
    },
    { 0, 128, 128, 255 }
  }
};

// -----------------------------------------------------------------------------

size_t GroupIndex = 0;

std::vector<std::vector<GroupData>> Groups = { Group1, Group2 };

std::vector<GroupData>* CurrentGroup = &Groups[0];

// =============================================================================

class TLR : public DrawWrapper
{
  public:

    // -------------------------------------------------------------------------

    void DrawToFrameBuffer() override
    {
      SaveColor();

      for (size_t i = 0; i < (*CurrentGroup).size(); i++)
      {
        if (i == 0 and not ShowTriangle1) continue;
        if (i == 1 and not ShowTriangle2) continue;
        if (i == 2 and not ShowTriangle3) continue;
        if (i == 3 and not ShowTriangle4) continue;

        SDL_Color& c = (*CurrentGroup)[i].second;
        SDL_SetRenderDrawColor(_renderer, c.r, c.g, c.b, c.a);
        Rasterizer.Rasterize((*CurrentGroup)[i].first, Wireframe);
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

            case SDLK_SPACE:
            {
              GroupIndex++;
              GroupIndex %= Groups.size();
              CurrentGroup = &Groups[GroupIndex];
            }
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
