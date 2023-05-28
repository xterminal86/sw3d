#include "sw3d.h"

using namespace SW3D;

uint16_t x = 0;
uint16_t y = 0;

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
    }

    void Draw() override
    {
      // ***********************************************************************
      //
      // DEBUG:
      //
      // Flat bottom
      //

      //FillTriangle(10, 0, 0, 20, 30, 20, 0xFFFFFF);
      //FillTriangle(0, 20, 30, 20, 10, 0, 0xFFFFFF);
      //FillTriangle(30, 20, 10, 0, 0, 20, 0xFFFFFF);

      //
      // Flat top
      //

      //FillTriangle(20, 20, 10, 0, 0, 0, 0xFFFFFF);
      //FillTriangle(30, 0, 0, 0, 20, 10, 0xFFFFFF);
      //FillTriangle(0, 0, 20, 10, 30, 0, 0xFFFFFF);

      //
      // Composite
      //
      //FillTriangle(20, 0, 10, 10, 30, 20, 0xFFFFFF);
      //FillTriangle(10, 10, 30, 20, 20, 0, 0xFFFFFF);
      //FillTriangle(30, 20, 20, 0, 10, 10, 0xFFFFFF);
      //FillTriangle(10, 0, 0, 10, 20, 5, 0xFFFFFF);
      //

      // ***********************************************************************

      //
      // The objects exist in 3D space, but our screen is 2D space.
      // In order to draw 3D object onto the screen we need to find a way to
      // transform 3D coordinates to the 2D screen.
      // This is called a projection.
      //
      // To do this several steps need to be performed. First of which is
      // normalization of our screen space to be [-1; 1] along the x axis
      // and [-1; 1] along the y axis.
      //
      // (NOTE: need details in the following section, research more)
      //
      //      h
      // a = ---
      //      w
      //
      // First step is to multiply aspect ratio with x coordinate of a given 3D
      // point to take into account different screen width.
      //
      // [x, y, z] = [ a * x, a * y, z];
      //
      // Next, we need to take into account Field Of View (FOV), which is
      // defined by angle theta (TH).
      //
      //                        1               1
      // [x, y, z] = [ a *  ---------- * x, ---------- * y, z];
      //                     tan(TH/2)       tan(TH/2)
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
      //                        1
      // [x, y, z] = [ a *  ---------- * x,
      //                     tan(TH/2)
      //
      //                         1
      //                    ---------- * y,
      //                     tan(TH/2)
      //
      //                       Zfar         (Znear * Zfar)
      //               z * -------------- - -------------- ];
      //                   (Zfar - Znear)   (Zfar - Znear)
      //
    }

  private:
    SW3D::Mesh _cube;
};

int main(int argc, char* argv[])
{
  Drawer d;

  if ( d.Init(800, 600, 12) )
  {
    d.Run(true);
  }

  return 0;
}
