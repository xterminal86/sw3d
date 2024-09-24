#ifndef BLG_H
#define BLG_H

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
    void Init(int x1, int y1, int x2, int y2);

    ///
    /// \brief Returns next point for current path or nullptr if end was reached.
    ///
    /// \return pointer to std::pair<int, int> containing next point values or
    /// nullptr if generator has fininshed.
    ///
    Point* Next();

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

#endif // BLG_H
