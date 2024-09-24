#include "blg.h"

void BLG::Init(int x1, int y1, int x2, int y2)
{
  _initialized = false;
  _done        = false;

  _xs = x1;
  _ys = y1;
  _xe = x2;
  _ye = y2;

  if(_ys > _ye)
  {
    std::swap(_xs, _xe);
    std::swap(_ys, _ye);
  }

  _goesRight = (_xe > _xs);

  _x = _xs;
  _y = _ys;

  _dx = std::abs(_xe - _xs);
  _dy = std::abs(_ye - _ys);

  _gentle = (_dy < _dx);

  if (_gentle)
  {
    std::swap(_dx, _dy);
  }

  _P = 2 * _dx - _dy;

  _initialized = true;
}

// =============================================================================

BLG::Point* BLG::Next()
{
  if (_done)
  {
    return nullptr;
  }

  if (not _gentle)
  {
    if (_y != _ye)
    {
      _point.first  = _x;
      _point.second = _y;

      _y++;

      if (_P < 0)
      {
        _P += 2 * _dx;
      }
      else
      {
        _P += 2 * _dx - 2 * _dy;
        _x = _goesRight ? (_x + 1) : (_x - 1);
      }
    }
    else
    {
      _point.first  = _x;
      _point.second = _y;

      _done = true;
    }
  }
  else
  {
    if (_x != _xe)
    {
      _point.first  = _x;
      _point.second = _y;

      _x = _goesRight ? (_x + 1) : (_x - 1);

      if (_P < 0)
      {
        _P += 2 * _dx;
      }
      else
      {
        _P += 2 * _dx - 2 * _dy;
        _y++;
      }
    }
    else
    {
      _point.first  = _x;
      _point.second = _y;

      _done = true;
    }
  }

  return &_point;
}
