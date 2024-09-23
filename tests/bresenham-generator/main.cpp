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

    bool _goesRight   = false;
    bool _gentle      = false;
    bool _done        = false;
    bool _initialized = false;

    Point _point;
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

  return 0;
}

