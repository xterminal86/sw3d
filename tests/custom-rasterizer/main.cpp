#include <cstdio>

#include "sw3d.h"
#include "instant-font.h"

using namespace SW3D;

const size_t QualityReductionFactor = 4;

size_t SelectionIndex = 0;

bool SubpixelDrawing    = false;
bool ScanlineRasterizer = true;

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

struct ParamsDDA
{
  double x1;
  double y1;
  double x2;
  double y2;
  double dx;
  double dy;
  double steps;
  double xInc;
  double yInc;
};

ParamsDDA ParamsDDA_;

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
                           t.Points[2].X + dx, t.Points[2].Y + dy);

        SDL_RenderDrawLineF(_renderer,
                           t.Points[2].X + dx, t.Points[2].Y + dy,
                           t.Points[0].X, t.Points[0].Y);
      }
      else
      {
        SDL_RenderDrawLine(_renderer,
                            t.Points[0].X, t.Points[0].Y,
                            t.Points[1].X, t.Points[1].Y);

        SDL_RenderDrawLine(_renderer,
                           t.Points[1].X, t.Points[1].Y,
                           (int)(t.Points[2].X + dx), (int)(t.Points[2].Y + dy));

        SDL_RenderDrawLine(_renderer,
                           (int)(t.Points[2].X + dx), (int)(t.Points[2].Y + dy),
                           t.Points[0].X, t.Points[0].Y);
      }
    }

    // -------------------------------------------------------------------------

    //
    // From https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage.html
    //
    // "In certain cases, a pixel may overlap more than one triangle, especially
    // when it lies precisely on an edge shared by two triangles. Such a pixel
    // would pass the coverage test for both triangles. If the triangles are
    // semi-transparent, a dark edge may appear where the pixels overlap the two
    // triangles due to the way semi-transparent objects are combined (imagine
    // two superimposed semi-transparent sheets of plastic. The surface becomes
    // more opaque and appears darker than the individual sheets).
    //
    // To address this issue, a rule is needed to ensure that a pixel can never
    // overlap two triangles sharing an edge twice. Most graphics APIs, such as
    // OpenGL and DirectX, define what they call the top-left rule. This rule
    // stipulates that a pixel or point is considered to overlap a triangle if
    // it is either inside the triangle or lies on a triangle's top edge or any
    // edge considered to be a left edge. What constitutes a top and left edge?
    //
    // 1) A top edge is an edge that is perfectly horizontal and whose defining
    //    vertices are positioned above the third one. Technically, this means
    //    that the y-coordinates of the vector V[ (X+1)%3 ] - V[X] are equal to
    //    0, and its x-coordinates are positive (greater than 0).
    // 2) A left edge is essentially an edge that is ascending. Given that
    //    vertices are defined in clockwise order in our context, an edge is
    //    considered ascending if its respective vector V[ (X+1)%3 ] - V[X]
    //    (where X can be 0, 1, 2) has a positive y-coordinate."
    //
    void FillTriangleC(const TriangleSimple& t)
    {

    }

    // -------------------------------------------------------------------------

    //
    // DDA stands for "Digital Differential Analyzer".
    //
    void DrawLineDDA(double x1, double y1, double x2, double y2)
    {
      double dx = x2 - x1;
      double dy = y2 - y1;

      double steps = (std::fabs(dx) > std::fabs(dy))
                    ? std::fabs(dx)
                    : std::fabs(dy);

      ParamsDDA_.x1 = x1;
      ParamsDDA_.y1 = y1;
      ParamsDDA_.x2 = x2;
      ParamsDDA_.y2 = y2;

      ParamsDDA_.dx = dx;
      ParamsDDA_.dy = dy;

      ParamsDDA_.steps = steps;

      if (steps == 0.0)
      {
        ParamsDDA_.xInc = 0.0;
        ParamsDDA_.yInc = 0.0;

        SDL_RenderDrawPoint(_renderer, std::round(x1), std::round(y1));
        return;
      }

      double xInc = dx / steps;
      double yInc = dy / steps;

      ParamsDDA_.xInc = xInc;
      ParamsDDA_.yInc = yInc;

      double x = x1;
      double y = y1;

      for (int i = 0; i <= (int)steps; i++)
      {
        SDL_RenderDrawPoint(_renderer, std::round(x), std::round(y));
        x += xInc;
        y += yInc;
      }
    }

    // -------------------------------------------------------------------------

    void DrawToFrameBuffer() override
    {
      SaveColor();

      if (ScanlineRasterizer)
      {
        SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
        FillTriangleC(CurrentTriangle);
      }
      else
      {
        SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);

        SDL_RenderDrawPointF(_renderer, dx, dy);

        SDL_SetRenderDrawColor(_renderer, 255, 0, 0, 255);

        SDL_RenderDrawPoint(_renderer, dx, dy + 6);

        SDL_SetRenderDrawColor(_renderer, 0, 255, 255, 255);

        DrawTriangleContour(CurrentTriangle);

        SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);

        FillTriangleC(CurrentTriangle);

        DrawLineDDA(10, 50, 50 + dx, 100 + dy);
      }

      RestoreColor();
    }

    // -------------------------------------------------------------------------

    void DrawToScreen() override
    {
      if (not ScanlineRasterizer)
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

        IF::Instance().Printf(0, 140,
                              IF::TextParams::Set(0xFFFFFF,
                                                  IF::TextAlignment::LEFT,
                                                  1.0),
                              "(%.2f ; %.2f) - (%.2f ; %.2f)",
                              ParamsDDA_.x1,
                              ParamsDDA_.y1,
                              ParamsDDA_.x2,
                              ParamsDDA_.y2);

        IF::Instance().Printf(0, 160,
                              IF::TextParams::Set(0xFFFFFF,
                                                  IF::TextAlignment::LEFT,
                                                  1.0),
                              "dx = %.2f dy = %.2f steps = %.2f, "
                              "xInc = %.2f yInc = %.2f",
                              ParamsDDA_.dx,
                              ParamsDDA_.dy,
                              ParamsDDA_.steps,
                              ParamsDDA_.xInc,
                              ParamsDDA_.yInc);
      }
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

            case SDLK_m:
              ScanlineRasterizer = not ScanlineRasterizer;
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
