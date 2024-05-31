#ifndef OLDSTUFF_H
#define OLDSTUFF_H

//
// Old hand-made drawing and scanline rasterization which doesn't work,
// but maybe I'll fix it sometime just to compare existing and this one.
//
// Well, actually I just couldn't delete everything because of lots of comments
// I wrote during research of this topic. )
//

enum class TriangleType
{
  UNDEFINED = -1,
  FLAT_TOP,
  FLAT_BOTTOM,
  COMPOSITE
};

TriangleType _triangleType = TriangleType::FLAT_TOP;

const std::unordered_map<TriangleType, std::string> _triangleTypeToString =
{
  { TriangleType::UNDEFINED,   "UNDEFINED"   },
  { TriangleType::FLAT_BOTTOM, "FLAT_BOTTOM" },
  { TriangleType::FLAT_TOP,    "FLAT_TOP"    },
  { TriangleType::COMPOSITE,   "COMPOSITE"   },
};
TriangleType GetTriangleType(int x1, int y1,
                             int x2, int y2,
                             int x3, int y3)
{
  //
  // There are 3 cases of 2D triangles.
  //
  // BASIC:
  // ------
  //
  // 1) Flat-bottom
  //
  //       1
  //
  //     2   3
  //
  // 2) Flat-top:
  //
  //     3   2
  //
  //       1
  //
  // SPECIAL:
  // --------
  //
  // 3) Composite, which comes in two forms:
  //
  //       1              1
  //
  //     2        ->    2   3
  //                      +
  //          3         3     2
  //
  //                            1
  //
  // Its mirror image:
  //
  //     1                1
  //
  //        3     ->    2   3
  //                      +
  //  2               3     2
  //
  //
  //                1
  //
  //
  // and which can be split into two basic ones.
  //
  bool isFlatBottom = ( (y2 == y3) and (y1 < y2) ) or
                      ( (y1 == y2) and (y3 < y2) ) or
                      ( (y3 == y1) and (y2 < y3) );

  bool isFlatTop = ( (y1 == y3) and (y2 > y1) ) or
                   ( (y3 == y2) and (y1 > y3) ) or
                   ( (y2 == y1) and (y3 > y2) );

  return isFlatTop ? TriangleType::FLAT_TOP
                   : isFlatBottom ? TriangleType::FLAT_BOTTOM
                   : TriangleType::COMPOSITE;
}

// -----------------------------------------------------------------------

void FillFlatBottomTriangle(int x1, int y1,
                            int x2, int y2,
                            int x3, int y3,
                            uint32_t colorMask,
                            bool includeLastPoint = true)
{
  //
  // For any type of winding one invSlope will be negative, one positive.
  //
  // You can do the same with "classic" slope as well, but then
  // you'll have to scanline vertically instead. I left the original
  // algorithm I found in the Internet, since it's more natural to think
  // of scanlines in terms of a line going from left to right,
  // top to bottom, like in CRT television.
  //
  // Since we're controlling y here, we have to find out next X coordinate
  // when Y has increased by one. That's why we use inverted slope.
  // Because k*x is the same as (k + k + k + k) x times, we can see that
  // by solving for x we'll get x = y/k. So we can calculate 1/k
  // beforehand and then just add it in every iteration of the loop.
  //
  // Since:
  //
  // y
  //
  // ^
  // |     * p2
  // |    /
  // |   /
  // |  /
  // | * p1
  // |
  // +----------------------> x
  //
  // p1: (x1, y1), p2: (x2, y2)
  //
  // dy = (y2 - y1)
  // dx = (x2 - x1)
  //
  //      dy      1     1       1     dx
  // k = ----,   --- = ----,   --- = ----
  //      dx      k     dy      k     dy
  //                   ----
  //                    dx
  //
  // In case of our triangle on the screen:
  //
  // +-----------------------> x
  // |
  // |
  // |            p1 (x1, y1)
  // |            / \
  // |           /   \
  // |          /     \
  // | (x2, y2)p2-----p3 (x3, y3)
  // |
  // |
  // ▼
  // y
  //
  // It's easy to see, that one invSlope will be negative, thus no need
  // to bother with + or - during increment, it will be handled
  // automatically in += depending on the sign of the term.
  //
  double invSlope1 = (double)(x2 - x1) / (double)(y2 - y1);
  double invSlope2 = (double)(x3 - x1) / (double)(y3 - y1);

  double curx1 = x1;
  double curx2 = x1;

  for (int scanline = y1;
       (includeLastPoint ? scanline <= y3 : scanline < y3);
       scanline++)
  {
    for (int lineX = (int)curx1; lineX <= (int)curx2; lineX++)
    {
      DrawPoint({ lineX, scanline }, colorMask);
    }

    curx1 += invSlope1;
    curx2 += invSlope2;
  }
}

// -----------------------------------------------------------------------

void FillFlatTopTriangle(int x1, int y1,
                         int x2, int y2,
                         int x3, int y3,
                         uint32_t colorMask,
                         bool includeLastPoint = true)
{
  //
  // The idea is exactly the same as above,
  // just in this case we go from bottom to top.
  //
  double invSlope1 = (double)(x2 - x1) / (double)(y2 - y1);
  double invSlope2 = (double)(x3 - x1) / (double)(y3 - y1);

  double curx1 = x1;
  double curx2 = x1;

  //
  // Bottom-up from lowest point.
  //
  for (int scanline = y1;
       (includeLastPoint ? scanline >= y3 : scanline > y3);
       scanline--)
  {
    for (int lineX = (int)curx2; lineX <= (int)curx1; lineX++)
    {
      DrawPoint({ lineX, scanline }, colorMask);
    }

    //
    // And since we go up, we need to decrement y.
    // Again, the sign will be handled automatically since this is
    // basically the mirror image operation of the flat bottom triangle.
    //
    curx1 -= invSlope1;
    curx2 -= invSlope2;
  }
}

// -----------------------------------------------------------------------

void SwapCoords(int& x1, int& y1,
                int& x2, int& y2,
                int& x3, int& y3,
                TriangleType tt)
{
  //
  // There are two main methods of specifying the order of vertices:
  // ClockWise (CW) and Counter ClockWise (CCW). Its main purpose
  // is to determine which faces are considered "front" faces,
  // i.e. the ones that should be drawn.
  //
  // For a simple drawing of a 2D filled triangle on the screen
  // winding doesn't matter, but it will matter if you plan to do
  // shading and back face culling for example.
  // At design phase you can choose CW or CCW winding order,
  // it won't make a difference for the end result, as long as you're
  // consistent with it.
  //
  // We'll be using CCW in this project.
  //
  // So we'll be assuming that the user is responsible for supplying
  // vertices in the correct winding order for the object.
  // But it may be possible to specify points in the correct winding order
  // but different relative order. Our rasterization algorithms assume
  // that triangle points have *specific* relative order, so if user
  // specified points in the wrong relative order, rasterization will fail.
  // So we need to account for that by swapping the corresponding coordinates.
  // Finding out if relative order of points was wrong is just a matter of
  // bruteforcing the coordinates' location against one another.
  //
  switch (tt)
  {
    //
    // Proper order:
    //
    //       1
    //
    //     2   3
    //
    case TriangleType::FLAT_BOTTOM:
    {
      //
      //     3          1
      //          ->
      //   1   2      2   3
      //
      if ( (x1 < x2) and (y1 > y3) )
      {
        std::swap(x1, x2);
        std::swap(y1, y2);
        std::swap(x1, x3);
        std::swap(y1, y3);
      }
      //
      //     2          1
      //          ->
      //   3   1      2   3
      //
      else if ( (x3 < x1) and (y3 > y2) )
      {
        std::swap(x1, x3);
        std::swap(y1, y3);
        std::swap(x1, x2);
        std::swap(y1, y2);
      }
    }
    break;

    //
    // Proper order:
    //
    //    3   2
    //
    //      1
    //
    case TriangleType::FLAT_TOP:
    {
      //
      //   2   1     3   2
      //          ->
      //     3         1
      //
      if ( (x1 > x2) and (y1 < y3) )
      {
        std::swap(x1, x2);
        std::swap(y1, y2);
        std::swap(x1, x3);
        std::swap(y1, y3);
      }
      //
      //   1   3     3   2
      //          ->
      //     2         1
      //
      else if ( (x3 > x1) and (y2 > y3) )
      {
        std::swap(x1, x2);
        std::swap(y1, y2);
        std::swap(x2, x3);
        std::swap(y2, y3);
      }
    }
    break;

    //
    // Since we'll be splitting this triangle in two later on there is
    // no proper relative order for this one, but we can prepare certain
    // relative order beforehand just to make everything sort of standard.
    // So when we'll be splitting this in two later we can be sure that
    // point 1 is always at the top, point 2 is to the left and point 3 is
    // to the right.
    //
    case TriangleType::COMPOSITE:
    {
      //
      //
      //      3           1
      //
      //    1      ->   2
      //
      //         2           3
      //
      if ( (y1 > y3) and (y1 < y2) )
      {
        std::swap(x1, x3);
        std::swap(y1, y3);
        std::swap(x2, x3);
        std::swap(y2, y3);
      }
      //
      //      2           1
      //
      //    3      ->   2
      //
      //         1           3
      //
      else if ( (y3 > y2) and (y3 < y1) )
      {
        std::swap(x1, x2);
        std::swap(y1, y2);
        std::swap(x2, x3);
        std::swap(y2, y3);
      }
      //
      //      3           1
      //
      //        2  ->       3
      //
      //   1           2
      //
      else if ( (y1 > y2) and (y2 > y3) )
      {
        std::swap(x1, x3);
        std::swap(y1, y3);
        std::swap(x2, x3);
        std::swap(y2, y3);
      }
      //
      //      2           1
      //
      //        1  ->       3
      //
      //   3           2
      //
      else if ( (y3 > y1) and (y1 > y2) )
      {
        std::swap(x1, x2);
        std::swap(y1, y2);
        std::swap(x2, x3);
        std::swap(y2, y3);
      }
    }
    break;
  }
}

// -----------------------------------------------------------------------

void FillTriangleOld(int x1, int y1,
                     int x2, int y2,
                     int x3, int y3,
                     uint32_t colorMask)
{
  TriangleType tt = GetTriangleType(x1, y1, x2, y2, x3, y3);

  if (_triangleType != tt)
  {
    printf("Before: %s\n", _triangleTypeToString.at(_triangleType).data());
    _triangleType = tt;
  }

  SwapCoords(x1, y1, x2, y2, x3, y3, tt);

  switch (tt)
  {
    case TriangleType::FLAT_BOTTOM:
      FillFlatBottomTriangle(x1, y1, x2, y2, x3, y3, colorMask);
      break;

    case TriangleType::FLAT_TOP:
      FillFlatTopTriangle(x1, y1, x2, y2, x3, y3, colorMask);
      break;

    case TriangleType::COMPOSITE:
    {
      int x4;
      int y4;

      if (y3 > y2)
      {
        //
        // In order to split composite triangle in two we need to find out
        // the x coordinate of the new point that lies on the longest side
        // (it's written as 4 here).
        //
        // x4 is derived from Intercept Theorem:
        //
        // "The ratio of the two segments on the same ray starting at S
        // equals the ratio of the segments on the parallels:"
        //
        // So, given this:
        //
        // +---------------> x
        // |
        // |        1
        // |
        // |     2  z  4
        // |
        // |        w    3
        // ▼
        // y
        //
        // the following holds true:
        //
        // [1 to z]   [z to 4]
        // -------- = --------
        // [1 to w]   [w to 3]
        //
        // which gives us:
        //
        // (y2 - y1)   (x4 - x1)
        // --------- = ---------
        // (y3 - y1)   (x3 - x1)
        //
        //
        // Solving for x4 yields:
        //
        // (y2 - y1)
        // --------- * (x3 - x1) = (x4 - x1)
        // (y3 - y1)
        //
        //           (y2 - y1)
        // x4 = x1 + --------- * (x3 - x1)
        //           (y3 - y1)
        //

        x4 = x1 + ( (double)(y2 - y1) / (double)(y3 - y1) ) * (x3 - x1);
        y4 = y2;

        FillFlatBottomTriangle(x1, y1, x2, y2, x4, y4, colorMask, false);
        FillFlatTopTriangle(x3, y3, x4, y4, x2, y2, colorMask);
      }
      else if (y2 > y3)
      {
        //
        // For mirror-image situation the process is the same.
        //
        // +------------> x
        // |
        // |        1
        // |
        // |     4  z  3
        // |
        // |   2    w
        // |
        // ▼
        // y
        //
        // [1 to z]   [z to 4]
        // -------- = --------
        // [1 to w]   [w to 3]
        //
        // (y3 - y1)   (x1 - x4)
        // --------- = ---------
        // (y2 - y1)   (x1 - x2)
        //
        //             (y3 - y1)
        // (x1 - x2) * --------- = (x1 - x4)
        //             (y2 - y1)
        //
        //           (y3 - y1)
        // x4 = x1 - --------- * (x1 - x2)
        //           (y2 - y1)
        //

        x4 = x1 - ( (double)(y3 - y1) / (double)(y2 - y1) ) * (x1 - x2);
        y4 = y3;

        FillFlatBottomTriangle(x1, y1, x4, y4, x3, y3, colorMask, false);
        FillFlatTopTriangle(x2, y2, x3, y3, x4, y4, colorMask);
      }
    }
    break;
  }
}

#endif // OLDSTUFF_H
