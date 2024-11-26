#include "scanline-rasterizer.h"

void ScanlineRasterizer::Init(SDL_Renderer* rendererRef)
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

WindingOrder ScanlineRasterizer::GetWindingOrder(const TriangleSimple& t)
{
  double cp = CrossProduct2D(t.Points[1] - t.Points[0],
                             t.Points[2] - t.Points[1]);

  return cp > 0 ? WindingOrder::CW : WindingOrder::CCW;
}

// =============================================================================

void ScanlineRasterizer::SortVertices(TriangleSimple& t)
{
  bool sorted = false;

  while (not sorted)
  {
    sorted = true;

    for (size_t i = 0; i < 2; i++)
    {
      bool sortingCondition = ((int)t.Points[i].Y >  (int)t.Points[i + 1].Y)
                           or ((int)t.Points[i].Y == (int)t.Points[i + 1].Y
                           and (int)t.Points[i].X >  (int)t.Points[i + 1].X);
      if (sortingCondition)
      {
        std::swap(t.Points[i].X, t.Points[i + 1].X);
        std::swap(t.Points[i].Y, t.Points[i + 1].Y);
        sorted = false;
      }
    }
  }
}

// =============================================================================

void ScanlineRasterizer::CheckAndFixWinding(TriangleSimple& t)
{
  WindingOrder wo = GetWindingOrder(t);
  if (wo == WindingOrder::CCW)
  {
    std::swap(t.Points[1].X, t.Points[2].X);
    std::swap(t.Points[1].Y, t.Points[2].Y);
  }
}

// =============================================================================

TriangleType ScanlineRasterizer::GetTriangleType(const TriangleSimple& t)
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

// =============================================================================

void ScanlineRasterizer::DrawVL()
{
  int x  = _copy.Points[0].X;

  int y1 = _copy.Points[0].Y;
  int y2 = _copy.Points[1].Y;

  for (int scanline = y1; scanline < y2; scanline++)
  {
    SDL_RenderDrawPoint(_renderer, x, scanline);
  }
}

// =============================================================================

void ScanlineRasterizer::DrawHL()
{
  int scanline = _copy.Points[0].Y;

  int x1 = _copy.Points[0].X;
  int x2 = _copy.Points[1].X;

  for (int x = x1; x < x2; x++)
  {
    SDL_RenderDrawPoint(_renderer, x, scanline);
  }
}

// =============================================================================

void ScanlineRasterizer::DrawFT(const TriangleSimple& t)
{
  _first.Init(t.Points[0].X,
              t.Points[0].Y,
              t.Points[2].X,
              t.Points[2].Y);

  _second.Init(t.Points[1].X,
               t.Points[1].Y,
               t.Points[2].X,
               t.Points[2].Y);

  if (_drawWireframe)
  {
    PerformRasterizationWireframe(_first, _second, t, TriangleType::FLAT_TOP);
  }
  else
  {
    PerformRasterization(_first, _second, t, TriangleType::FLAT_TOP);
  }
}

// =============================================================================

void ScanlineRasterizer::DrawFB(const TriangleSimple& t)
{
  _first.Init(t.Points[0].X,
              t.Points[0].Y,
              t.Points[2].X,
              t.Points[2].Y);

  _second.Init(t.Points[0].X,
               t.Points[0].Y,
               t.Points[1].X,
               t.Points[1].Y);

  if (_drawWireframe)
  {
    PerformRasterizationWireframe(_first, _second, t, TriangleType::FLAT_BOTTOM);
  }
  else
  {
    PerformRasterization(_first, _second, t, TriangleType::FLAT_BOTTOM);
  }
}

// =============================================================================

void ScanlineRasterizer::DrawMR(const TriangleSimple& t)
{
  double a = (t.Points[2].Y - t.Points[0].Y) / (t.Points[1].Y - t.Points[0].Y);

  Vec3 x = t.Points[0] + (t.Points[1] - t.Points[0]) * a;

  TriangleSimple fb = { t.Points[0], x, t.Points[2] };
  DrawFB(fb);

  TriangleSimple ft = { t.Points[2], x, t.Points[1] };
  DrawFT(ft);
}

// =============================================================================

void ScanlineRasterizer::DrawML(const TriangleSimple& t)
{
  double a = (t.Points[1].Y - t.Points[0].Y) / (t.Points[2].Y - t.Points[0].Y);
  Vec3 x = t.Points[0] + (t.Points[2] - t.Points[0]) * a;

  TriangleSimple fb = { t.Points[0], t.Points[1], x };
  DrawFB(fb);

  TriangleSimple ft = { x, t.Points[1], t.Points[2] };
  DrawFT(ft);
}

// =============================================================================

void ScanlineRasterizer::Rasterize(const TriangleSimple& t, bool wireframe)
{
  if (not _initialized)
  {
    return;
  }

  _copy = t;

  _drawWireframe = wireframe;

  SortVertices(_copy);
  CheckAndFixWinding(_copy);

  TriangleType tt = GetTriangleType(_copy);
  switch (tt)
  {
    case TriangleType::VERTICAL_LINE:
      DrawVL();
      break;

    case TriangleType::HORIZONTAL_LINE:
      DrawHL();
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

void ScanlineRasterizer::PerformRasterization(BLG& first,
                                              BLG& second,
                                              const TriangleSimple& t,
                                              TriangleType tt)
{
  BLG::Point* p1 = first.Next();
  BLG::Point* p2 = second.Next();

  if (p1 == nullptr or p2 == nullptr)
  {
    return;
  }

  int x1 = p1->first;
  int x2 = p2->first;

  int y1 = t.Points[0].Y;
  int y2 = t.Points[0].Y;

  PointCaptureType ctLine1 = PointCaptureType::UNDEFINED;
  PointCaptureType ctLine2 = PointCaptureType::UNDEFINED;

  switch (tt)
  {
    case TriangleType::FLAT_BOTTOM:
    {
      ctLine1 = (t.Points[2].X <= t.Points[0].X)
                ? PointCaptureType::LAST
                : PointCaptureType::FIRST;

      ctLine2 = (t.Points[1].X <= t.Points[0].X)
                ? PointCaptureType::FIRST
                : PointCaptureType::LAST;

      y2 = t.Points[1].Y;
    }
    break;

    case TriangleType::FLAT_TOP:
    {
      ctLine1 = (t.Points[2].X <= t.Points[0].X)
                ? PointCaptureType::LAST
                : PointCaptureType::FIRST;

      ctLine2 = (t.Points[2].X <= t.Points[1].X)
                ? PointCaptureType::FIRST
                : PointCaptureType::LAST;

      y2 = t.Points[2].Y;
    }
    break;
  }

  for (int currentScanline = y1; currentScanline <= y2; currentScanline++)
  {
    if (p1 != nullptr
    and p1->second == currentScanline
    and ctLine1 == PointCaptureType::FIRST)
    {
      x1 = p1->first;
    }

    while (p1 != nullptr and p1->second == currentScanline)
    {
      if (ctLine1 == PointCaptureType::LAST)
      {
        x1 = p1->first;
      }

      p1 = first.Next();
    }

    if (p2 != nullptr
    and p2->second == currentScanline
    and ctLine2 == PointCaptureType::FIRST)
    {
      x2 = p2->first;
    }

    while (p2 != nullptr and p2->second == currentScanline)
    {
      if (ctLine2 == PointCaptureType::LAST)
      {
        x2 = p2->first;
      }

      p2 = second.Next();
    }

    for (int x = x1; x <= x2; x++)
    {
      SDL_RenderDrawPoint(_renderer, x, currentScanline);
    }
  }
}

// =============================================================================

void ScanlineRasterizer::PerformRasterizationWireframe(BLG& first,
                                                       BLG& second,
                                                       const TriangleSimple& t,
                                                       TriangleType tt)
{
  BLG::Point* p1 = first.Next();
  BLG::Point* p2 = second.Next();

  if (p1 == nullptr or p2 == nullptr)
  {
    return;
  }

  int x1 = p1->first;
  int x2 = p2->first;

  int y1 = t.Points[0].Y;
  int y2 = t.Points[0].Y;

  if (tt == TriangleType::FLAT_TOP)
  {
    y2 = t.Points[2].Y;

    for (int x = t.Points[0].X; x <= t.Points[1].X; x++)
    {
      SDL_RenderDrawPoint(_renderer, x, t.Points[0].Y);
    }
  }
  else if (tt == TriangleType::FLAT_BOTTOM)
  {
    y2 = t.Points[1].Y;

    for (int x = t.Points[2].X; x <= t.Points[1].X; x++)
    {
      SDL_RenderDrawPoint(_renderer, x, t.Points[1].Y);
    }
  }

  for (int currentScanline = y1; currentScanline <= y2; currentScanline++)
  {
    while (p1 != nullptr and p1->second == currentScanline)
    {
      SDL_RenderDrawPoint(_renderer, p1->first, p1->second);
      p1 = first.Next();
    }

    while (p2 != nullptr and p2->second == currentScanline)
    {
      SDL_RenderDrawPoint(_renderer, p2->first, p2->second);
      p2 = second.Next();
    }
  }
}
