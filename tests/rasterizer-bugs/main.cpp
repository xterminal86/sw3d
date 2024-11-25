#include <cstdio>

#include "sw3d.h"
#include "instant-font.h"
#include "srtl.h"
#include "pit-rasterizer.h"
#include "pit-rasterizer-tlr.h"

using namespace SW3D;

const size_t QualityReductionFactor = 4;

bool Wireframe = false;
bool HideText  = true;

const double RotationFactor = 0.01;
const double RotationSpeed  = 50.0;

double AngleX = 0.0;
double AngleY = 0.0;
double AngleZ = 0.0;

std::vector<SDL_Color> TriangleColors =
{
  { 255,   0,   0, 255 },
  { 0,   255,   0, 255 },
  { 0,     0, 255, 255 },
  { 255, 255,   0, 255 },
  { 0,   255, 255, 255 },
  { 255,   0, 255, 255 },
  { 64,   96, 128, 255 },
  { 32,   64,  96, 255 },
  { 64,   32, 128, 255 },
  { 128,  64,  32, 255 },
  { 64,   96, 160, 255 },
  { 64,  150,  26, 255 },
};

// =============================================================================

class TLR : public DrawWrapper
{
  public:

    // -------------------------------------------------------------------------

    void PostInit() override
    {
      _rasterizer.Init(_renderer);

      _cube.Triangles =
      {
        //// FRONT
        { 0.0, 0.0, 0.0,    1.0, 0.0, 0.0,    0.0, 1.0, 0.0 },
        { 1.0, 0.0, 0.0,    1.0, 1.0, 0.0,    0.0, 1.0, 0.0 },
        //// BACK
        //{ 0.0, 0.0, 1.0,    0.0, 1.0, 1.0,    1.0, 0.0, 1.0 },
        //{ 1.0, 0.0, 1.0,    0.0, 1.0, 1.0,    1.0, 1.0, 1.0 },
        //
        //// LEFT
        //{ 0.0, 0.0, 0.0,    0.0, 1.0, 0.0,    0.0, 1.0, 1.0 },
        //{ 0.0, 0.0, 0.0,    0.0, 1.0, 1.0,    0.0, 0.0, 1.0 },
        //
        //// RIGHT
        //{ 1.0, 0.0, 0.0,    1.0, 0.0, 1.0,    1.0, 1.0, 1.0 },
        //{ 1.0, 0.0, 0.0,    1.0, 1.0, 1.0,    1.0, 1.0, 0.0 },
        //
        //// TOP
        //{ 0.0, 1.0, 0.0,    1.0, 1.0, 0.0,    0.0, 1.0, 1.0 },
        //{ 1.0, 1.0, 0.0,    1.0, 1.0, 1.0,    0.0, 1.0, 1.0 },
        //
        //// BOTTOM
        //{ 0.0, 0.0, 0.0,    0.0, 0.0, 1.0,    1.0, 0.0, 0.0 },
        //{ 1.0, 0.0, 0.0,    0.0, 0.0, 1.0,    1.0, 0.0, 1.0 },
      };
    }

    // -------------------------------------------------------------------------

    void DrawToFrameBuffer() override
    {
      SaveColor();

      SetWeakPerspective();

      _triScreenSpace.clear();

      size_t colorIndex = 0;

      for (TriangleSimple& t : _cube.Triangles)
      {
        TriangleSimple tr = t;

        for (size_t i = 0; i < 3; i++)
        {
          tr.Points[i] = SW3D::RotateZ(tr.Points[i], AngleZ);
          tr.Points[i] = SW3D::RotateY(tr.Points[i], AngleY);
          tr.Points[i] = SW3D::RotateX(tr.Points[i], AngleX);
        }

        TriangleSimple tt = tr;

        for (size_t i = 0; i < 3; i++)
        {
          tt.Points[i].Z = 2.5;
        }

        TriangleSimple tp;

        for (size_t i = 0; i < 3; i++)
        {
          tp.Points[i] = _projectionMatrix * tt.Points[i];

          tp.Points[i].X += 1;
          tp.Points[i].Y += 1;

          tp.Points[i].X /= 2.0;
          tp.Points[i].Y /= 2.0;

          tp.Points[i].X *= (double)FrameBufferSize();
          tp.Points[i].Y *= (double)FrameBufferSize();
        }

        _triScreenSpace.push_back(tp);

        auto& clr = TriangleColors[colorIndex];

        SDL_SetRenderDrawColor(_renderer, clr.r, clr.g, clr.b, clr.a);
        _rasterizer.Rasterize(tp, Wireframe);

        colorIndex++;
      }

      RestoreColor();
    }

    // -------------------------------------------------------------------------

    void DrawToScreen() override
    {
      if (HideText)
      {
        return;
      }

      size_t cnt = 0;
      for (auto& t : _triScreenSpace)
      {
        auto& clr = TriangleColors[cnt];

        IF::Instance().Printf(0, cnt * 80,
                              IF::TextParams::Set(RGBA2HTML(clr),
                                                  IF::TextAlignment::LEFT,
                                                  2.0),
                              "%s",
                              ToString(t.Points[0]).data());
        IF::Instance().Printf(0, cnt * 80 + 20,
                              IF::TextParams::Set(RGBA2HTML(clr),
                                                  IF::TextAlignment::LEFT,
                                                  2.0),
                              "%s",
                              ToString(t.Points[1]).data());
        IF::Instance().Printf(0, cnt * 80 + 40,
                              IF::TextParams::Set(RGBA2HTML(clr),
                                                  IF::TextAlignment::LEFT,
                                                  2.0),
                              "%s",
                              ToString(t.Points[2]).data());
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

            case SDLK_a:
              AngleY -= (RotationFactor * RotationSpeed);
              break;

            case SDLK_d:
              AngleY += (RotationFactor * RotationSpeed);
              break;

            case SDLK_w:
              AngleX -= (RotationFactor * RotationSpeed);
              break;

            case SDLK_s:
              AngleX += (RotationFactor * RotationSpeed);
              break;

            case SDLK_q:
              AngleZ -= (RotationFactor * RotationSpeed);
              break;

            case SDLK_e:
              AngleZ += (RotationFactor * RotationSpeed);
              break;

            case SDLK_h:
              HideText = not HideText;
              break;

            default:
              break;
          }
        }
      }
    }

  private:
    Mesh _cube;

    std::vector<TriangleSimple> _triScreenSpace;

    //
    // BUG: some pixels are not filled.
    //
    //SRTL _rasterizer;

    //ScanlineRasterizer _rasterizer;
    //PitRasterizer      _rasterizer;
    PitRasterizerTLR   _rasterizer;
};

// =============================================================================

int main(int argc, char* argv[])
{
  TLR c;

  if (c.Init(800, 800, QualityReductionFactor))
  {
    IF::Instance().Init(c.GetRenderer());
    c.Run(true);
  }

  return 0;
}
