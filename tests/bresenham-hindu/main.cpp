#include "sw3d.h"
#include "instant-font.h"

const size_t QualityReductionFactor = 10;

int X1 = 10;
int Y1 = 5;
int X2 = 50;
int Y2 = 20;

int xStart = 0;
int yStart = 0;
int xEnd   = 0;
int yEnd   = 0;

bool ShowText = true;

bool ShowHindu   = true;
bool ShowCorrect = false;
bool ShowDumb    = false;

bool gSteep = false;

bool VerticalMode = false;

using namespace SW3D;

class BH : public DrawWrapper
{
  public:
    //
    // Straight implementation.
    //
    void LineDumb(int sx, int sy, int ex, int ey)
    {
      int dy = ey - sy;
      int dx = ex - sx;

      double k = 0;

      if (dx != 0)
      {
        k = (double)dy / (double)dx;
      }

      double b = (double)ey - (double)ex * k;

      SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);

      for (int x = sx; x <= ex; x++)
      {
        int y = (int)(k * x + b);
        SDL_RenderDrawPoint(_renderer, x, y);
      }
    }

    // -------------------------------------------------------------------------

    //
    // "Classic" implementation that uses error variable.
    //
    void BresenhamCorrect(int sx, int sy, int ex, int ey)
    {
      int x1 = sx;
      int y1 = sy;
      int x2 = ex;
      int y2 = ey;

      const bool steep = std::abs(y2 - y1) > std::abs(x2 - x1);

      gSteep = steep;

      //
      // If line is steep make it gentle by swapping components.
      //
      if(steep)
      {
        std::swap(x1, y1);
        std::swap(x2, y2);
      }

      //
      // Sort by X so that x1 is always the leftmost.
      //
      if(x1 > x2)
      {
        std::swap(x1, x2);
        std::swap(y1, y2);
      }

      xStart = x1;
      yStart = y1;

      xEnd = x2;
      yEnd = y2;

      const double dx = x2 - x1;
      const double dy = std::abs(y2 - y1);

      //
      // Set error to be half the distance across X between points.
      // Why 2.0 exactly? To me it's unclear but in my opinion the motivation
      // is this: suppose we want to draw a line from (0, 0) to (20, 1).
      // We would like our line to go across Y = 0 for exactly half the distance
      // between X1 and X2 before going up 1 pixel. So that's why we use half
      // the distance.
      //
      double error = dx / 2.0;

      //
      // Draw direction depending on whether line goes up or down.
      //
      const int ystep = (y1 < y2) ? 1 : -1;

      int y = (int)y1;

      const int maxX = (int)x2;

      SDL_SetRenderDrawColor(_renderer, 0, 255, 0, 255);

      for(int x = (int)x1; x < maxX; x++)
      {
        //
        // If line was steep we swapped components in the code above but at the
        // drawing phase here we will use component values "backwards" since our
        // swapped version of the line is a mirror image.
        //
        if(steep)
        {
          SDL_RenderDrawPoint(_renderer, y, x);
        }
        else
        {
          SDL_RenderDrawPoint(_renderer, x, y);
        }

        //
        // Again, beats me why you must subtract dy and not something else.
        //
        error -= dy;

        //
        // If line is steep its dy will be greater than dx meaning that error
        // will become negative. This signifies that we need to go "up" or
        // "down" depending on line direction (ystep is already set accordingly).
        // Again, one should think that we're dealing with gentle sloped line
        // here because we will be using mirrored values for x and y (see above).
        //
        if(error < 0)
        {
          y += ystep;
          error += dx;
        }
      }
    }

    // -------------------------------------------------------------------------

    //
    // https://www.youtube.com/watch?v=RGB-wlatStc
    //
    void BresenhamHindu(int xs, int ys, int xe, int ye)
    {
      //
      // -----------------------------------------------------------------------
      //
      // There are several implementations of Bresenham algorithm, the one that
      // people probably see for the first time is not very clear because it
      // uses some weird variable called error (implemented above). So I found
      // out better explanation that is very straightforward (see link) and
      // doesn't use floating point values. Most of the time it probably doesn't
      // matter which implementation is used because they all achieve more or
      // less the same end result and have no impact on performance with modern
      // hardware.
      //
      // -----------------------------------------------------------------------
      //
      // Consider a line (x1, y1) - (x2, y2):
      //
      //   y
      //   ^
      //   |
      //   |
      //   |
      //   |
      // y2|            .
      //   |            |
      //   |            | y2 - y1 = dy
      // y1| .__________|
      //   |    x2 - x1 = dx
      //  -|--------------> x
      //   | x1         x2
      //
      // Line equation is y = kx + C.
      //
      // Let's recall SOHCAHTOA, where Tangent = Opposite / Adjacent.
      // So it's easy to see that in line equation k = dy / dx = tan(a).
      // It's obvious that if k = 1 then a = 45 deg. If k < 1 then angle is less
      // than 45 degrees, and if k > 1 then it's greater than 45.
      //
      // Basic idea of Bresenham algorithm is to determine which pixel to fill
      // in case when our true line intersects both of them. Like this (zoomed
      // in and not to scale):
      //
      //                           * (x2, y2)
      //                         /
      //      ----- ----- ---- /
      //     |     |     |   / |
      //     |  1  |  2  | /3  |
      //     |     |     /     |
      //      ----- -- /- -----
      //     |     | /   |     |
      //     |  4  /  5  |  6  |
      //     |   / |     |     |
      //      -/--- ----- -----
      //     /
      //   * (x1, y1)
      //
      // Here true line crosses pixels 2 and 5 and we need to choose which one
      // to fill. The main idea of algorithm is to fill the one whose distance
      // to the "temporary pixel" (let's called it T) is smaller. So let's
      // imagine that there is another pixel in between pixels 2 and 5 and our
      // line crosses it. Let's zoom in that part (again, not to scale because
      // ASCII art):
      //
      //         -----    -----
      //        |     |  |     |
      // Yk + 1 |  1  |  |  2  |    /
      //        |     |  |     |  /
      //         -----    ----- /  --
      //                 |    /|     | d2
      //   Yt            |  T  |   --
      //                 |/    |     | d1
      //         -----  / -----    --
      //        |     /  |     |
      //   Yk   |  4/ |  |  5  |
      //        | /   |  |     |
      //         -----    -----
      //           Xk       Xk + 1
      //
      // Now we just need to find distance between 5 to T (d1) and 2 to T (d2),
      // and choose the one that is smaller. So if (d1 - d2) < 0 then we should
      // turn pixel 5 on, otherwise if (d1 - d2) >= 0 it's pixel 2.
      //
      // One needs not to be deceived by everything looking the same size on the
      // illustration (lol) above: line can cross pixels with a slope much
      // closer to a line parallel to X axis, so, for example, our T could be
      // closer to pixel 5 than it is portrayed on the picture.
      // For example, consider our true line equation to be y = 0.25x
      // For x = 9 we will have y = 2.25, which is 0.25 away from pixel center
      // at (9, 2) and 0.75 from pixel (9, 3).
      //
      // For simplicity sake let's consider that our line has k < 1 for now.
      // For k >= 1 it's just a matter of swapping some variables (more on that
      // later).
      //
      // To draw a line we will iterate from x1 to x2. Since k < 1 we have more
      // points across X axis, so our x will always increase when we traverse
      // from x1 to x2, but whether y should increase or not given any x sample
      // - that's what we'll have to decide.
      //
      // So it's time for some math:
      //
      // Xnext = Xk + 1
      //
      // We're interested in what Y we should choose for point Xk + 1, whether
      // it will remain Yk or it will be Yk + 1.
      //
      //           - Yk
      // Ynext -> |
      //           - Yk + 1
      //
      // Our line equation for point T is this:
      //
      // Yt = k(Xk + 1) + C
      //
      // Let's find out d1 and d2:
      //
      // d1 = Yt - Yk
      // d2 = (Yk + 1) - Yt
      //
      // Now let's plug in Yt:
      //
      // d1 = k(Xk + 1) + C - Yk
      // d2 = (Yk + 1) - [ k(Xk + 1) + C ] = Yk + 1 - k(Xk + 1) - C
      //
      // If (d1 - d2) < 0:
      //   then our value Y for this X sample should remain as Yk.
      //
      // If (d1 - d2) >= 0:
      //   then our value Y for this X sample should be Yk + 1.
      //
      // Let's actually subtract d1 and d2:
      //
      // (d1 - d2) = [ k(Xk + 1) + C - Yk ] - [ Yk + 1 - k(Xk + 1) - C ]
      //
      // Open up brackets:
      //
      // (d1 - d2) = k(Xk + 1) + C - Yk - Yk - 1 + k(Xk + 1) + C
      //             =========   ~ ---- ----       =========   ~
      //
      // Group things:
      //
      // (d1 - d2) = 2k(Xk + 1) - 2Yk + 2C - 1
      //
      // We still have k in our equation which is dy / dx. To get rid of
      // floating point calculation we just multiply the whole thing by dx:
      //
      //                             dy
      // dx * (d1 - d2) = dx * ( 2 * -- (Xk + 1) - 2Yk + 2C - 1)
      //                             dx
      //
      // dx * (d1 - d2) = 2dy(Xk + 1) - 2dxYk + 2dxC - dx
      //
      // Let's call this whole thing "decision parameter" and call it Pk.
      //
      // dx * (d1 - d2) = 2dyXk + 2dy - 2dxYk + 2dxC - dx = Pk
      //                          ~~~           ~~~~   ~~
      // We can observe that highlighted variables are constant. For some reason
      // that is not very clear to me we can discard them as irrelevant, because
      // we're only interested in terms that depend on Xk and Yk.
      //
      // So we're left with:
      //
      // Pk = 2dyXk - 2dxYk
      //
      // Now let's write Pk for general case:
      //
      // Pnext = 2dyXnext - 2dxYnext
      //
      // Xnext is always Xk + 1 because, like it was mentioned above, we're
      // currently dealing with a line with k < 1. But value of Ynext we'll have
      // to decide. By subtracting (Pnext - Pk) we can find out how much a
      // decision variable should change at every iteration.
      //
      // (Pnext - Pk) = [ 2dyXnext - 2dxYnext ] - [ 2dyXk - 2dxYk ]
      // (Pnext - Pk) = 2dyXnext - 2dxYnext - 2dyXk + 2dxYk
      //
      // Factor out stuff:
      //
      // (Pnext - Pk) = 2dy * (Xnext - Xk) - 2dx * (Ynext - Yk)
      //
      // If (Pnext - Pk) < 0 then we should remain on the same Yk,
      // so Ynext = Yk. X is always increasing, so Xnext = Xk + 1:
      //
      // Pnext = Pk + 2dy(Xk + 1 - Xk) - 2dx * (Yk - Yk)
      //
      // Cancel out Xk and Yk:
      //
      // +------------------+
      // | Pnext = Pk + 2dy |
      // +------------------+
      //
      // If (Pnext - Pk) >= 0 then Ynext = Yk + 1. Xnext is still Xk + 1:
      //
      // Pnext = Pk + 2dy(Xk + 1 - Xk) - 2dx * (Yk + 1 - Yk)
      //
      // Again, shit gets cancelled:
      //
      // +------------------------+
      // | Pnext = Pk + 2dy - 2dx |
      // +------------------------+
      //
      // The only thing left is to calculate initial value for Pk. Recall:
      //
      // Pk = 2dyXk + 2dy - 2dxYk + 2dxC - dx
      //
      // This time for calculation of initial value we will use full form of the
      // equation. But we need to remove constant C. To do that we can express C
      // from line equation:
      //
      // y = kx + C            dy
      // C = y - kx or C = y - -- x
      //                       dx
      //
      // Let's plug it in the formula for Pk above:
      //                                     dy
      // Pk = 2dyXk + 2dy - 2dxYk + 2dx[ y - -- x ] - dx
      //                                     dx
      //
      // For starting point let's call x = X1 and y = Y1 and also open up the
      // brackets:
      //
      // Pk1 = 2dyX1 + 2dy - 2dxY1 + 2dxY1 - 2dyX1 - dx
      //       ~~~~~       ~~~~~~~   ~~~~~   ~~~~~~~
      // +----------------+
      // | Pk1 = 2dy - dx |
      // +----------------+
      //
      // Now we have everything we need to actually implement the algorithm.
      //
      // ============================================================================
      //
      // Before we start let's recall that explanation above was done for a line
      // with k < 1 and I mentioned that "more on that later".
      //
      // Well, *now* is "later".
      //
      // First let's consider the following two equations:
      //
      // 1) y = 10x    2) y = 0.1x
      //
      // It's obvious to see that these lines are mirrored across axis y = x and
      // in order to create mirrored line we just need to invert k, i.e. k = 1/k
      // or 1 / (dy / dx) = dx / dy
      //
      // So, whenever we have a line with k > 1 we can convert it to the
      // mirrored version which will have k < 1 by swapping y from 1) as x for
      // 2):
      //
      //     1)              2)
      //
      //  x = 0.01    +--> x = 0.1
      //              |
      //  y = 0.1  ---+    y = 0.01
      //
      //  (0.01, 0.1)      (0.1, 0.01)
      //
      // Thus we kinda reduced the problem to already solved one.
      //
      // Now notice that when k < 1 it means that we have more X values than Y
      // and Y values are the ones that we need to determine whether to
      // increment or not during rasterization loop. So by checking the k value
      // beforehand we can create two execution branches that will increment
      // either X constantly and determine Y or vice versa. How exactly? If we
      // have a line with k > 1 we mirror it and then use the same calculations
      // as usual but when we actually draw stuff we will iterate over Y axis
      // instead of X and increment X (or not) instead of Y, and since
      // everything is a mirror image it will all work out nicely.
      //
      // Edge cases when line is vertical (dx = 0) and horizontal (dy = 0) can
      // be handled separately, but we actually don't need to do that since at
      // those cases everything will work in the code by itself.
      //
      // This leaves the last question: direction of a line.
      //
      // If we sort points by X we can then have only two cases:
      //
      // 1) line goes up
      // 2) line goes down
      //
      // So by finding out which type of line is our case we can determine
      // whether we need to increment Y or decrement it.
      //
      // ============================================================================

      int x1 = xs;
      int y1 = ys;
      int x2 = xe;
      int y2 = ye;

      //
      // Sort by X so that starting point is always the left one.
      //
      if (x1 > x2)
      {
        std::swap(x1, x2);
        std::swap(y1, y2);
      }

      xStart = x1;
      yStart = y1;

      xEnd = x2;
      yEnd = y2;

      bool goesDown = (y2 > y1);

      //
      // These will be our pixel coordinates to fill.
      // Prime them with starting point.
      //
      int x = x1;
      int y = y1;

      int dx = std::abs(x2 - x1);
      int dy = std::abs(y2 - y1);

      //
      // Let's try to avoid any floating point calculation: if k > 1 then it
      // means that dy is greater than dx, so our line slope is "steep", that is
      // its angle is greater than 45 degrees. Otherwise we'll consider line
      // slope to be "gentle".
      //
      bool steep = (dy > dx);

      //
      // If line is steep convert it to gentle by inverting k.
      //
      if (steep)
      {
        std::swap(dx, dy);
      }

      //
      // Initial value for Pk (Pk1 in the longread above).
      //
      int P = 2 * dy - dx;

      SDL_SetRenderDrawColor(_renderer, 255, 0, 0, 255);

      //
      // Gentle slope, everything goes as planned.
      //
      if (not steep)
      {
        //
        // This will always work because points are sorted by X.
        //
        while (x != x2)
        {
          SDL_RenderDrawPoint(_renderer, x, y);

          x++;

          //
          // Our decision parameter.
          //
          // If line is horizontal (dy = 0) we will always go into (P < 0)
          // branch thus not affecting Y at all (check that initial value of
          // P = 2 * dy - dx).
          //
          if (P < 0)
          {
            P += 2 * dy;
          }
          else
          {
            P += 2 * dy - 2 * dx;
            y = goesDown ? (y + 1) : (y - 1);
          }
        }
      }
      else
      {
        //
        // If line is steep just swap stuff around. This time we're going across
        // Y axis and must determine whether to increase X or not. Notice that
        // decision parameter calculation remained the same but it actually is
        // working on mirrored line but doesn't know about it. :-)
        //
        while (y != y2)
        {
          SDL_RenderDrawPoint(_renderer, x, y);

          //
          // This time Y is always incremented (or decremented, depending on
          // line direction).
          //
          y = goesDown ? (y + 1) : (y - 1);

          //
          // Same here, if line is vertical (dx = 0), we will go into this
          // branch (remember, that dx and dy get swapped if line is steep).
          //
          if (P < 0)
          {
            P += 2 * dy;
          }
          else
          {
            P += 2 * dy - 2 * dx;
            x++;
          }
        }
      }
    }

    // -------------------------------------------------------------------------

    //
    // For scanline rasterization it is actually more convenient to go from
    // top to bottom. We could use existing implementation and put all points
    // inside some kind of container, like std::map by Y position, and use that,
    // but let's pretend that we're trying to write optimal code for drawing
    // and try to avoid unnecessary allocations and stuff. By having "vertical"
    // implementation we will use it inside rasterizer along with its
    // calculation to determine our scanline endpoints.
    //
    // The implementation is basically the same as "horizontal" one, just
    // everything gets swapped around. You could check it out yourself
    // by manually deriving the same stuff, but this time your Y will always
    // increase instead of X. I'll leave it as an exercise for the reader. :-)
    //
    void BresenhamVertical(int sx, int sy, int ex, int ey)
    {
      int x1 = sx;
      int y1 = sy;
      int x2 = ex;
      int y2 = ey;

      //
      // This time sort by Y.
      //
      if(y1 > y2)
      {
        std::swap(x1, x2);
        std::swap(y1, y2);
      }

      xStart = x1;
      yStart = y1;

      xEnd = x2;
      yEnd = y2;

      //
      // Now our "base" case is a steep line going down.
      // It can either go right or left.
      //
      bool goesRight = (x2 > x1);

      int x = x1;
      int y = y1;

      int dx = std::abs(x2 - x1);
      int dy = std::abs(y2 - y1);

      //
      // Now our mirror case is a gentle line instead of a steep one.
      //
      bool gentle = (dy < dx);

      if (gentle)
      {
        std::swap(dx, dy);
      }

      //
      // Initial value for Pk
      //
      int P = 2 * dx - dy;

      SDL_SetRenderDrawColor(_renderer, 255, 0, 0, 255);

      //
      // Let's hope everything works by this whole big swapping rule...
      //
      if (not gentle)
      {
        while (y != y2)
        {
          SDL_RenderDrawPoint(_renderer, x, y);

          y++;

          if (P < 0)
          {
            P += 2 * dx;
          }
          else
          {
            P += 2 * dx - 2 * dy;
            x = goesRight ? (x + 1) : (x - 1);
          }
        }
      }
      else
      {
        while (x != x2)
        {
          SDL_RenderDrawPoint(_renderer, x, y);

          x = goesRight ? (x + 1) : (x - 1);

          if (P < 0)
          {
            P += 2 * dx;
          }
          else
          {
            P += 2 * dx - 2 * dy;
            y++;
          }
        }
      }
    }

    // -------------------------------------------------------------------------

    void DrawToFrameBuffer() override
    {
      SaveColor();

      if (not VerticalMode)
      {
        if (ShowHindu)
        {
          BresenhamHindu(X1, Y1, X2, Y2);
        }

        if (ShowCorrect)
        {
          BresenhamCorrect(X1, Y1, X2, Y2);
        }

        if (ShowDumb)
        {
          LineDumb(X1, Y1, X2, Y2);

          SDL_SetRenderDrawColor(_renderer, 0, 255, 255, 255);
          SDL_RenderDrawPoint(_renderer, X1, Y1);

          SDL_SetRenderDrawColor(_renderer, 255, 0, 255, 255);
          SDL_RenderDrawPoint(_renderer, X2, Y2);
        }
        else
        {
          SDL_SetRenderDrawColor(_renderer, 0, 255, 255, 255);
          SDL_RenderDrawPoint(_renderer, xStart, yStart);

          SDL_SetRenderDrawColor(_renderer, 255, 0, 255, 255);
          SDL_RenderDrawPoint(_renderer, xEnd, yEnd);
        }
      }
      else
      {
        BresenhamVertical(X1, Y1, X2, Y2);

        SDL_SetRenderDrawColor(_renderer, 0, 255, 255, 255);
        SDL_RenderDrawPoint(_renderer, xStart, yStart);

        SDL_SetRenderDrawColor(_renderer, 255, 0, 255, 255);
        SDL_RenderDrawPoint(_renderer, xEnd, yEnd);
      }

      RestoreColor();
    }

    // -------------------------------------------------------------------------

    void DrawToScreen() override
    {
      if (not VerticalMode)
      {
        if (ShowText)
        {
          IF::Instance().Printf(X1 * QualityReductionFactor,
                                Y1 * QualityReductionFactor - 4 * QualityReductionFactor,
                                IF::TextParams::Set(0xFFFFFF,
                                                    IF::TextAlignment::LEFT,
                                                    2.0),
                                "(%d, %d)",
                                X1, Y1);

          IF::Instance().Printf(X2 * QualityReductionFactor,
                                Y2 * QualityReductionFactor - 4 * QualityReductionFactor,
                                IF::TextParams::Set(0xFFFFFF,
                                                    IF::TextAlignment::LEFT,
                                                    2.0),
                                "(%d, %d)",
                                X2, Y2);

          IF::Instance().Printf(xStart * QualityReductionFactor,
                                yStart * QualityReductionFactor +
                                4 * QualityReductionFactor,
                                IF::TextParams::Set(0xFFFFFF,
                                                    IF::TextAlignment::LEFT,
                                                    2.0),
                                "1");

          IF::Instance().Printf(xEnd * QualityReductionFactor,
                                yEnd * QualityReductionFactor +
                                4 * QualityReductionFactor,
                                IF::TextParams::Set(0xFFFFFF,
                                                    IF::TextAlignment::LEFT,
                                                    2.0),
                                "2");

          if (ShowCorrect and not ShowHindu and gSteep)
          {
            IF::Instance().Printf(xStart * QualityReductionFactor +
                                  2 * QualityReductionFactor,
                                  yStart * QualityReductionFactor,
                                  IF::TextParams::Set(0xFFFF00,
                                                      IF::TextAlignment::LEFT,
                                                      2.0),
                                  "(%d, %d)",
                                  xStart, yStart);

            IF::Instance().Printf(xEnd * QualityReductionFactor +
                                  2 * QualityReductionFactor,
                                  yEnd * QualityReductionFactor,
                                  IF::TextParams::Set(0xFFFF00,
                                                      IF::TextAlignment::LEFT,
                                                      2.0),
                                  "(%d, %d)",
                                  xEnd, yEnd);
          }

          IF::Instance().Printf(0,
                                _windowHeight - 20 * 5,
                                IF::TextParams::Set(0xFFFFFF,
                                                    IF::TextAlignment::LEFT,
                                                    2.0),
                                "'1' - Hindu Bresenham");

          IF::Instance().Printf(0,
                                _windowHeight - 20 * 4,
                                IF::TextParams::Set(0xFFFFFF,
                                                    IF::TextAlignment::LEFT,
                                                    2.0),
                                "'2' - 'classic' Bresenham");

          IF::Instance().Printf(0,
                                _windowHeight - 20 * 3,
                                IF::TextParams::Set(0xFFFFFF,
                                                    IF::TextAlignment::LEFT,
                                                    2.0),
                                "'3' - (mx + c)");

          IF::Instance().Printf(0,
                                _windowHeight - 20 * 2,
                                IF::TextParams::Set(0xFFFFFF,
                                                    IF::TextAlignment::LEFT,
                                                    2.0),
                                "WASD and arrow keys - move points");

          IF::Instance().Printf(0,
                                _windowHeight - 20,
                                IF::TextParams::Set(0xFFFFFF,
                                                    IF::TextAlignment::LEFT,
                                                    2.0),
                                "'TAB' - hide text");
        }
      }
      else
      {
        IF::Instance().Printf(0,
                              _windowHeight - 20,
                              IF::TextParams::Set(0xFFFFFF,
                                                  IF::TextAlignment::LEFT,
                                                  2.0),
                              "Vertical mode");
      }
    }

    void HandleEvent(const SDL_Event& evt) override
    {
      switch (evt.type)
      {
        case SDL_KEYDOWN:
        {
          switch (evt.key.keysym.sym)
          {
            case SDLK_ESCAPE:
              Stop();
              break;

            case SDLK_RIGHT:
              X1++;
              break;

            case SDLK_LEFT:
              X1--;
              break;

            case SDLK_UP:
              Y1--;
              break;

            case SDLK_DOWN:
              Y1++;
              break;

            case SDLK_a:
              X2--;
              break;

            case SDLK_d:
              X2++;
              break;

            case SDLK_w:
              Y2--;
              break;

            case SDLK_s:
              Y2++;
              break;

            case SDLK_m:
              VerticalMode = not VerticalMode;
              break;

            case SDLK_TAB:
              ShowText = not ShowText;
              break;

            case SDLK_1:
              ShowHindu = not ShowHindu;
              break;

            case SDLK_2:
              ShowCorrect = not ShowCorrect;
              break;

            case SDLK_3:
              ShowDumb = not ShowDumb;
              break;

            default:
              break;
          }
        }
      }
    }
};

int main(int argc, char* argv[])
{
  BH bh;

  if (bh.Init(700, 700, QualityReductionFactor))
  {
    IF::Instance().Init(bh.GetRenderer());
    bh.Run(true);
  }

  return 0;
}

