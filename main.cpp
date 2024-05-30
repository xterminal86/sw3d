#include "sw3d.h"

using namespace SW3D;

const uint32_t WW = 600;
const uint32_t WH = 600;
const uint32_t RESOLUTION = 100;

int x = 0;
int y = 0;

double DX = 0.0;
double DY = 0.0;
double DZ = 0.0;

bool wireframe = false;

const uint32_t DebugColor = 0xAAAAAA;

class Drawer : public DrawWrapper
{
  public:
    Drawer()
    {
      _windowName = "Software 3D renderer";

      _cube.Triangles =
      {
        // FRONT
        { 0.0, 0.0, 0.0,    1.0, 0.0, 0.0,    0.0, 1.0, 0.0 },
        { 1.0, 0.0, 0.0,    1.0, 1.0, 0.0,    0.0, 1.0, 0.0 },

        // BACK
        { 0.0, 0.0, 1.0,    0.0, 1.0, 1.0,    1.0, 0.0, 1.0 },
        { 1.0, 0.0, 1.0,    0.0, 1.0, 1.0,    1.0, 1.0, 1.0 },

        // LEFT
        { 0.0, 0.0, 0.0,    0.0, 1.0, 0.0,    0.0, 1.0, 1.0 },
        { 0.0, 0.0, 0.0,    0.0, 1.0, 1.0,    0.0, 0.0, 1.0 },

        // RIGHT
        { 1.0, 0.0, 0.0,    1.0, 0.0, 1.0,    1.0, 1.0, 1.0 },
        { 1.0, 0.0, 0.0,    1.0, 1.0, 1.0,    1.0, 1.0, 0.0 },

        // TOP
        { 0.0, 1.0, 0.0,    1.0, 1.0, 0.0,    0.0, 1.0, 1.0 },
        { 1.0, 1.0, 0.0,    1.0, 1.0, 1.0,    0.0, 1.0, 1.0 },

        // BOTTOM
        { 0.0, 0.0, 0.0,    0.0, 0.0, 1.0,    1.0, 0.0, 0.0 },
        { 1.0, 0.0, 0.0,    0.0, 0.0, 1.0,    1.0, 0.0, 1.0 },
      };

      for (auto& i : _cube.Triangles)
      {
        i.Points[0].Z += 20;
        i.Points[1].Z += 20;
        i.Points[2].Z += 20;
      }

      //
      // The objects exist in 3D space, but our screen is 2D space.
      // In order to draw 3D object onto the screen we need to find a way to
      // transform 3D coordinates to the 2D screen.
      // This is called a projection.
      //
      // To do this several steps need to be performed. First of which is
      // normalization of our screen space to be [-1; 1] along the x axis
      // and [-1; 1] along the y axis. This is called Normalized Device
      // Coordinates (or NDC for short).
      //
      // (NOTE: need details in the following section, research more)
      //
      // Since desktop screens usually have their width greater than height,
      // we'll define aspect ratio as w / h. It really is just a matter of
      // convention, we could've easily defined aspect ratio as h / w, just like
      // in OLC video, it would just resulted in multiplying aspect ratio by
      // coordinate in the matrix instead of dividing coordinate over it.
      //
      //      w
      // a = ---
      //      h
      //
      // First step is to divide x coordinate of a given 3D point over aspect
      // ratio to take into account different screen width.
      // Since we'll be "condensing" all points into NDC, we need to take into
      // account different screen resolutions. E.g. resolution 2000x1000 would
      // mean that we have to map all [ -1 ; 1 ] into 2000 pixels wide.
      // That would make the object stretch across X axis. So, to prevent that
      // we will divide its projected X coordinate over aspect ratio to bring it
      // back into [ -1 ; 1 ] of normalized space.
      // Conversely, if resolution would've been 1000x2000 the projected Y
      // coordinates would have to be squeezed into 2000 pixels of height.
      // Since aspect ratio would be less than 1, by dividing Y over it we will
      // bring it back into [ -1 ; 1 ] for Y.
      //
      //                x
      // [x, y, z] = [ ---, y, z];
      //                a
      //
      // Next, we need to take into account Field Of View (FOV), which is
      // defined by angle theta (TH).
      //
      //                1          1              1
      // [x, y, z] = [ ---  *  --------- * x, --------- * y, z];
      //                a      tan(TH/2)      tan(TH/2)
      //
      //
      // Next, position of object in depth:
      //
      //
      //      10
      // Z+ ^    ----------------- Zfar
      //    |    \               /    |
      //    |     \   x         /     |
      //    |      \           /      |
      //    |       \         /       |
      //    |        \_______/        |
      //      1             | Znear   |
      //                    |         |
      //                 *__|_________|
      //                 ^
      //               (eye)
      //
      // We need to normalize point's z coordinate. We do this by dividing over
      // (Zfar - Znear):
      //
      //       z
      // --------------
      // (Zfar - Znear)
      //
      // which will give us z in range from 0 to 1. But our Zfar in this example
      // is 10, so we need to scale back up again. We do this by multiplying
      // over Zfar:
      //
      //          Zfar
      // z * --------------
      //     (Zfar - Znear)
      //
      // But we also need to offset our scaled point back closer to the eye
      // by the distance amount from the eye to Znear:
      //
      // (Zfar * Znear)
      // --------------
      // (Zfar - Znear)
      //
      // This is basically equivalent to taking point Znear, normalizing it
      // like we just did above, and then just subtracting its value from our z
      // point's value:
      //
      //         Zfar         (Znear * Zfar)
      // z * -------------- - -------------- ];
      //     (Zfar - Znear)   (Zfar - Znear)
      //
      //
      // So, our total projection so far looks like this:
      //
      //                1          1
      // [x, y, z] = [ --- *  ---------- * x,
      //                a      tan(TH/2)
      //
      //                         1
      //                    ---------- * y,
      //                     tan(TH/2)
      //
      //                       Zfar         (Znear * Zfar)
      //               z * -------------- - -------------- ];
      //                   (Zfar - Znear)   (Zfar - Znear)
      //
      //
      // When things are further away, they appear less, so this implies a
      // change in x coordinate is somehow related to z depth, more specifically
      // it's inversely proportional:
      //
      //           1
      // x' = x * ---
      //           z
      //
      // The same goes for y:
      //
      //           1
      // y' = y * ---
      //           z
      //
      //
      // So our formula becomes:
      //
      //
      //                1        1        x
      // [x, y, z] = [ --- * --------- * ---,
      //                a    tan(TH/2)    z
      //
      //                         1        y
      //                    ---------- * ---,
      //                     tan(TH/2)    z
      //
      //                       Zfar         (Znear * Zfar)
      //               z * -------------- - -------------- ];
      //                   (Zfar - Znear)   (Zfar - Znear)
      //
      //
      // Let's simplify this a bit by making aliases:
      //
      //         1
      // F = ---------
      //     tan(TH/2)
      //
      //
      //         Zfar
      // q = --------------
      //     (Zfar - Znear)
      //
      //
      // With this we can rewrite the transformations above as:
      //
      //                Fx     Fy
      // [x, y, z] = [ ---- , ---- , q * (z - Znear) ]
      //                az     z
      //
      //
      // We can implement these equations directly, but in 3D graphics it's
      // common to use matrix multiplication, so we'll convert this to matrix
      // form.
      //
      //      -                 -
      //     |                   |
      //     | F/a 0  0          |
      //     |                   |
      //     | 0   F  0          |
      // M = |                   |
      //     | 0   0  q          |
      //     |                   |
      //     | 0   0  -Znear * q |
      //     |                   |
      //      -                 -
      //
      // Given like this, it is called the projection matrix. By multiplying
      // our 3D coordinates by this matrix we will transform them into
      // coordinates on the screen. But there is a problem.
      // We need to divide everything by Z in order to take into account depth
      // information, but there will be no Z saved in the resulting vector after
      // multiplication by this matrix. To solve this we need to add another
      // column to our matrix, thus making it 4 dimensional, as well as add
      // another coordinate to our original vector, which is conventionally
      // called W. We will put a 1 into cell [3][4] (one based index) of the
      // projection matrix which will allow us to put original Z value of a
      // vector into 4th element W of a resulting vector. Then we can divide
      // by it to correct for depth.
      // We can explicitly add another coordinate into vector class, or
      // calculate W implicitly during matrix-vector multiplication and
      // performing divide by W there (which exactly how it's done now).
      // But I believe this W coordinate also comes in handy in other situations
      // so usually people operate directly on Vec4.
      //
      //      -                    -
      //     |                      |
      //     | F/a 0  0           0 |
      //     |                      |
      //     | 0   F  0           0 |
      // M = |                      |
      //     | 0   0  q           1 |
      //     |                      |
      //     | 0   0  -Znear * q  0 |
      //     |                      |
      //      -                    -
      //
      // v4 = [ x, y, z, 0 ]
      //
      // projected = M * v;
      //
      //        F
      // [ x * ---   y * F   z * q - z * (Znear * q)   z ]
      //        a
      //
      //
      // Divide everything over 4th coordinate W (which is effectively Z):
      //
      //      xF
      // [ ( ---- ) / z   (y * F) / z   (z * q - z * (Znear * q)) / z   1 ]
      //      a
      //

      _projection = GetProjection(90.0,
                                 (double)WW / (double)WH,
                                 0.1,
                                 1000.0);
    }

    void HandleEvent(const SDL_Event& evt) override
    {
      const uint8_t* kb = SDL_GetKeyboardState(nullptr);
      if (kb[SDL_SCANCODE_ESCAPE])
      {
        Stop();
      }

      if (kb[SDL_SCANCODE_RIGHT])
      {
        x++;
      }

      if (kb[SDL_SCANCODE_LEFT])
      {
        x--;
      }

      if (kb[SDL_SCANCODE_UP])
      {
        y--;
      }

      if (kb[SDL_SCANCODE_DOWN])
      {
        y++;
      }

      switch (evt.type)
      {
        case SDL_KEYDOWN:
        {
          switch (evt.key.keysym.sym)
          {
            case SDLK_TAB:
              wireframe = !wireframe;
              break;

            case SDLK_p:
              SDL_Log("%d %d", x, y);
              break;

            case SDLK_e:
              DZ += 0.1;
              break;

            case SDLK_q:
              DZ -= 0.1;
              break;

            case SDLK_d:
              DX += 0.1;
              break;

            case SDLK_a:
              DX -= 0.1;
              break;

            case SDLK_w:
              DY -= 0.1;
              break;

            case SDLK_s:
              DY += 0.1;
              break;
          }
        }
        break;
      }
    }

    void DebugDraw()
    {
      DrawPoint({ 10, 10 }, 0xFFFFFF);
      DrawLine({ 10, 0 }, { 20, 5 }, 0xFF0000);

      DrawTriangle(SDL_Point{ 10 + x,  20 + y },
                   SDL_Point{  0, 40 },
                   SDL_Point{ 30, 40 },
                   DebugColor,
                   wireframe);

      DrawTriangle(SDL_Point{ 10 + x,  20 + y },
                   SDL_Point{ 30, 40 },
                   SDL_Point{ 30, 20 },
                   0x00FFFF,
                   wireframe);

      //
      // NOTE: old
      //
      // Flat bottom
      //

      //FillTriangle(10 + x,  0 + y,  0, 20, 30, 20, DebugColor);
      //FillTriangle(0 + x,  20 + y, 30, 20, 10,  0, DebugColor);
      //FillTriangle(30 + x, 20 + y, 10,  0,  0, 20, DebugColor);

      //
      // Flat top
      //

      //FillTriangle(20 + x, 20 + y, 10,  0,  0,  0, DebugColor);
      //FillTriangle(30 + x,  0 + y,  0,  0, 20, 10, DebugColor);
      //FillTriangle(0,   0, 20, 10, 30,  0, DebugColor);

      //
      // Composite
      //
      //FillTriangle(20,  0, 10, 10, 30, 20, DebugColor);
      //FillTriangle(10, 10, 30, 20, 20,  0, DebugColor);
      //FillTriangle(30, 20, 20,  0, 10, 10, DebugColor);
      //FillTriangle(10,  0,  0, 10, 20,  5, DebugColor);
      //
    }

    void Draw() override
    {
      // DEBUG:
      //{
      //  DebugDraw();
      //  return;
      //}

      Triangle t;

      t.Points[0] = {  0.0 + DX, 0.0 + DY, -1.0 + DZ };
      t.Points[1] = {  0.0 + DX, 1.0 + DY, -1.0 + DZ };
      t.Points[2] = {  1.0 + DX, 1.0 + DY, -1.0 + DZ };

      Triangle tp;

      for (size_t i = 0; i < 3; i++)
      {
        //
        // The resulting coordinates will be in range [ -1 ; 1 ].
        // So leftmost part will be offscreen.
        //
        tp.Points[i] = _projection * t.Points[i];

        //
        // To move it back into view, add 1 to make it in range [ 0 ; 2 ]
        // and thus visible.
        //
        tp.Points[i] += 1.0;

        //
        // Now we need to scale it properly into viewscreen.
        //
        tp.Points[i] *= (0.5 * ((double)WW / (double)FrameBufferSize()));
      }

      DrawTriangle(tp.Points[0],
                    tp.Points[1],
                    tp.Points[2],
                    0xFFFFFF,
                    wireframe);

      //for (auto& t : _cube.Triangles)
      //{
      //  Triangle triProj;
      //
      //  for (size_t i = 0; i < 3; i++)
      //  {
      //    triProj.Points[i] = _projection * t.Points[i];
      //  }
      //
      //  DrawTriangle(triProj.Points[0],
      //                triProj.Points[1],
      //                triProj.Points[2],
      //                0xFFFFFF,
      //                wireframe);
      //}
    }

  private:
    Mesh _cube;
    Matrix _projection;
};

int main(int argc, char* argv[])
{
  Drawer d;

  if ( d.Init(WW, WH, RESOLUTION) )
  {
    d.Run(true);
  }

  return 0;
}
