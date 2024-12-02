#include "pit-rasterizer.h"

void PitRasterizer::Init(SDL_Renderer* rendererRef)
{
  if (rendererRef == nullptr)
  {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                 "%s: rendererRef is nullptr!", __func__);
    return;
  }

  if (_initialized)
  {
    SDL_LogWarn(SDL_LOG_PRIORITY_WARN, "%s: already initialized", __func__);
    return;
  }

  _renderer = rendererRef;
  _initialized = true;
}

// =============================================================================

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
// Actually, as far as I learned during research, this method is actually an
// industry standard and modern graphics cards use this one for rasterization
// because it's easy to parallelize: you can split screen (framebuffer would be
// more correctly I guess) in tiles and perform such PIT rasterization on a tile
// by tile basis using several thousands of GPU cores in parallel (or something
// like that). While in scanline rasterization you can't precalculate all point
// needed for scanlines. Well, I guess you could, but like I said, as far as my
// research goes it seems it's not as efficient as PIT method.
//
// I guess it all started from famous 1988 paper by Juan Piñeda called
// "A Parallel Algorithm for Polygon Rasterization".
//
void PitRasterizer::Rasterize(const TriangleSimple& t, bool wireframe)
{
  static SDL_Point p1, p2, p3;

  p1 = { (int)t.Points[0].X, (int)t.Points[0].Y };
  p2 = { (int)t.Points[1].X, (int)t.Points[1].Y };
  p3 = { (int)t.Points[2].X, (int)t.Points[2].Y };

  int xMin = std::min( std::min(p1.x, p2.x), p3.x);
  int yMin = std::min( std::min(p1.y, p2.y), p3.y);
  int xMax = std::max( std::max(p1.x, p2.x), p3.x);
  int yMax = std::max( std::max(p1.y, p2.y), p3.y);

  if (wireframe)
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
    for (int y = yMin; y <= yMax; y++)
    {
      for (int x = xMin; x <= xMax; x++)
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
