#include <cstdint>
#include <cstdio>
#include <utility>
#include <cmath>

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
    }

    // -------------------------------------------------------------------------

    ///
    /// \brief Returns next point for current path or nullptr if end was reached.
    ///
    /// \return pointer to std::pair<int, int> containing next point values.
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

    bool _goesDown = false;
    bool _steep    = false;

    Point _point;

    bool _done = false;
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

  return 0;
}

