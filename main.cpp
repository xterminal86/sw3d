#include "sw3d.h"

using namespace SW3D;

const uint32_t WW = 600;
const uint32_t WH = 600;

const uint32_t RESOLUTION = 200;

int KeyboardX = 0;
int KeyboardY = 0;

double DX = 0.0;
double DY = 0.0;
double DZ = 0.0;

bool wireframe = false;

const uint32_t DebugColor = 0xAAAAAA;

class Drawer : public DrawWrapper
{
  public:
    void PostInit() override
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

      /*
      for (auto& v : _cube.Triangles)
      {
        v.Points[0].Z += 0.0;
        v.Points[1].Z += 0.0;
        v.Points[2].Z += 0.0;
      }
      */

      SetPerspective(90.0,
                     (double)WW / (double)WH,
                     0.1,
                     1000.0);

      /*
      //
      // Works for orthographic.
      //
      for (auto& v : _cube.Triangles)
      {
        v.Points[0] *= 100.0;
        v.Points[1] *= 100.0;
        v.Points[2] *= 100.0;
      }

      SetOrthographic(-1.0, 1.0, 1.0, -1.0, 1.0, -1.0);
      */
    }

    // -------------------------------------------------------------------------

    void HandleEvent(const SDL_Event& evt) override
    {
      const uint8_t* kb = SDL_GetKeyboardState(nullptr);
      if (kb[SDL_SCANCODE_ESCAPE])
      {
        Stop();
      }

      if (kb[SDL_SCANCODE_RIGHT])
      {
        KeyboardX++;
      }

      if (kb[SDL_SCANCODE_LEFT])
      {
        KeyboardX--;
      }

      if (kb[SDL_SCANCODE_UP])
      {
        KeyboardY--;
      }

      if (kb[SDL_SCANCODE_DOWN])
      {
        KeyboardY++;
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
              SDL_Log("%d %d", KeyboardX, KeyboardY);
              break;

            case SDLK_e:
              DZ += 0.01;
              SDL_Log("DZ: %.2f", DZ);
              break;

            case SDLK_q:
              DZ -= 0.01;
              SDL_Log("DZ: %.2f", DZ);
              break;

            case SDLK_d:
              DX += 1;
              SDL_Log("DX: %.2f", DX);
              break;

            case SDLK_a:
              DX -= 1;
              SDL_Log("DX: %.2f", DX);
              break;

            case SDLK_w:
              DY -= 1;
              SDL_Log("DY: %.2f", DY);
              break;

            case SDLK_s:
              DY += 1;
              SDL_Log("DY: %.2f", DY);
              break;
          }
        }
        break;
      }
    }

    // -------------------------------------------------------------------------

    void DebugDraw()
    {
      DrawPoint({ 10, 10 }, 0xFFFFFF);
      DrawLine({ 10, 0 }, { 20, 5 }, 0xFF0000);

      DrawTriangle(SDL_Point{ 10 + KeyboardX,  20 + KeyboardY },
                   SDL_Point{  0, 40 },
                   SDL_Point{ 30, 40 },
                   DebugColor,
                   wireframe);

      DrawTriangle(SDL_Point{ 10 + KeyboardX,  20 + KeyboardY },
                   SDL_Point{ 30, 40 },
                   SDL_Point{ 30, 20 },
                   0x00FFFF,
                   wireframe);
    }


    // -------------------------------------------------------------------------

    void Draw2D()
    {
      DrawLine({ 80, 20 }, { 85, 40 }, 0x00FF00);
    }

    // -------------------------------------------------------------------------

    void DrawTestTriangle()
    {
      static double angle = 0.0;

      //
      // Original model.
      //
      Triangle t;

      t.Points[0] = {  0.0,                   2.0*SW3D::SQRT3OVER4, 3.0 };
      t.Points[1] = { -2.0*SW3D::SQRT3OVER4, -2.0*SW3D::SQRT3OVER4, 3.0 };
      t.Points[2] = {  2.0*SW3D::SQRT3OVER4, -2.0*SW3D::SQRT3OVER4, 3.0 };

      //
      // Rotated.
      //
      //Triangle tr;

      for (size_t i = 0; i < 3; i++)
      {
        // FIXME: doesn' work.
        //tr.Points[i] = Rotate(t.Points[i], Directions::RIGHT, angle);

        // NOTE: works for orthographic.
        //tr.Points[i] = RotateX(t.Points[i], 0.5   * angle);
        //tr.Points[i] = RotateY(tr.Points[i], 0.25  * angle);
        //tr.Points[i] = RotateZ(tr.Points[i], 0.125 * angle);
      }

      //
      // Translated.
      //
      //Triangle tt = tr;
      Triangle tt = t;

      for (size_t i = 0; i < 3; i++)
      {
        tt.Points[i].X += DX;
        tt.Points[i].Y += DY;
        tt.Points[i].Z += DZ;
      }

      //
      // Projected.
      //
      Triangle tp;

      for (size_t i = 0; i < 3; i++)
      {
        //
        // The resulting coordinates will be in range [ -1 ; 1 ].
        // So leftmost part will be offscreen.
        //
        tp.Points[i] = _projectionMatrix * tt.Points[i];

        //
        // To move it back into view, add 1 to make it in range [ 0 ; 2 ]
        // and thus visible.
        //
        tp.Points[i].X += 1.0;
        tp.Points[i].Y += 1.0;

        //
        // Now we need to scale it properly into viewscreen.
        //
        tp.Points[i].X *= 0.5 * ( (double)WW / (double)FrameBufferSize() );
        tp.Points[i].Y *= 0.5 * ( (double)WW / (double)FrameBufferSize() );
      }

      DrawTriangle(tp.Points[0],
                   tp.Points[1],
                   tp.Points[2],
                   0xFFFFFF,
                   wireframe);

      angle += (100.0 * DeltaTime());
    }

    // -------------------------------------------------------------------------

    void DrawTestCube()
    {
      static double angle = 0.0;

      for (auto& t : _cube.Triangles)
      {
        Triangle tr;

        for (size_t i = 0; i < 3; i++)
        {
          //tr.Points[i] = Rotate(t.Points[i], Directions::RIGHT, angle);

          tr.Points[i] = RotateZ(t.Points[i],  0.5   * angle);
          tr.Points[i] = RotateY(tr.Points[i], 0.25  * angle);
          tr.Points[i] = RotateX(tr.Points[i], 0.125 * angle);
        }

        Triangle tt = tr;
        //Triangle tt = t;

        for (size_t i = 0; i < 3; i++)
        {
          tt.Points[i].X += DX;
          tt.Points[i].Y += DY;
          tt.Points[i].Z += DZ;
        }

        Triangle tp;

        for (size_t i = 0; i < 3; i++)
        {
          tp.Points[i] = _projectionMatrix * tt.Points[i];
          tp.Points[i] += 1.0;
          tp.Points[i] *= (0.5 * ((double)WW / (double)FrameBufferSize()));
        }

        DrawTriangle(tp.Points[0],
                     tp.Points[1],
                     tp.Points[2],
                     0xFFFFFF,
                     wireframe);
      }

      angle += (100.0 * DeltaTime());
    }

    // -------------------------------------------------------------------------

    void Draw3D()
    {
      DrawTestTriangle();
      //DrawTestCube();
    }

    // -------------------------------------------------------------------------

    void Draw() override
    {
      // DEBUG:
      //{
      //  DebugDraw();
      //  return;
      //}

      //Draw2D();
      Draw3D();
    }

  private:
    Mesh _cube;
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
