#include "srd.h"

void SRD::Init(SDL_Renderer* rendererRef)
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

void SRD::Rasterize(const TriangleSimple& t, bool wireframe)
{
  _wireframe = wireframe;

  _copy = t;

  _leftLineXByScanline.clear();
  _rightLineXByScanline.clear();

  SortVertices();
  CheckAndFixWinding();

  RasterizeImpl();
}

// =============================================================================

void SRD::RasterizeImpl()
{
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

void SRD::DrawVL()
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

void SRD::DrawHL()
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

void SRD::DrawFB(const TriangleSimple& t)
{
  //
  //    1     1                   1
  //
  //  3   2        3  2    3  2
  //

  _lineGen.Init(t.Points[0].X, t.Points[0].Y, t.Points[2].X, t.Points[2].Y);

  BLG::Point* p = _lineGen.Next();

  while (p != nullptr)
  {
    if (_leftLineXByScanline.count(p->second) == 0)
    {
      _leftLineXByScanline[p->second] = p->first;
    }

    p = _lineGen.Next();
  }

  // ---------------------------------------------------------------------------

  _lineGen.Init(t.Points[0].X, t.Points[0].Y, t.Points[1].X, t.Points[1].Y);

  p = _lineGen.Next();

  while (p != nullptr)
  {
    if (_rightLineXByScanline.count(p->second) == 0)
    {
      _rightLineXByScanline[p->second] = p->first;
    }

    p = _lineGen.Next();
  }

  // ---------------------------------------------------------------------------

  for (int y = t.Points[0].Y; y < t.Points[2].Y; y++)
  {
    int x1 = _leftLineXByScanline[y];
    int x2 = _rightLineXByScanline[y];

    if (_wireframe)
    {
      if (y == t.Points[0].Y)
      {
        for (int x = x1; x < x2; x++)
        {
          SDL_RenderDrawPoint(_renderer, x, y);
        }
      }
      else
      {
        SDL_RenderDrawPoint(_renderer, x1,     y);
        SDL_RenderDrawPoint(_renderer, x2 - 1, y);
      }
    }
    else
    {
      for (int x = x1; x < x2; x++)
      {
        SDL_RenderDrawPoint(_renderer, x, y);
      }
    }
  }
}

// =============================================================================

void SRD::DrawFT(const TriangleSimple& t)
{
  //
  //  1   2     1  2           1  2
  //
  //    3             3     3
  //

  _lineGen.Init(t.Points[0].X, t.Points[0].Y, t.Points[2].X, t.Points[2].Y);

  BLG::Point* p = _lineGen.Next();

  while (p != nullptr)
  {
    if (_leftLineXByScanline.count(p->second) == 0)
    {
      _leftLineXByScanline[p->second] = p->first;
    }

    p = _lineGen.Next();
  }

  // ---------------------------------------------------------------------------

  _lineGen.Init(t.Points[1].X, t.Points[1].Y, t.Points[2].X, t.Points[2].Y);

  p = _lineGen.Next();

  while (p != nullptr)
  {
    if (_rightLineXByScanline.count(p->second) == 0)
    {
      _rightLineXByScanline[p->second] = p->first;
    }

    p = _lineGen.Next();
  }

  // ---------------------------------------------------------------------------

  for (int y = t.Points[0].Y; y < t.Points[2].Y; y++)
  {
    int x1 = _leftLineXByScanline[y];
    int x2 = _rightLineXByScanline[y];

    if (_wireframe)
    {
      if (y == t.Points[0].Y)
      {
        for (int x = x1; x < x2; x++)
        {
          SDL_RenderDrawPoint(_renderer, x, y);
        }
      }
      else
      {
        SDL_RenderDrawPoint(_renderer, x1,     y);
        SDL_RenderDrawPoint(_renderer, x2 - 1, y);
      }
    }
    else
    {
      for (int x = x1; x < x2; x++)
      {
        SDL_RenderDrawPoint(_renderer, x, y);
      }
    }
  }
}

// =============================================================================

void SRD::DrawMR(const TriangleSimple& t)
{
  static BLG diag;
  diag.Init(t.Points[0].X, t.Points[0].Y, t.Points[1].X, t.Points[1].Y);

  BLG::Point* p = diag.Next();
  while (p != nullptr)
  {
    if (_rightLineXByScanline.count(p->second) == 0)
    {
      _rightLineXByScanline[p->second] = p->first;
    }

    p = diag.Next();
  }

  // ---------------------------------------------------------------------------

  //
  // Flat Bottom part
  //

  _lineGen.Init(t.Points[0].X, t.Points[0].Y, t.Points[2].X, t.Points[2].Y);

  p = _lineGen.Next();

  while (p != nullptr)
  {
    if (_leftLineXByScanline.count(p->second) == 0)
    {
      _leftLineXByScanline[p->second] = p->first;
    }

    p = _lineGen.Next();
  }

  for (int y = t.Points[0].Y; y < t.Points[2].Y; y++)
  {
    int x1 = _leftLineXByScanline[y];
    int x2 = _rightLineXByScanline[y];

    if (_wireframe)
    {
      if (y == t.Points[0].Y)
      {
        for (int x = x1; x < x2; x++)
        {
          SDL_RenderDrawPoint(_renderer, x, y);
        }
      }
      else
      {
        SDL_RenderDrawPoint(_renderer, x1,     y);
        SDL_RenderDrawPoint(_renderer, x2 - 1, y);
      }
    }
    else
    {
      for (int x = x1; x < x2; x++)
      {
        SDL_RenderDrawPoint(_renderer, x, y);
      }
    }
  }

  // ---------------------------------------------------------------------------

  //
  // Flat Top part
  //

  _leftLineXByScanline.clear();

  _lineGen.Init(t.Points[2].X, t.Points[2].Y, t.Points[1].X, t.Points[1].Y);

  p = _lineGen.Next();

  while (p != nullptr)
  {
    if (_leftLineXByScanline.count(p->second) == 0)
    {
      _leftLineXByScanline[p->second] = p->first;
    }

    p = _lineGen.Next();
  }

  for (int y = t.Points[2].Y; y < t.Points[1].Y; y++)
  {
    int x1 = _leftLineXByScanline[y];
    int x2 = _rightLineXByScanline[y];

    if (_wireframe)
    {
      if (y == t.Points[2].Y)
      {
        for (int x = x1; x < x2; x++)
        {
          SDL_RenderDrawPoint(_renderer, x, y);
        }
      }
      else
      {
        SDL_RenderDrawPoint(_renderer, x1,     y);
        SDL_RenderDrawPoint(_renderer, x2 - 1, y);
      }
    }
    else
    {
      for (int x = x1; x < x2; x++)
      {
        SDL_RenderDrawPoint(_renderer, x, y);
      }
    }
  }
}

// =============================================================================

void SRD::DrawML(const TriangleSimple& t)
{
  static BLG diag;
  diag.Init(t.Points[0].X, t.Points[0].Y, t.Points[2].X, t.Points[2].Y);

  BLG::Point* p = diag.Next();
  while (p != nullptr)
  {
    if (_leftLineXByScanline.count(p->second) == 0)
    {
      _leftLineXByScanline[p->second] = p->first;
    }

    p = diag.Next();
  }

  // ---------------------------------------------------------------------------

  //
  // Flat Bottom part
  //

  _lineGen.Init(t.Points[0].X, t.Points[0].Y, t.Points[1].X, t.Points[1].Y);

  p = _lineGen.Next();

  while (p != nullptr)
  {
    if (_rightLineXByScanline.count(p->second) == 0)
    {
      _rightLineXByScanline[p->second] = p->first;
    }

    p = _lineGen.Next();
  }

  for (int y = t.Points[0].Y; y < t.Points[1].Y; y++)
  {
    int x1 = _leftLineXByScanline[y];
    int x2 = _rightLineXByScanline[y];

    if (_wireframe)
    {
      if (y == t.Points[0].Y)
      {
        for (int x = x1; x < x2; x++)
        {
          SDL_RenderDrawPoint(_renderer, x, y);
        }
      }
      else
      {
        SDL_RenderDrawPoint(_renderer, x1,     y);
        SDL_RenderDrawPoint(_renderer, x2 - 1, y);
      }
    }
    else
    {
      for (int x = x1; x < x2; x++)
      {
        SDL_RenderDrawPoint(_renderer, x, y);
      }
    }
  }

  // ---------------------------------------------------------------------------

  //
  // Flat Top part
  //

  _rightLineXByScanline.clear();

  _lineGen.Init(t.Points[1].X, t.Points[1].Y, t.Points[2].X, t.Points[2].Y);

  p = _lineGen.Next();

  while (p != nullptr)
  {
    if (_rightLineXByScanline.count(p->second) == 0)
    {
      _rightLineXByScanline[p->second] = p->first;
    }

    p = _lineGen.Next();
  }

  for (int y = t.Points[1].Y; y < t.Points[2].Y; y++)
  {
    int x1 = _leftLineXByScanline[y];
    int x2 = _rightLineXByScanline[y];

    if (_wireframe)
    {
      if (y == t.Points[1].Y)
      {
        for (int x = x1; x < x2; x++)
        {
          SDL_RenderDrawPoint(_renderer, x, y);
        }
      }
      else
      {
        SDL_RenderDrawPoint(_renderer, x1,     y);
        SDL_RenderDrawPoint(_renderer, x2 - 1, y);
      }
    }
    else
    {
      for (int x = x1; x < x2; x++)
      {
        SDL_RenderDrawPoint(_renderer, x, y);
      }
    }
  }
}

// =============================================================================

void SRD::SortVertices()
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

void SRD::CheckAndFixWinding()
{
  WindingOrder wo = GetWindingOrder(_copy);
  if (wo == WindingOrder::CCW)
  {
    std::swap(_copy.Points[1].X, _copy.Points[2].X);
    std::swap(_copy.Points[1].Y, _copy.Points[2].Y);
  }
}

// =============================================================================

WindingOrder SRD::GetWindingOrder(const TriangleSimple& t)
{
  double cp = CrossProduct2D(t.Points[1] - t.Points[0],
                             t.Points[2] - t.Points[1]);

  return cp > 0 ? WindingOrder::CW : WindingOrder::CCW;
}

// =============================================================================

TriangleType SRD::GetTriangleType(const TriangleSimple& t)
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
