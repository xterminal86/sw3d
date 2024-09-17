#include <cstdint>
#include <cstdio>
#include <utility>
#include <cmath>
#include <map>

class BLG
{
  public:
    using Point = std::pair<int, int>;

    ///
    /// \brief Initializes generator with start and end point coordinates.
    ///
    /// \param starting point X
    /// \param starting point Y
    /// \param end point X
    /// \param end point Y
    ///
    void Init(int x1, int y1, int x2, int y2)
    {
      _initialized = false;
      _solved      = false;
      _done        = false;

      _xs = x1;
      _ys = y1;
      _xe = x2;
      _ye = y2;

      if (_xs > _xe)
      {
        std::swap(_xs, _xe);
        std::swap(_ys, _ye);
      }

      _goesDown = (_ye > _ys);

      _x = _xs;
      _y = _ys;

      _dx = std::abs(_xe - _xs);
      _dy = std::abs(_ye - _ys);

      _steep = (_dy > _dx);

      if (_steep)
      {
        std::swap(_dx, _dy);
      }

      _P = 2 * _dy - _dx;

      _initialized = true;
    }

    // -------------------------------------------------------------------------

    ///
    /// \brief Returns next point for current path or nullptr if end was reached.
    ///
    /// \return pointer to std::pair<int, int> containing next point values or
    /// nullptr if generator has fininshed.
    ///
    Point* Next()
    {
      if (_done)
      {
        return nullptr;
      }

      if (not _steep)
      {
        if (_x != _xe)
        {
          _point.first  = _x;
          _point.second = _y;

          _x++;

          if (_P < 0)
          {
            _P += 2 * _dy;
          }
          else
          {
            _P += 2 * _dy - 2 * _dx;
            _y = _goesDown ? (_y + 1) : (_y - 1);
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
        if (_y != _ye)
        {
          _point.first  = _x;
          _point.second = _y;

          _y = _goesDown ? (_y + 1) : (_y - 1);

          if (_P < 0)
          {
            _P += 2 * _dy;
          }
          else
          {
            _P += 2 * _dy - 2 * _dx;
            _x++;
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

    // -------------------------------------------------------------------------

    //
    // Gather all line points across Y scanlines.
    //
    void GatherScanlines()
    {
      if (not _initialized or _solved)
      {
        return;
      }

      _lastXByScanline = std::map<int, int>();

      Point* p = Next();
      while (p != nullptr)
      {
        int xPos      = p->first;
        int yScanline = p->second;

        if (_lastXByScanline.count(yScanline) == 0)
        {
          _lastXByScanline[yScanline] = xPos;
        }
        else
        {
          //
          // Take last point across X if line goes down, and first point across
          // X if line goes up since our implementation draws line from bottom
          // to top basically.
          //
          bool lastRightX = (_goesDown
                         and xPos > _lastXByScanline[yScanline]);
          bool firstLeftX = (not _goesDown
                         and xPos < _lastXByScanline[yScanline]);

          if (lastRightX or firstLeftX)
          {
            _lastXByScanline[yScanline] = xPos;
          }
        }

        p = Next();
      }

      _solved = true;
    }

    // -------------------------------------------------------------------------

    const std::map<int, int>& GetScanlines()
    {
      return _lastXByScanline;
    }

    // -------------------------------------------------------------------------

  private:
    int _xs = 0;
    int _ys = 0;
    int _xe = 0;
    int _ye = 0;

    int _x = 0;
    int _y = 0;

    int _dx = 0;
    int _dy = 0;

    int _P = 0;

    bool _goesDown    = false;
    bool _steep       = false;
    bool _done        = false;
    bool _initialized = false;
    bool _solved      = false;

    Point _point;

    //
    // Proper X point across Y scanline depending on line direction.
    // I.e. max X for lines going down and min X for lines going up.
    //
    std::map<int, int> _lastXByScanline;
};


int main(int argc, char* argv[])
{
  auto Test = [](int x1, int y1, int x2, int y2)
  {
    printf("(%d, %d) - (%d, %d)\n\n", x1, y1, x2, y2);

    BLG bg;

    bg.Init(x1, y1, x2, y2);

    BLG::Point* p = bg.Next();
    while (p != nullptr)
    {
      printf("(%d, %d)\n", p->first, p->second);
      p = bg.Next();
    }

    printf("-----\n");
  };

  Test(0, 0, 10, 10);
  Test(10, 10, 0, 0);
  Test(10, 10, 20, 9);
  Test(20, 9, 10, 10);
  Test(10, 10, 20, 11);
  Test(20, 11, 10, 10);
  Test(10, 10, 20, 15);
  Test(10, 10, 20, 5);
  Test(10, 10, 15, 20);

  BLG bg;
  bg.Init(10, 10, 15, 20);
  bg.GatherScanlines();

  auto s = bg.GetScanlines();

  for (auto& kvp : s)
  {
    printf("scanline Y: %d, X = %d\n", kvp.first, kvp.second);
  }

  return 0;
}

