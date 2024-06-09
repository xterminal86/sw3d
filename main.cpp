#include "sw3d.h"

using namespace SW3D;

const uint16_t WW = 600;
const uint16_t WH = 600;

const uint16_t QualityReductionFactor = 2;

int KeyboardX = 0;
int KeyboardY = 0;

double DX = 0.0;
double DY = 0.0;
double DZ = 0.0;

const double RotationSpeed = 100.0;

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

      SetPerspective(60.0,
                     (double)WW / (double)WH,
                     0.1,
                     1000.0);

      /*
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
              DZ += 0.1;
              SDL_Log("DZ: %.2f", DZ);
              break;

            case SDLK_q:
              DZ -= 0.1;
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

    void DrawTestCube()
    {
      static double angle = 0.0;

      for (Triangle& t : _cube.Triangles)
      {
        // +-----------+
        // |MODEL-WORLD|
        // +-----------+
        //
        // Rotate.
        //
        Triangle tr = t;

        for (size_t i = 0; i < 3; i++)
        {
          // FIXME: doesn't work :-(
          //tr.Points[i] = Rotate(t.Points[i], Directions::UP, angle);

          tr.Points[i] = RotateZ(tr.Points[i], 0.5   * angle);
          tr.Points[i] = RotateY(tr.Points[i], 0.25  * angle);
          tr.Points[i] = RotateX(tr.Points[i], 0.125 * angle);
        }

        //
        // Translate.
        //
        Triangle tt = tr;

        for (size_t i = 0; i < 3; i++)
        {
          tt.Points[i].X += DX;
          tt.Points[i].Y += DY;
          tt.Points[i].Z += (5.0 + DZ);
        }

        // +----------+
        // |PROJECTION|
        // +----------+
        //
        // Project.
        //
        Triangle tp;

        for (size_t i = 0; i < 3; i++)
        {
          tp.Points[i] = _projectionMatrix * tt.Points[i];

          //
          // Scale into view.
          //
          tp.Points[i].X *= (double)FrameBufferSize();
          tp.Points[i].Y *= (double)FrameBufferSize();
        }

        DrawTriangle(tp.Points[0],
                     tp.Points[1],
                     tp.Points[2],
                     0xFFFFFF,
                     wireframe);
      }

      angle += (RotationSpeed * DeltaTime());
    }

    // -------------------------------------------------------------------------

    void Draw3D()
    {
      DrawTestCube();
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

  if ( d.Init(WW, WH, QualityReductionFactor) )
  {
    d.Run(true);
  }

  return 0;
}
