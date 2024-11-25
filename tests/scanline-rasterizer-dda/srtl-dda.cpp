#include "srtl-dda.h"

// =============================================================================

void SRTLDDA::Init(SDL_Renderer* rendererRef)
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

void SRTLDDA::DrawFT(const TriangleSimple& t)
{
  //
  // 1   2
  //
  //   3
  //
  double kl = (t.Points[2].X - t.Points[0].X) / (t.Points[2].Y - t.Points[0].Y);
  double kr = (t.Points[2].X - t.Points[1].X) / (t.Points[2].Y - t.Points[1].Y);

  const int yStart = (int)std::floor(t.Points[0].Y + 0.5);
  const int yEnd   = (int)std::floor(t.Points[2].Y + 0.5);

  for (int y = yStart; y < yEnd; y++)
  {
    //
    // y = kx + b
    // x = y - kx
    //
    const double pxl = kl * (double(y) + 0.5 - t.Points[0].Y) + t.Points[0].X;
    const double pxr = kr * (double(y) + 0.5 - t.Points[1].Y) + t.Points[1].X;

    const int xStart = (int)std::floor(pxl + 0.5);
    const int xEnd   = (int)std::floor(pxr + 0.5);

    for (int x = xStart; x < xEnd; x++)
    {
      if (_wireframe)
      {
        if (y == yStart or y == (yEnd - 1) or x == xStart or x == (xEnd - 1))
        {
          SDL_RenderDrawPoint(_renderer, x, y);
        }
      }
      else
      {
        SDL_RenderDrawPoint(_renderer, x, y);
      }
    }
  }
}

// =============================================================================

void SRTLDDA::DrawFB(const TriangleSimple& t)
{
  //
  //   1
  //
  // 3   2
  //
  double kl = (t.Points[2].X - t.Points[0].X) / (t.Points[2].Y - t.Points[0].Y);
  double kr = (t.Points[1].X - t.Points[0].X) / (t.Points[1].Y - t.Points[0].Y);

  const int yStart = (int)std::floor(t.Points[0].Y + 0.5);
  const int yEnd   = (int)std::floor(t.Points[2].Y + 0.5);

  for (int y = yStart; y < yEnd; y++)
  {
    const double pxl = kl * (double(y) + 0.5 - t.Points[0].Y) + t.Points[0].X;
    const double pxr = kr * (double(y) + 0.5 - t.Points[1].Y) + t.Points[1].X;

    const int xStart = (int)std::floor(pxl + 0.5);
    const int xEnd   = (int)std::floor(pxr + 0.5);

    for (int x = xStart; x < xEnd; x++)
    {
      if (_wireframe)
      {
        if (y == yStart or y == (yEnd - 1) or x == xStart or x == (xEnd - 1))
        {
          SDL_RenderDrawPoint(_renderer, x, y);
        }
      }
      else
      {
        SDL_RenderDrawPoint(_renderer, x, y);
      }
    }
  }
}

// =============================================================================

void SRTLDDA::DrawMR(const TriangleSimple& t)
{
  //
  //   1
  //
  // 3
  //
  //      2
  //
  double a = (t.Points[2].Y - t.Points[0].Y) / (t.Points[1].Y - t.Points[0].Y);
  Vec3 x = t.Points[0] + (t.Points[1] - t.Points[0]) * a;

  TriangleSimple fb = { t.Points[0], x, t.Points[2] };
  DrawFB(fb);

  TriangleSimple ft = { t.Points[2], x, t.Points[1] };
  DrawFT(ft);
}

// =============================================================================

void SRTLDDA::DrawML(const TriangleSimple& t)
{
  //
  //   1
  //
  //     2
  //
  // 3
  //
  double a = (t.Points[1].Y - t.Points[0].Y) / (t.Points[2].Y - t.Points[0].Y);
  Vec3 x = t.Points[0] + (t.Points[2] - t.Points[0]) * a;

  TriangleSimple fb = { t.Points[0], t.Points[1], x };
  DrawFB(fb);

  TriangleSimple ft = { x, t.Points[1], t.Points[2] };
  DrawFT(ft);
}

// =============================================================================

void SRTLDDA::Rasterize(const TriangleSimple& t, bool wireframe)
{
  _wireframe = wireframe;

  _copy = t;

  SortVertices();
  CheckAndFixWinding();

  PerformRasterization();
}

// =============================================================================

void SRTLDDA::PerformRasterization()
{
  TriangleType tt = GetTriangleType(_copy);
  switch (tt)
  {
    case TriangleType::VERTICAL_LINE:
      //DrawVL();
      break;

    case TriangleType::HORIZONTAL_LINE:
      //DrawHL();
      break;

    case TriangleType::FLAT_TOP:
      DrawFT(_copy);
      break;

    case TriangleType::FLAT_BOTTOM:
      DrawFB(_copy);
      break;

    case TriangleType::MAJOR_RIGHT:
      DrawMR(_copy);
      break;

    case TriangleType::MAJOR_LEFT:
      DrawML(_copy);
      break;

    default:
      break;
  }
}

// =============================================================================

void SRTLDDA::SortVertices()
{
  bool sorted = false;

  while (not sorted)
  {
    sorted = true;

    for (size_t i = 0; i < 2; i++)
    {
      bool sortingCondition = ((int)_copy.Points[i].Y >  (int)_copy.Points[i + 1].Y)
                           or ((int)_copy.Points[i].Y == (int)_copy.Points[i + 1].Y
                           and (int)_copy.Points[i].X >  (int)_copy.Points[i + 1].X);
      if (sortingCondition)
      {
        std::swap(_copy.Points[i].X, _copy.Points[i + 1].X);
        std::swap(_copy.Points[i].Y, _copy.Points[i + 1].Y);
        sorted = false;
      }
    }
  }
}

// =============================================================================

void SRTLDDA::CheckAndFixWinding()
{
  WindingOrder wo = GetWindingOrder(_copy);
  if (wo == WindingOrder::CCW)
  {
    std::swap(_copy.Points[1].X, _copy.Points[2].X);
    std::swap(_copy.Points[1].Y, _copy.Points[2].Y);
  }
}

// =============================================================================

WindingOrder SRTLDDA::GetWindingOrder(const TriangleSimple& t)
{
  double cp = CrossProduct2D(t.Points[1] - t.Points[0],
                             t.Points[2] - t.Points[1]);

  return cp > 0 ? WindingOrder::CW : WindingOrder::CCW;
}

// =============================================================================

TriangleType SRTLDDA::GetTriangleType(const TriangleSimple& t)
{
  const int& y1 = (int)t.Points[0].Y;
  const int& y2 = (int)t.Points[1].Y;
  const int& y3 = (int)t.Points[2].Y;

  const int& x1 = (int)t.Points[0].X;
  const int& x2 = (int)t.Points[1].X;
  const int& x3 = (int)t.Points[2].X;

  if (y1 == y2 and y2 == y3)
  {
    return TriangleType::HORIZONTAL_LINE;
  }

  if (x1 == x2 and x2 == x3)
  {
    return TriangleType::VERTICAL_LINE;
  }

  if (y1 == y2)
  {
    return TriangleType::FLAT_TOP;
  }

  if (y2 == y3)
  {
    return TriangleType::FLAT_BOTTOM;
  }

  if (y2 > y3)
  {
    return TriangleType::MAJOR_RIGHT;
  }

  if (y2 < y3)
  {
    return TriangleType::MAJOR_LEFT;
  }

  return TriangleType::UNDEFINED;
}
