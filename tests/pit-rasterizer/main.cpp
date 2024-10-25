#include <cstdio>

#include "sw3d.h"
#include "instant-font.h"

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

    //
    // "Point In Triangle" method.
    //
    // Loop across triangle bounding box and draw pixel if it falls inside the
    // triangle. Testing is performed using so-called 2D cross product (which
    // is basically a normal cross product with Z set to 0). Point lies inside
    // a triangle only if all cross products produce the same sign.
    // Obviously, by using this method we're wasting exactly half amount of
    // work since area of a triangle is half the area of outlined rectangle.
    //
    void PitRasterizer(TriangleSimple& t)
    {
      static SDL_Point p1, p2, p3;

      p1 = { (int)t.Points[0].X, (int)t.Points[0].Y };
      p2 = { (int)t.Points[1].X, (int)t.Points[1].Y };
      p3 = { (int)t.Points[2].X, (int)t.Points[2].Y };

      int xMin = std::min( std::min(p1.x, p2.x), p3.x);
      int yMin = std::min( std::min(p1.y, p2.y), p3.y);
      int xMax = std::max( std::max(p1.x, p2.x), p3.x);
      int yMax = std::max( std::max(p1.y, p2.y), p3.y);

      if (Wireframe)
      {
        //
        // Triangle degenerated into vertical or horizontal line.
        //
        bool isLine = (p1.y == p2.y and p2.y == p3.y)
                   or (p1.x == p2.x and p2.x == p3.x);
        if (isLine)
        {
          SDL_RenderDrawLine(_renderer, xMin, yMin, xMax, yMax);
        }
        else
        {
          SDL_RenderDrawLine(_renderer, p1.x, p1.y, p2.x, p2.y);
          SDL_RenderDrawLine(_renderer, p1.x, p1.y, p3.x, p3.y);
          SDL_RenderDrawLine(_renderer, p2.x, p2.y, p3.x, p3.y);
        }
      }
      else
      {
        for (int x = xMin; x <= xMax; x++)
        {
          for (int y = yMin; y <= yMax; y++)
          {
            SDL_Point p = { x, y };

            //
            // It seems that if we don't use a function call (CrossProduct2D)
            // and just perform calculations directly, this way it works a
            // little bit faster.
            //
            int w0 = (p2.x - p1.x) * (p.y - p1.y) - (p2.y - p1.y) * (p.x - p1.x);
            int w1 = (p3.x - p2.x) * (p.y - p2.y) - (p3.y - p2.y) * (p.x - p2.x);
            int w2 = (p1.x - p3.x) * (p.y - p3.y) - (p1.y - p3.y) * (p.x - p3.x);

            bool inside = (w0 <= 0 and w1 <= 0 and w2 <= 0)
                       or (w0 >= 0 and w1 >= 0 and w2 >= 0);

            if (inside)
            {
              SDL_RenderDrawPoint(_renderer, p.x, p.y);
            }
          }
        }
      }
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
      PitRasterizer(CurrentTriangle);

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
    c.Run(true);
  }

  return 0;
}
