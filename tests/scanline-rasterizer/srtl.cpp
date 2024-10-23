#include "srtl.h"

void SRTL::PerformRasterization(BLG& first,
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

  for (int currentScanline = y1; currentScanline < y2; currentScanline++)
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

    for (int x = x1; x < x2; x++)
    {
      SDL_RenderDrawPoint(_renderer, x, currentScanline);
    }
  }
}

// =============================================================================

void SRTL::PerformRasterizationWireframe(BLG& first,
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

  for (int currentScanline = y1; currentScanline < y2; currentScanline++)
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

    if (currentScanline == y1)
    {
      for (int x = x1; x < x2; x++)
      {
        SDL_RenderDrawPoint(_renderer, x, currentScanline);
      }
    }
    else
    {
      SDL_RenderDrawPoint(_renderer, x1,     currentScanline);
      SDL_RenderDrawPoint(_renderer, x2 - 1, currentScanline);
    }
  }

  if (tt == TriangleType::FLAT_BOTTOM)
  {
    for (int x = x1; x < x2; x++)
    {
      SDL_RenderDrawPoint(_renderer, x, y2);
    }
  }
}

