#include <cstdio>

#include "sw3d.h"
#include "instant-font.h"

using namespace SW3D;

const size_t QualityReductionFactor = 16;

double PosX = 20.0;
double PosY = 20.0;

double ToAdd = 0.01;

// =============================================================================

class CTF : public DrawWrapper
{
  public:
    // -------------------------------------------------------------------------

    void DrawToFrameBuffer() override
    {
      SaveColor();

      SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
      SDL_RenderDrawPoint(_renderer, PosX, PosY);

      SDL_SetRenderDrawColor(_renderer, 255, 0, 255, 255);
      SDL_RenderDrawPointF(_renderer, PosX, PosY + 10.0);

      RestoreColor();
    }

    // -------------------------------------------------------------------------

    void DrawToScreen() override
    {
      IF::Instance().Printf((int)(PosX * QualityReductionFactor),
                            std::ceil(PosY * QualityReductionFactor) + 20,
                            IF::TextParams::Set(0xFFFFFF,
                                                IF::TextAlignment::LEFT,
                                                2.0),
                            "x = %.2f y = %.2f",
                            PosX, PosY);

      IF::Instance().Printf((int)(PosX * QualityReductionFactor),
                            std::ceil(PosY * QualityReductionFactor) + 40,
                            IF::TextParams::Set(0xFFFFFF,
                                                IF::TextAlignment::LEFT,
                                                2.0),
                            "std::round = (%d %d)",
                            (int)std::round(PosX),
                            (int)std::round(PosY));

      IF::Instance().Printf((int)(PosX * QualityReductionFactor),
                            std::ceil(PosY * QualityReductionFactor) + 60,
                            IF::TextParams::Set(0xFFFFFF,
                                                IF::TextAlignment::LEFT,
                                                2.0),
                            "truncate = (%d %d)",
                            (int)PosX,
                            (int)PosY);

      IF::Instance().Printf((int)(PosX * QualityReductionFactor),
                            std::ceil(PosY * QualityReductionFactor) + 180,
                            IF::TextParams::Set(0xFFFFFF,
                                                IF::TextAlignment::LEFT,
                                                2.0),
                            "SDLRenderDrawPointF");
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

            case SDLK_a:
              PosX -= ToAdd;
              break;

            case SDLK_d:
              PosX += ToAdd;
              break;

            case SDLK_w:
              PosY -= ToAdd;
              break;

            case SDLK_s:
              PosY += ToAdd;
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
    c.Run(true);
  }

  return 0;
}
