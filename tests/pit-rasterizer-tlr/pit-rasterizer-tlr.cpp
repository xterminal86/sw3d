#include "pit-rasterizer-tlr.h"

void PitRasterizerTLR::Init(SDL_Renderer* rendererRef)
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

void PitRasterizerTLR::Rasterize(const TriangleSimple& t, bool wireframe)
{
  static SDL_Point p1, p2, p3;

  _tmp = t;

  SortVertices();
  CheckAndFixWinding();

  p1 = { (int)_tmp.Points[0].X, (int)_tmp.Points[0].Y };
  p2 = { (int)_tmp.Points[1].X, (int)_tmp.Points[1].Y };
  p3 = { (int)_tmp.Points[2].X, (int)_tmp.Points[2].Y };

  int xMin = std::min( std::min(p1.x, p2.x), p3.x);
  int yMin = std::min( std::min(p1.y, p2.y), p3.y);
  int xMax = std::max( std::max(p1.x, p2.x), p3.x);
  int yMax = std::max( std::max(p1.y, p2.y), p3.y);

  if (wireframe)
  {
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
    //
    // In order to adhere to the top-left rule by using PIT rasterizer,
    // this time we're actually going to rely on a sign of CrossProduct2D().
    // In previous iteration of this rasterizer we didn't care about overdraw,
    // so we didn't care about the sign of a result from CrossProduct2D().
    // As long as it's the same for all 3 conditions, point is considered to lie
    // inside the triangle. This also meant that we didn't care about vertex
    // order either because we're going to iterate pixelwise across bounding box
    // anyway - all that was needed is to calculate box top left corder and
    // bottom right, which can be easily done using simple min-max testing.
    //
    // Let's recall this top-left rule again.
    // A pixel is considered to belong to the triangle if it's either completely
    // isinde the triangle or - in case it happens to be on one of the
    // triangle's edges - this edge satisfies top-left rule, which is:
    //
    // 1. Edge is a flat-top edge.
    // 2. Edge is a "left" edge, that is difference between its end and start
    //    points is negative (provided we chose CW winding order), that is it
    //    "goes up".
    //
    // Some examples follow:
    //
    //
    // ---------- <--- top edge
    // \        /
    //  \      /
    // ^ \    /
    // |  \  /
    // |   \/
    // |
    // left edge
    //
    //
    //
    //
    //            /|
    //   +-----> / |
    //   |      /  |
    //   |     /   |
    //   |    \    |
    //   |     \   |
    //   +----> \  |
    //   |       \ |
    //   |        \|
    //   |
    //   left edges
    //
    //
    // If winding order would've been different, than left edge were the one
    // that "goes down".
    //
    // So, obviously, what we need to do is draw pixels that happen to lay
    // on top or left edges. With our CW winding order chosen it's obvious that
    // pixel will belong to the triangle if CrossProduct2D() >= 0.
    // So what we're doing here is check whether current 3 edges of a triangle
    // happen to top or left and if some of them are, we don't do anything,
    // hence 0. Otherwise we assign -1 which will be later added to the result
    // of CrossProduct2D() thus artificially forcing condition to fail and pixel
    // not to be drawn. We can do that because we're working in integer domain
    // and minimum possible distance between two pixels is 1, so by adding -1
    // we're sort of "chipping off" 1 pixel from right and bottom edges of a
    // triangle.
    //
    int bias1 = IsTopLeft(p2, p1) ? 0 : -1;
    int bias2 = IsTopLeft(p3, p2) ? 0 : -1;
    int bias3 = IsTopLeft(p1, p3) ? 0 : -1;

    for (int x = xMin; x <= xMax; x++)
    {
      for (int y = yMin; y <= yMax; y++)
      {
        SDL_Point p = { x, y };

        int w0 = (p2.x - p1.x) * (p.y - p1.y) - (p2.y - p1.y) * (p.x - p1.x) + bias1;
        int w1 = (p3.x - p2.x) * (p.y - p2.y) - (p3.y - p2.y) * (p.x - p2.x) + bias2;
        int w2 = (p1.x - p3.x) * (p.y - p3.y) - (p1.y - p3.y) * (p.x - p3.x) + bias3;

        bool inside = (w0 >= 0 and w1 >= 0 and w2 >= 0);
        if (inside)
        {
          SDL_RenderDrawPoint(_renderer, p.x, p.y);
        }
      }
    }
  }
}

// =============================================================================

void PitRasterizerTLR::SortVertices()
{
  bool sorted = false;

  while (not sorted)
  {
    sorted = true;

    for (size_t i = 0; i < 2; i++)
    {
      bool sortingCondition = (_tmp.Points[i].Y >  _tmp.Points[i + 1].Y)
                           or (_tmp.Points[i].Y == _tmp.Points[i + 1].Y
                           and _tmp.Points[i].X >  _tmp.Points[i + 1].X);
      if (sortingCondition)
      {
        std::swap(_tmp.Points[i].X, _tmp.Points[i + 1].X);
        std::swap(_tmp.Points[i].Y, _tmp.Points[i + 1].Y);
        sorted = false;
      }
    }
  }
}

// =============================================================================

void PitRasterizerTLR::CheckAndFixWinding()
{
  WindingOrder wo = GetWindingOrder(_tmp);
  if (wo == WindingOrder::CCW)
  {
    std::swap(_tmp.Points[1].X, _tmp.Points[2].X);
    std::swap(_tmp.Points[1].Y, _tmp.Points[2].Y);
  }
}

// =============================================================================

bool PitRasterizerTLR::IsTopLeft(const SDL_Point& start, const SDL_Point& end)
{
  static Vec3 edge;
  edge.X = end.x - start.x;
  edge.Y = end.y - start.y;

  //
  // top edge is the one which has the same Y (hence egde.Y == 0) and
  // (edge.X > 0) to distinguish it from flat bottom. If we have consistent
  // winding order we will always have (edge.X > 0) for flat top and
  // (edge.X < 0) for flat bottom for CW winding order, and vice verca for CCW.
  //
  // If winding order were different (CCW), then everything would've been
  // "swapped" around: isTopEdge would have (edge.X < 0) and
  // isLeftEdge = (edge.Y > 0)
  //
  bool isTopEdge  = (edge.Y == 0) and (edge.X > 0);
  bool isLeftEdge = edge.Y < 0;

  return isTopEdge || isLeftEdge;
}
