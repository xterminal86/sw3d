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

void PitRasterizerTLR::SetFillConvention(FillConvention c)
{
  _fillConvention = c;
}

// =============================================================================

void PitRasterizerTLR::Rasterize(const TriangleSimple& t, bool wireframe)
{
  static SDL_Point p1, p2, p3;

  _tmp = t;

  SortVertices();
  CheckAndFixWinding();

  //
  // After sorting and fixing vertices can only be like these:
  //
  // 1   2  |    1     |    1      |     1
  //        |          |           |
  //   3    |  3    2  |       2   |  3
  //        |          |           |
  //        |          |  3        |        2
  //
  //

  p1 = { (int)_tmp.Points[0].X, (int)_tmp.Points[0].Y };
  p2 = { (int)_tmp.Points[1].X, (int)_tmp.Points[1].Y };
  p3 = { (int)_tmp.Points[2].X, (int)_tmp.Points[2].Y };

  int xMin = std::min( std::min(p1.x, p2.x), p3.x);
  int yMin = std::min( std::min(p1.y, p2.y), p3.y);
  int xMax = std::max( std::max(p1.x, p2.x), p3.x);
  int yMax = std::max( std::max(p1.y, p2.y), p3.y);

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
  // It seems that you can choose any type of convention as long as it
  // produces consistent results, but I guess there are reasons why people
  // chose top-left rule that I don't know about. I can only guess that it
  // might be connected with the fact that most people read left to right top
  // to bottom, so if you chip rightmost and downmost pixel edges, it will be
  // least noticeable.
  //

  int bias1 = 0;
  int bias2 = 0;
  int bias3 = 0;

  switch (_fillConvention)
  {
    case FillConvention::TOP_LEFT:
    {
      bias1 = IsTopLeft(p1, p2) ? 0 : -1;
      bias2 = IsTopLeft(p2, p3) ? 0 : -1;
      bias3 = IsTopLeft(p3, p1) ? 0 : -1;
    }
    break;

    case FillConvention::BOTTOM_RIGHT:
    {
      bias1 = IsBottomRight(p1, p2) ? 0 : -1;
      bias2 = IsBottomRight(p2, p3) ? 0 : -1;
      bias3 = IsBottomRight(p3, p1) ? 0 : -1;
    }
    break;

    case FillConvention::TOP_RIGHT:
    {
      bias1 = IsTopRight(p1, p2) ? 0 : -1;
      bias2 = IsTopRight(p2, p3) ? 0 : -1;
      bias3 = IsTopRight(p3, p1) ? 0 : -1;
    }
    break;

    case FillConvention::BOTTOM_LEFT:
    {
      bias1 = IsBottomLeft(p1, p2) ? 0 : -1;
      bias2 = IsBottomLeft(p2, p3) ? 0 : -1;
      bias3 = IsBottomLeft(p3, p1) ? 0 : -1;
    }
    break;

    default:
      break;
  }

  SDL_Point p;

  for (int x = xMin; x <= xMax; x++)
  {
    p.x = x;

    for (int y = yMin; y <= yMax; y++)
    {
      p.y = y;

      int cp1 = (p2.x - p1.x) * (p.y - p1.y) - (p2.y - p1.y) * (p.x - p1.x);
      int cp2 = (p3.x - p2.x) * (p.y - p2.y) - (p3.y - p2.y) * (p.x - p2.x);
      int cp3 = (p1.x - p3.x) * (p.y - p3.y) - (p1.y - p3.y) * (p.x - p3.x);

      int w0 = cp1 + bias1;
      int w1 = cp2 + bias2;
      int w2 = cp3 + bias3;

      bool inside = (w0 >= 0 and w1 >= 0 and w2 >= 0);
      if (wireframe)
      {
        if ((cp1 == 0 and bias1 == 0)
         or (cp2 == 0 and bias2 == 0)
         or (cp3 == 0 and bias3 == 0))
        {
          SDL_RenderDrawPoint(_renderer, p.x, p.y);
        }
      }
      else
      {
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
  // top edge is the one which has the same Y (hence edge.Y == 0) and
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

// =============================================================================

bool PitRasterizerTLR::IsBottomRight(const SDL_Point& start, const SDL_Point& end)
{
  static Vec3 edge;
  edge.X = end.x - start.x;
  edge.Y = end.y - start.y;

  bool isBottomEdge = (edge.Y == 0) and (edge.X < 0);
  bool isRightEdge  = edge.Y > 0;

  return isBottomEdge || isRightEdge;
}

// =============================================================================

bool PitRasterizerTLR::IsTopRight(const SDL_Point& start, const SDL_Point& end)
{
  static Vec3 edge;
  edge.X = end.x - start.x;
  edge.Y = end.y - start.y;

  bool isTopEdge   = (edge.Y == 0) and (edge.X > 0);
  bool isRightEdge = edge.Y > 0;

  return isTopEdge || isRightEdge;
}

// =============================================================================

bool PitRasterizerTLR::IsBottomLeft(const SDL_Point& start, const SDL_Point& end)
{
  static Vec3 edge;
  edge.X = end.x - start.x;
  edge.Y = end.y - start.y;

  bool isBottomEdge = (edge.Y == 0) and (edge.X < 0);
  bool isLeftEdge   = edge.Y < 0;

  return isBottomEdge || isLeftEdge;
}
