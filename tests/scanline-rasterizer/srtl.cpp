#include "srtl.h"

// =============================================================================

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
      y2 = t.Points[1].Y;
      break;

    case TriangleType::FLAT_TOP:
      y2 = t.Points[2].Y;
      break;
  }

  for (int currentScanline = y1; currentScanline < y2; currentScanline++)
  {
    //
    // Always get "first" point no matter the direction.
    //
    if (p1 != nullptr and p1->second == currentScanline)
    {
      x1 = p1->first;
    }

    while (p1 != nullptr and p1->second == currentScanline)
    {
      p1 = first.Next();
    }

    if (p2 != nullptr and p2->second == currentScanline)
    {
      x2 = p2->first;
    }

    while (p2 != nullptr and p2->second == currentScanline)
    {
      p2 = second.Next();
    }

    if (tt == TriangleType::FLAT_BOTTOM and currentScanline == y1)
    {
      continue;
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

  switch (tt)
  {
    case TriangleType::FLAT_BOTTOM:
      y2 = t.Points[1].Y;
      break;

    case TriangleType::FLAT_TOP:
      y2 = t.Points[2].Y;
      break;
  }

  for (int currentScanline = y1; currentScanline < y2; currentScanline++)
  {
    if (p1 != nullptr and p1->second == currentScanline)
    {
      x1 = p1->first;
    }

    while (p1 != nullptr and p1->second == currentScanline)
    {
      p1 = first.Next();
    }

    if (p2 != nullptr and p2->second == currentScanline)
    {
      x2 = p2->first;
    }

    while (p2 != nullptr and p2->second == currentScanline)
    {
      p2 = second.Next();
    }

    if (tt == TriangleType::FLAT_TOP)
    {
      if (currentScanline == y1)
      {
        for (int x = x1; x < x2; x++)
        {
          SDL_RenderDrawPoint(_renderer, x, currentScanline);
        }
      }
      else
      {
        SDL_RenderDrawPoint(_renderer, x1, currentScanline);
      }
    }
    else
    {
      SDL_RenderDrawPoint(_renderer, x1, currentScanline);
      SDL_RenderDrawPoint(_renderer, x2 - 1, currentScanline);
    }
  }

  if (tt == TriangleType::FLAT_BOTTOM)
  {
    for (int x = x1; x < x2; x++)
    {
      SDL_RenderDrawPoint(_renderer, x, y2 - 1);
    }
  }
}

