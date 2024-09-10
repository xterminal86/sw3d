#include "sw3d.h"
#include "instant-font.h"

const size_t QualityReductionFactor = 4;

int X1 = 10;
int Y1 = 5;
int X2 = 50;
int Y2 = 20;

using namespace SW3D;

class BH : public DrawWrapper
{
  public:
    void BresenhamHindu(int x1, int y1, int x2, int y2)
    {
      //
      // https://www.youtube.com/watch?v=RGB-wlatStc
      //
      // There are several implementations of Bresenham algorithm, the one that
      // people probably see for the first time is not very clear because it uses
      // some weird variable called error, so I found out better explanation that is
      // very straightforward (see link above) and doesn't use floating point values.
      // In the end it doesn't matter which implementation is used because they all
      // achieve the same end result and even the one that uses floating point
      // calculation has no impact on performance with modern hardware.
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
      // Let's recall SOHCAHTOA, where Tangent = Opposite / Adjacent.
      // So it's easy to see that in line equation k = dy / dx = tan(a).
      // It's obvious that if k = 1 then a = 45 deg. If k < 1 then angle is < 45,
      // and if k > 1 then it's > 45.
      //
      // Basic idea of Bresenham algorithm is to determine which pixel to fill in
      // case when our true line intersects both of them. Like this (zoomed in and
      // not to scale):
      //
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
      // Here true line crosses pixels 2 and 5 and we need to choose which one to
      // fill. The main idea of algorithm is to fill the one whose distance to the
      // "temporary pixel" (let's called it T) is smaller. So let's imagine that
      // there is another pixel in between pixels 2 and 5 and line crosses it. Let's
      // zoom in this part (again, not to scale because ASCII art):
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
      // Now we just need to find distance between 2 to T (d2) and 5 to T (d1) and
      // choose the one that is smaller. So if d1 - d2 < 0 then we should turn pixel
      // 5 on, otherwise if d1 - d2 >= 0 it's pixel 2. One needs not to be deceived
      // by everything looking the same size: line can cross pixels with slope much
      // closer to a line parallel to X axis, so, for example, our T could be closer
      // to pixel 5 than it is portrayed on the picture.
      //
      // For simplicity sake let's consider that our line has k < 1 for now.
      // For k >= 1 it's just a matter of swapping some variables (more on that
      // later).
      //
      // To draw a line we will iterate from x1 to x2. Since k < 1 we have more
      // points across X axis, so our x will always increase when we traverse from x1
      // to x2, but whether y should increase or not given any x sample - that's what
      // we'll have to decide.
      //
      // So it's time for some math:
      //
      // Xnext = Xk + 1
      //
      // We're interested in what Y we should choose for point Xk + 1, whether it
      // will remain Yk or it will be Yk + 1.
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
      // If (d1 - d2) <  0 then our value Y for this X sample should remain as Yk.
      // If (d1 - d2) >= 0 then our value Y for this X sample should be Yk + 1.
      //
      // (d1 - d2) = [ k(Xk + 1) + C - Yk ] - [ Yk + 1 - k(Xk + 1) - C ]
      //
      // Open up brackets:
      //
      // (d1 - d2) = k(Xk + 1) + C - Yk - Yk - 1 + k(Xk + 1) + C
      //             =========   ~ ---- ----       =========   ~
      //
      // (d1 - d2) = 2k(Xk + 1) - 2Yk + 2C - 1
      //
      // We still have k in our equation which is dy / dx. To get rid of floating
      // point calculation we just multiply the whole thing by dx:
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
      // We can observe that highlighted variables are constant. For some reason that
      // is not very clear to me we can discard them as irrelevant, because we're
      // only interested in terms that depend on Xk and Yk. So we're left with:
      //
      // Pk = 2dyXk - 2dxYk
      //
      // Now let's write Pk for general case:
      //
      // Pnext = 2dyXnext - 2dxYnext
      //
      // Xnext is always Xk + 1 since we're dealing with a line with k < 1.
      // But value of Ynext we'll have to decide.
      // By subtracting Pnext - Pk we can find out how much a decision variable
      // should change at every iteration.
      //
      // (Pnext - Pk) = [ 2dyXnext - 2dxYnext ] - [ 2dyXk - 2dxYk ]
      // (Pnext - Pk) = 2dyXnext - 2dxYnext - 2dyXk + 2dxYk
      //
      // Factor out stuff:
      //
      // (Pnext - Pk) = 2dy * (Xnext - Xk) - 2dx * (Ynext - Yk)
      //
      // If (Pnext - Pk) < 0 then we should remain on the same Yk, so Ynext = Yk:
      //
      // Pnext = Pk + 2dy(Xk + 1 - Xk) - 2dx * (Yk - Yk)
      //
      // Cancel out Xk and Yk:
      //
      // +------------------+
      // | Pnext = Pk + 2dy |
      // +------------------+
      //
      // If (Pnext - Pk) >= 0 then Ynext = Yk + 1
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
      // Let's plug it in the formula above:
      //                                     dy
      // Pk = 2dyXk + 2dy - 2dxYk + 2dx[ y - -- x ] - dx
      //                                     dx
      //
      // For starting point let's call x X1 and y -> Y1 and also open up the
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
      // Before we start let's recall that explanation above was done for a line with
      // k < 1 and I mentioned that "more on that later". Well, *now* is "later".
      //
      // First let's consider the following two equations:
      //
      // 1) y = 10x    2) y = 0.1x
      //
      // It's obvious to see that these lines are mirrored across axis y = x and in
      // order to create mirrored line we just need to invert k, i.e. k = 1/k or
      // 1 / (dy / dx) = dx / dy
      //
      // So, whenever we have line with k > 1 we can convert it to the mirrored
      // version which will have k < 1 by swapping y from 1) as x for 2):
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
      // Now notice that when k < 1 it means that we have more X values than Y and Y
      // values are the ones that we need to determine whether to increment or not
      // during rasterization loop. So by checking the k value beforehand we can
      // create two execution branches that will increment either X constantly and
      // determine Y or vice versa. How exactly? If we have a line with k > 1 we
      // mirror it and then use the same calculations as usual but when we actually
      // draw stuff we will iterate over Y axis instead of X and increment X (or not)
      // instead of Y, and since everything is a mirror image it will all work out
      // nicely.
      //
      // Edge case when dx = 0 (line is vertical) can be handled separately but it's
      // not necessary here because it falls into k > 1 case and we're not using
      // division.
      //
      // This leaves the last question: direction of a line.
      // If we sort points by X we can then have only two cases:
      //
      // 1) line goes up
      // 2) line goes down
      //
      // So by finding out which type of line is our case we can determine whether
      // we need to increment Y or decrement it.
      //
      // ============================================================================

      //
      // Sort by X so that starting point is always the left one.
      //
      int xStart = (x1 > x2) ? x2 : x1;

      //
      // Make sure that starting Y is correct as well, depending on the xStart we
      // chose.
      //
      int yStart = (xStart == x1) ? y1 : y2;

      //
      // Same principle for end points because our rasterization loop is tied to
      // condition of "while less than end point".
      //
      int xEnd = (xStart == x1) ? x2 : x1;
      int yEnd = (xStart == x1) ? y2 : y1;

      bool goesDown = (yEnd > yStart);

      //
      // These will be our pixel coordinates to fill. Prime them with starting point.
      //
      int x = xStart;
      int y = yStart;

      int dx = std::abs(xEnd - xStart);
      int dy = std::abs(yEnd - yStart);

      //
      // Let's try to avoid any floating point calculation: if dy is greater than dx
      // this means that k > 1, so our line slope is "steep", that is its angle is
      // greater than 45 degrees. Otherwise we'll consider line slope to be "gentle".
      //
      bool steep = (dy > dx);

      //
      // If line is steep convert it to gentle by inverting k.
      //
      if (steep)
      {
        std::swap(dx, dy);
      }

      int P = 2 * dx - dy;

      SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);

      //
      // Gentle slope, everything goes as planned.
      //
      if (not steep)
      {
        while (x <= xEnd)
        {
          SDL_RenderDrawPoint(_renderer, x, y);

          x++;

          //
          // Our decision parameter.
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
        // If line is steep just swap stuff around. This time we're going across Y
        // axis and must determine whether to increase X or not. Notice that decision
        // parameter calculation remained the same but it actually is working on
        // mirrored line but doesn't know about it. :-)
        //
        while (y <= yEnd)
        {
          SDL_RenderDrawPoint(_renderer, x, y);

          //
          // This time Y is always incremented (or decremented, depending on
          // direction).
          //
          y = goesDown ? (y + 1) : (y - 1);

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

      SDL_SetRenderDrawColor(_renderer, 0, 255, 255, 255);
      SDL_RenderDrawPoint(_renderer, xStart, yStart);

      SDL_SetRenderDrawColor(_renderer, 255, 0, 255, 255);
      SDL_RenderDrawPoint(_renderer, xEnd, yEnd);
    }

    // -------------------------------------------------------------------------

    void DrawToFrameBuffer() override
    {
      SaveColor();

      BresenhamHindu(X1, Y1, X2, Y2);

      RestoreColor();
    }

    // -------------------------------------------------------------------------

    void DrawToScreen() override
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

