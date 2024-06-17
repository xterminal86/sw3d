#include "sw3d.h"
#include "instant-font.h"

#include <map>

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

bool Paused = false;

RenderMode RenderMode_ = RenderMode::SOLID;

size_t RenderModeIndex = 0;
const std::map<RenderMode, std::string> RenderModes =
{
  { RenderMode::SOLID,     "SOLID"     },
  { RenderMode::WIREFRAME, "WIREFRAME" },
  { RenderMode::MIXED,     "MIXED"     }
};

enum class ProjectionMode
{
  ORTHOGRAPHIC,
  WEAK_PERSPECTIVE,
  PERSPECTIVE
};

ProjectionMode ProjectionMode_ = ProjectionMode::ORTHOGRAPHIC;

size_t ProjectionModeIndex = 0;
const std::map<ProjectionMode, std::string> ProjectionModes =
{
  { ProjectionMode::ORTHOGRAPHIC,     "ORTHO"        },
  { ProjectionMode::WEAK_PERSPECTIVE, "PERSP (weak)" },
  { ProjectionMode::PERSPECTIVE,      "PERSP (true)" }
};

enum class AppMode
{
  TEST = 0,
  FROM_OBJ
};

AppMode ApplicationMode = AppMode::TEST;

const uint32_t DebugColor = 0xAAAAAA;

const double InitialTranslation = 5.0;

class Drawer : public DrawWrapper
{
  public:
    void ApplyProjection()
    {
      switch (ProjectionMode_)
      {
        case ProjectionMode::ORTHOGRAPHIC:
          SetOrthographic(-InitialTranslation * 0.5, InitialTranslation * 0.5,
                          InitialTranslation * 0.5, -InitialTranslation * 0.5,
                          InitialTranslation * 0.5, -InitialTranslation * 0.5);
          break;

        case ProjectionMode::WEAK_PERSPECTIVE:
          SetWeakPerspective();
          break;

        case ProjectionMode::PERSPECTIVE:
          SetPerspective(60.0,
                         (double)WindowWidth / (double)WindowHeight,
                         0.1,
                         1000.0);
          break;
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
            {
              RenderModeIndex++;
              RenderModeIndex %= RenderModes.size();
              auto it = RenderModes.begin();
              std::advance(it, RenderModeIndex);
              RenderMode_ = it->first;
            }
            break;

            case SDLK_1:
              ApplicationMode = AppMode::TEST;
              break;

            case SDLK_2:
              ApplicationMode = AppMode::FROM_OBJ;
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
              Paused = not Paused;
              break;

            case SDLK_p:
              ProjectionModeIndex++;
              ProjectionModeIndex %= ProjectionModes.size();
              auto it = ProjectionModes.begin();
              std::advance(it, ProjectionModeIndex);
              ProjectionMode_ = it->first;
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
          tt.Points[i].Z += (InitialTranslation + DZ);
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
          //
          // Some vertices can share several faces and here we apply the same
          // operation to them several times. This can become quite inefficient.
          // Instead we need to specify all vertices, apply projection to them
          // once and then draw triangles based on those projected vertices
          // using faces enumeration.
          // We'll do that in .obj file loading, but I'll leave this here for
          // history and simplicity sake.
          //
          tp.Points[i] = _projectionMatrix * tt.Points[i];

          //
          // TODO: temporary hack to place (0;0) at the center of the screen.
          // In future this will happen only after transformation to NDC.
          // Right now this all "works" because our cube's coordinates span from
          // 0 to 1.
          //
          tp.Points[i].X += 1;
          tp.Points[i].Y += 1;

          tp.Points[i].X /= 2.0;
          tp.Points[i].Y /= 2.0;

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
                     RenderMode_);
      }

      if (not Paused)
      {
        angle += (RotationSpeed * DeltaTime());
      }
    }

    // -------------------------------------------------------------------------

    void DrawFromObj()
    {
      static double angle = 0.0;

      if (not Paused)
      {
        angle += (RotationSpeed * DeltaTime());
      }
    }

    // -------------------------------------------------------------------------

    void Draw3D()
    {
      switch (ApplicationMode)
      {
        case AppMode::TEST:
          DrawTestCube();
          break;

        case AppMode::FROM_OBJ:
          DrawFromObj();
          break;
      }
    }

    // -------------------------------------------------------------------------

    void DrawToScreen() override
    {
      PRINT(WindowWidth - 10, 10, "DX: %.2f", DX);
      PRINT(WindowWidth - 10, 20, "DY: %.2f", DY);
      PRINT(WindowWidth - 10, 30, "DZ: %.2f", DZ);

      PRINT(WindowWidth - 10, 40,
            "Projection: %s", ProjectionModes.at(ProjectionMode_).data());

      PRINT(WindowWidth - 10, 50,
            "Mode: %s", RenderModes.at(RenderMode_).data());

      PRINT(WindowWidth - 10, WindowHeight - 140, "Projection matrix:");

      for (uint8_t x = 0; x < 4; x++)
      {
        for (uint8_t y = 0; y < 4; y++)
        {
          PRINT(WindowWidth - 60 * (3 - y),
                WindowHeight - 30 * (4 - x),
                "%.2f  ",
                _projectionMatrix[x][y]);
        }
      }

      if (Paused)
      {
        IF::Instance().Print(0, 0, "PAUSED", 0xFFFF00);
      }
    }

    // -------------------------------------------------------------------------

    void DrawToFrameBuffer() override
    {
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
    IF::Instance().Init(d.GetRenderer());
    d.Run(true);
  }

  return 0;
}
