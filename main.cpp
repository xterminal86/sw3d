#include "sw3d.h"
#include "instant-font.h"

#define PRINT(x, y, format, ...) \
  IF::Instance().Printf(x, y, \
                        IF::TextParams::Set(0xFFFFFF, \
                                            IF::TextAlignment::RIGHT, \
                                            1.0), \
                        format, ##__VA_ARGS__);

using namespace SW3D;

const uint16_t WW = 600;
const uint16_t WH = 600;

const uint16_t QualityReductionFactor = 2;

uint16_t WindowWidth  = WW;
uint16_t WindowHeight = WH;

double DX = 0.0;
double DY = 0.0;
double DZ = 0.0;

const double RotationSpeed = 100.0;

bool Wireframe     = false;
bool IsPerspective = true;

const uint32_t DebugColor = 0xAAAAAA;

class Drawer : public DrawWrapper
{
  public:
    void ApplyProjection()
    {
      if (IsPerspective)
      {
        SetPerspective(60.0,
                       (double)WindowWidth / (double)WindowHeight,
                       0.1,
                       1000.0);
      }
      else
      {
        SetOrthographic(-1.0, 1.0, 1.0, -1.0, 1.0, -1.0);
      }
    }

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

      ApplyProjection();
    }

    // -------------------------------------------------------------------------

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

            case SDLK_TAB:
              Wireframe = !Wireframe;
              break;

            case SDLK_e:
              DZ += 0.1;
              break;

            case SDLK_q:
              DZ -= 0.1;
              break;

            case SDLK_d:
              DX += 1;
              break;

            case SDLK_a:
              DX -= 1;
              break;

            case SDLK_w:
              DY -= 1;
              break;

            case SDLK_s:
              DY += 1;
              break;

            case SDLK_SPACE:
              IsPerspective = not IsPerspective;
              ApplyProjection();
              break;
          }
        }
        break;

        case SDL_WINDOWEVENT:
        {
          if (evt.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
          {
            WindowWidth  = evt.window.data1;
            WindowHeight = evt.window.data2;

            ApplyProjection();
          }
        }
        break;
      }
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
                     Wireframe);
      }

      angle += (RotationSpeed * DeltaTime());
    }

    // -------------------------------------------------------------------------

    void Draw3D()
    {
      DrawTestCube();
    }

    // -------------------------------------------------------------------------

    void DrawGUI()
    {
      const uint32_t& fb = FrameBufferSize();

      PRINT(fb, 0,  "DX: %.2f", DX);
      PRINT(fb, 10, "DY: %.2f", DY);
      PRINT(fb, 20, "DZ: %.2f", DZ);
      PRINT(fb, 30, "Mode: %s", IsPerspective ? "P" : "O");
      PRINT(fb, 40, "'Tab' to change modes");
    }

    // -------------------------------------------------------------------------

    void Draw() override
    {
      Draw3D();
      DrawGUI();
    }

  private:
    Mesh _cube;
};

int main(int argc, char* argv[])
{
  Drawer d;

  if ( d.Init(WW, WH, QualityReductionFactor) )
  {
    IF::Instance().Init(d.GetRenderer());
    d.Run(true);
  }

  return 0;
}
