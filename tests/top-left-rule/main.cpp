#include <cstdio>

#include "sw3d.h"
#include "instant-font.h"
#include "srtl.h"

using namespace SW3D;

const size_t QualityReductionFactor = 6;

bool Wireframe = false;

std::vector<bool> ShowTriangle =
{
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
};

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

std::vector<GroupData> Group3 =
{
  {
    {
      {
        { 60, 20, 0 }, { 20, 60, 0 }, { 40, 60, 0 }
      }
    },
    { 255, 0, 0, 255 }
  },
  {
    {
      {
        { 60, 20, 0 }, { 40, 60, 0 }, { 80, 60, 0 }
      }
    },
    { 0, 255, 0, 255 }
  },
  {
    {
      {
        { 60, 20, 0 }, { 80, 60, 0 }, { 120, 60, 0 }
      }
    },
    { 0, 128, 128, 255 }
  }
};

// -----------------------------------------------------------------------------

std::vector<GroupData> Group4 =
{
  {
    {
      {
        { 10, 10, 0 }, { 140, 60, 0 }, { 130, 60, 0 }
      }
    },
    { 255, 0, 0, 255 }
  },
  {
    {
      {
        { 10, 10, 0 }, { 130, 60, 0 }, { 120, 60, 0 }
      }
    },
    { 0, 255, 0, 255 }
  },
  {
    {
      {
        { 10, 10, 0 }, { 120, 60, 0 }, { 110, 60, 0 }
      }
    },
    { 0, 128, 128, 255 }
  },
  {
    {
      {
        { 10, 10, 0 }, { 110, 60, 0 }, { 100, 60, 0 }
      }
    },
    { 128, 0, 128, 255 }
  },
  {
    {
      {
        { 10, 10, 0 }, { 100, 60, 0 }, { 90, 60, 0 }
      }
    },
    { 128, 128, 0, 255 }
  },
  {
    {
      {
        { 10, 10, 0 }, { 90, 60, 0 }, { 80, 60, 0 }
      }
    },
    { 128, 0, 0, 255 }
  },
  {
    {
      {
        { 10, 10, 0 }, { 80, 60, 0 }, { 70, 60, 0 }
      }
    },
    { 0, 128, 0, 255 }
  },
  {
    {
      {
        { 10, 10, 0 }, { 70, 60, 0 }, { 60, 60, 0 }
      }
    },
    { 0, 0, 128, 255 }
  }
};

// -----------------------------------------------------------------------------

std::vector<GroupData> Group5 =
{
  {
    {
      {
        { 10, 60, 0 }, { 120, 10, 0 }, { 110, 10, 0 }
      }
    },
    { 255, 0, 0, 255 }
  },
  {
    {
      {
        { 10, 60, 0 }, { 110, 10, 0 }, { 100, 10, 0 }
      }
    },
    { 0, 255, 0, 255 }
  },
  {
    {
      {
        { 10, 60, 0 }, { 100, 10, 0 }, { 90, 10, 0 }
      }
    },
    { 0, 128, 128, 255 }
  },
  {
    {
      {
        { 10, 60, 0 }, { 90, 10, 0 }, { 80, 10, 0 }
      }
    },
    { 128, 0, 128, 255 }
  },
  {
    {
      {
        { 10, 60, 0 }, { 80, 10, 0 }, { 70, 10, 0 }
      }
    },
    { 128, 128, 0, 255 }
  },
  {
    {
      {
        { 10, 60, 0 }, { 70, 10, 0 }, { 60, 10, 0 }
      }
    },
    { 128, 0, 0, 255 }
  },
  {
    {
      {
        { 10, 60, 0 }, { 60, 10, 0 }, { 50, 10, 0 }
      }
    },
    { 0, 128, 0, 255 }
  },
  {
    {
      {
        { 10, 60, 0 }, { 50, 10, 0 }, { 40, 10, 0 }
      }
    },
    { 0, 0, 128, 255 }
  }
};

// -----------------------------------------------------------------------------

size_t GroupIndex = 0;

std::vector<std::vector<GroupData>> Groups =
{
  Group1,
  Group2,
  Group3,
  Group4,
  Group5
};

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
        if (i >= 0 and i < ShowTriangle.size() and ShowTriangle[i] == false)
        {
          continue;
        }

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
                            "'1-9' to toggle triangles");

      IF::Instance().Printf(0, 20,
                            IF::TextParams::Set(0xFFFFFF,
                                                IF::TextAlignment::LEFT,
                                                2.0),
                            "'TAB' - toggle wireframe rendering");

      int cnt = 0;
      for (bool shown : ShowTriangle)
      {
        IF::Instance().Printf(cnt * (QualityReductionFactor + 20), _windowHeight - 20,
                              IF::TextParams::Set(shown ? 0x00FF00 : 0xFF0000,
                                                  IF::TextAlignment::LEFT,
                                                  2.0),
                              "%d",
                              cnt + 1);
        cnt++;
      }
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
              ShowTriangle[0] = not ShowTriangle[0];
              break;

            case SDLK_2:
              ShowTriangle[1] = not ShowTriangle[1];
              break;

            case SDLK_3:
              ShowTriangle[2] = not ShowTriangle[2];
              break;

            case SDLK_4:
              ShowTriangle[3] = not ShowTriangle[3];
              break;

            case SDLK_5:
              ShowTriangle[4] = not ShowTriangle[4];
              break;

            case SDLK_6:
              ShowTriangle[5] = not ShowTriangle[5];
              break;

            case SDLK_7:
              ShowTriangle[6] = not ShowTriangle[6];
              break;

            case SDLK_8:
              ShowTriangle[7] = not ShowTriangle[7];
              break;

            case SDLK_9:
              ShowTriangle[8] = not ShowTriangle[8];
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
