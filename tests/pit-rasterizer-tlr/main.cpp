#include <cstdio>
#include <unordered_map>

#include "sw3d.h"
#include "instant-font.h"
#include "pit-rasterizer-tlr.h"

using namespace SW3D;

const size_t QualityReductionFactor = 6;

size_t SelectionIndex    = 0;
size_t CurrentPointIndex = 0;

bool Wireframe = false;

const std::vector<PitRasterizerTLR::FillConvention> FillConventions =
{
  PitRasterizerTLR::FillConvention::NONE,
  PitRasterizerTLR::FillConvention::TOP_LEFT,
  PitRasterizerTLR::FillConvention::BOTTOM_RIGHT,
  PitRasterizerTLR::FillConvention::TOP_RIGHT,
  PitRasterizerTLR::FillConvention::BOTTOM_LEFT,
};

const std::unordered_map<PitRasterizerTLR::FillConvention, std::string>
FillConventionNameByType =
{
  { PitRasterizerTLR::FillConvention::NONE,         "NONE"         },
  { PitRasterizerTLR::FillConvention::TOP_LEFT,     "TOP-LEFT"     },
  { PitRasterizerTLR::FillConvention::BOTTOM_RIGHT, "BOTTOM-RIGHT" },
  { PitRasterizerTLR::FillConvention::TOP_RIGHT,    "TOP-RIGHT"    },
  { PitRasterizerTLR::FillConvention::BOTTOM_LEFT,  "BOTTOM-LEFT"  },
};

uint8_t FillConventionIndex = 1;

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

using GroupData = std::pair<TriangleSimple, SDL_Color>;

PitRasterizerTLR Rasterizer;

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
        { 20, 20, 0 }, { 40, 20, 0 }, { 60, 60, 0 }
      }
    },
    { 255, 0, 0, 255 }
  },
  {
    {
      {
        { 40, 20, 0 }, { 60, 20, 0 }, { 60, 60, 0 }
      }
    },
    { 0, 255, 0, 255 }
  },
  {
    {
      {
        { 60, 20, 0 }, { 80, 20, 0 }, { 60, 60, 0 }
      }
    },
    { 0, 128, 128, 255 }
  },
  {
    {
      {
        { 80, 20, 0 }, { 100, 20, 0 }, { 60, 60, 0 }
      }
    },
    { 128, 128, 0, 255 }
  },
};

// -----------------------------------------------------------------------------

std::vector<GroupData> Group5 =
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

std::vector<GroupData> Group6 =
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

std::vector<GroupData> Group7 =
{
  {
    {
      {
        { 20, 20, 0 }, { 60, 20, 0 }, { 40, 40, 0 }
      }
    },
    { 255, 0, 0, 255 }
  },
  {
    {
      {
        { 60, 20, 0 }, { 40, 40, 0 }, { 80, 40, 0 }
      }
    },
    { 0, 255, 0, 255 }
  },
  {
    {
      {
        { 60, 20, 0 }, { 100, 20, 0 }, { 80, 40, 0 }
      }
    },
    { 0, 128, 128, 255 }
  },
  {
    {
      {
        { 20, 20, 0 }, { 40, 40, 0 }, { 20, 60, 0 }
      }
    },
    { 128, 0, 128, 255 }
  },
  {
    {
      {
        { 20, 60, 0 }, { 40, 40, 0 }, { 60, 60, 0 }
      }
    },
    { 128, 128, 0, 255 }
  },
  {
    {
      {
        { 40, 40, 0 }, { 80, 40, 0 }, { 60, 60, 0 }
      }
    },
    { 128, 128, 128, 255 }
  },
  {
    {
      {
        { 80, 40, 0 }, { 60, 60, 0 }, { 100, 60, 0 }
      }
    },
    { 255, 255, 0, 255 }
  },
  {
    {
      {
        { 80, 40, 0 }, { 100, 20, 0 }, { 100, 60, 0 }
      }
    },
    { 0, 0, 255, 255 }
  }
};

// -----------------------------------------------------------------------------

std::vector<GroupData> Group8 =
{
  {
    {
      {
        { 50,      50,      0 },
        { 53.4862, 10.1522, 0 },
        { 89.8487, 53.4862, 0 }
      }
    },
    { 255, 0, 0, 255 }
  },
  {
    {
      {
        { 53.8462, 10.1522, 0 },
        { 93.3340, 13.6384, 0 },
        { 89.8487, 53.4862, 0 }
      }
    },
    { 0, 255, 0, 255 }
  },
};

// -----------------------------------------------------------------------------

size_t GroupIndex = 0;

std::vector<std::vector<GroupData>> Groups =
{
  Group1,
  Group2,
  Group3,
  Group4,
  Group5,
  Group6,
  Group7,
  Group8
};

std::vector<GroupData>* CurrentGroup = &Groups[0];

// =============================================================================

class CTF : public DrawWrapper
{
  public:
    // -------------------------------------------------------------------------

    void DrawToFrameBuffer() override
    {
      SaveColor();

      SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);

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
      PitRasterizerTLR::FillConvention c = FillConventions[FillConventionIndex];


      IF::Instance().Printf(0, 0,
                            IF::TextParams::Set(0xFFFFFF,
                                                IF::TextAlignment::LEFT,
                                                2.0),
                            "%s%s",
                            FillConventionNameByType.at(c).data(),
                            Rasterizer.UseOptimizedVariant
                            ? " (optimized)"
                            : "");
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

            case SDLK_f:
            {
              FillConventionIndex++;
              FillConventionIndex %= FillConventions.size();
              Rasterizer.SetFillConvention(FillConventions[FillConventionIndex]);
            }
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

            case SDLK_o:
              Rasterizer.UseOptimizedVariant = not Rasterizer.UseOptimizedVariant;
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
  CTF c;

  if (c.Init(800, 800, QualityReductionFactor))
  {
    IF::Instance().Init(c.GetRenderer());
    Rasterizer.Init(c.GetRenderer());
    Rasterizer.SetFillConvention(FillConventions[FillConventionIndex]);
    c.Run(true);
  }

  return 0;
}
