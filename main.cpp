//
// Trying to write some sort of software 3D renderer.
//
// Learning materials:
//
// -----------------------------------------------------------------------------
//
// https://www.youtube.com/@javidx9
//
// 3D Graphics Engine (multipart series, not very detailed, but still helpful):
//
// Part 1: https://www.youtube.com/watch?v=ih20l3pJoeU
// Part 2: https://www.youtube.com/watch?v=XgMWc6LumG4
// Part 3: https://www.youtube.com/watch?v=HXSuNxpCzdM
//
// -----------------------------------------------------------------------------
//
// https://www.youtube.com/@ChiliTomatoNoodle
//
// 3D Fundamentals playlist (quite long, lots of details and stuff):
//
// https://www.youtube.com/playlist?list=PLqCJpWy5Fohe8ucwhksiv9hTF5sfid8lA
//
// -----------------------------------------------------------------------------
//
#include "sw3d.h"
#include "instant-font.h"

#include <map>

#define PRINTL(x, y, format, ...) \
  IF::Instance().Printf(x, y, \
                        IF::TextParams::Set(0xFFFFFF, \
                                            IF::TextAlignment::LEFT, \
                                            1.0), \
                        format, ##__VA_ARGS__);


#define PRINTR(x, y, format, ...) \
  IF::Instance().Printf(x, y, \
                        IF::TextParams::Set(0xFFFFFF, \
                                            IF::TextAlignment::RIGHT, \
                                            1.0), \
                        format, ##__VA_ARGS__);

using namespace SW3D;

const uint16_t WW = 640;
const uint16_t WH = 480;

const uint16_t QualityReductionFactor = 2;

uint16_t WindowWidth  = WW;
uint16_t WindowHeight = WH;

double DX = 0.0;
double DY = 0.0;
double DZ = 0.0;

const double RotationSpeed = 100.0;

bool Paused = false;

CullFaceMode CullFaceMode_ = CullFaceMode::BACK;

size_t CullFaceModeIndex = 1;
const std::map<CullFaceMode, std::string> CullFaceModes =
{
  { CullFaceMode::FRONT, "Cull FRONT" },
  { CullFaceMode::BACK,  "Cull BACK"  },
  { CullFaceMode::NONE,  "Cull NONE"  },
};

RenderMode RenderMode_ = RenderMode::SOLID;

size_t RenderModeIndex = 0;
const std::map<RenderMode, std::string> RenderModes =
{
  { RenderMode::SOLID,     "SOLID"     },
  { RenderMode::WIREFRAME, "WIREFRAME" },
  { RenderMode::MIXED,     "MIXED"     }
};

ProjectionMode ProjectionMode_ = ProjectionMode::PERSPECTIVE;

size_t ProjectionModeIndex = 2;
const std::map<ProjectionMode, std::string> ProjectionModes =
{
  { ProjectionMode::ORTHOGRAPHIC,     "ORTHO"        },
  { ProjectionMode::WEAK_PERSPECTIVE, "PERSP (weak)" },
  { ProjectionMode::PERSPECTIVE,      "PERSP (true)" }
};

enum class AppMode
{
  TEST = 0,
  FROM_OBJ,
  SHOW_AXES,
  PIPELINE,
  TWO_PROJECTIONS
};

const std::unordered_map<AppMode, std::string> AppModes =
{
  { AppMode::TEST,            "Test cube, manual rendering"        },
  { AppMode::FROM_OBJ,        "Loaded from .obj, manual rendering" },
  { AppMode::SHOW_AXES,       "Default axes"                       },
  { AppMode::PIPELINE,        "Rendering pipeline"                 },
  { AppMode::TWO_PROJECTIONS, "Two projections"                    }
};

AppMode ApplicationMode = AppMode::TEST;

const uint32_t DebugColor = 0xAAAAAA;

const double InitialTranslation = 5.0;

SW3D::ModelLoader Loader;

int ModelIndex = 0;

const std::vector<std::string> ModelsList =
{
  "models/cube.obj",
  "models/monkey.obj",
  "models/two.obj",
};

const std::string kAxesFname = "models/axes.obj";
SW3D::ModelLoader Axes;

const std::string kCubeFname = "models/cube.obj";
SW3D::ModelLoader Cube;

// =============================================================================

class Drawer : public DrawWrapper
{
  public:
    void ApplyProjection()
    {
      switch (ProjectionMode_)
      {
        case ProjectionMode::ORTHOGRAPHIC:
          SetOrthographic(-InitialTranslation * 0.5,  InitialTranslation * 0.5,
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

      bool ok = Loader.Load(ModelsList[ModelIndex]);
      if (not ok)
      {
        SDL_Log("%s", SW3D::ErrorToString());
      }

      ok = Axes.Load(kAxesFname);
      if (not ok)
      {
        SDL_Log("%s", SW3D::ErrorToString());
      }

      ok = Cube.Load(kCubeFname);
      if (not ok)
      {
        SDL_Log("%s", SW3D::ErrorToString());
      }

      ApplyProjection();

      SetMatrixMode(MatrixMode::MODELVIEW);
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
              SetRenderMode(RenderMode_);
            }
            break;

            case SDLK_1:
              ApplicationMode = AppMode::TEST;
              break;

            case SDLK_2:
              ApplicationMode = AppMode::FROM_OBJ;
              break;

            case SDLK_3:
              ApplicationMode = AppMode::SHOW_AXES;
              break;

            case SDLK_4:
              ApplicationMode = AppMode::PIPELINE;
              break;

            case SDLK_5:
            {
              //
              // Since we have several display "modes" but one rendering
              // pipeline we could break some internal states in between, e.g.
              // by changing projection in one mode and then switching to
              // another mode. So let's explicitly set some when needed.
              //
              ApplicationMode = AppMode::TWO_PROJECTIONS;
              SetWeakPerspective();
              ProjectionMode_ = ProjectionMode::WEAK_PERSPECTIVE;
            }
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

            case SDLK_c:
            {
              CullFaceModeIndex++;
              CullFaceModeIndex %= CullFaceModes.size();
              auto it = CullFaceModes.begin();
              std::advance(it, CullFaceModeIndex);
              CullFaceMode_ = it->first;
              SetCullFaceMode(CullFaceMode_);
            }
            break;

            case SDLK_p:
            {
              if (ApplicationMode != AppMode::TWO_PROJECTIONS)
              {
                ProjectionModeIndex++;
                ProjectionModeIndex %= ProjectionModes.size();
                auto it = ProjectionModes.begin();
                std::advance(it, ProjectionModeIndex);
                ProjectionMode_ = it->first;
                ApplyProjection();
              }
            }
            break;

            if (ApplicationMode == AppMode::FROM_OBJ)
            {
              case SDLK_LEFTBRACKET:
              {
                ModelIndex--;
                if (ModelIndex < 0)
                {
                  ModelIndex = ModelsList.size() - 1;
                }

                bool ok = Loader.Load(ModelsList[ModelIndex]);
                if (not ok)
                {
                  SDL_Log("%s", SW3D::ErrorToString());
                }
              }
              break;

              case SDLK_RIGHTBRACKET:
              {
                ModelIndex++;
                ModelIndex %= ModelsList.size();
                bool ok = Loader.Load(ModelsList[ModelIndex]);
                if (not ok)
                {
                  SDL_Log("%s", SW3D::ErrorToString());
                }
              }
              break;
            }
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

      for (TriangleSimple& t : _cube.Triangles)
      {
        // +-----------+
        // |MODEL-WORLD|
        // +-----------+
        //
        // Rotate.
        //
        TriangleSimple tr = t;

        for (size_t i = 0; i < 3; i++)
        {
          // FIXME: doesn't work :-(
          //tr.Points[i] = Rotate(t.Points[i], Directions::UP, angle);

          tr.Points[i] = SW3D::RotateZ(tr.Points[i], 0.5   * angle);
          tr.Points[i] = SW3D::RotateY(tr.Points[i], 0.25  * angle);
          tr.Points[i] = SW3D::RotateX(tr.Points[i], 0.125 * angle);
        }

        //
        // Translate.
        //
        TriangleSimple tt = tr;

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
        TriangleSimple tp;

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

      static Triangle tr;

      //
      // Old school style.
      //
      PushMatrix();

      RotateZ(0.5   * angle);
      RotateY(0.25  * angle);
      RotateX(0.125 * angle);

      Translate(DX, DY, (InitialTranslation + DZ));

      for (auto& obj : Loader.GetScene().Objects)
      {
        for (auto& face : obj.Faces)
        {
          for (size_t i = 0; i < 3; i++)
          {
            int32_t vertexInd = face.Indices[i][0];
            if (vertexInd == -1)
            {
              SDL_Log("vertex index is -1, expect crash");
            }

            const Vec3& v = Loader.GetScene().Vertices[vertexInd];

            tr.Points[i].Position = (_modelViewMatrix * v);
          }

          if (CullFaceMode_ != CullFaceMode::NONE)
          {
            ShouldCullFace(Vec3::Zero(), tr);
          }
          else
          {
            tr.CullFlag = false;
          }

          for (size_t i = 0; i < 3; i++)
          {
            tr.Points[i].Position = (_projectionMatrix * tr.Points[i].Position);

            tr.Points[i].Position.X += 1;
            tr.Points[i].Position.Y += 1;

            tr.Points[i].Position.X /= 2.0;
            tr.Points[i].Position.Y /= 2.0;

            //
            // Scale into view.
            //
            tr.Points[i].Position.X *= (double)FrameBufferSize();
            tr.Points[i].Position.Y *= (double)FrameBufferSize();
          }

          if (not tr.CullFlag)
          {
            DrawTriangle(tr.Points[0].Position,
                         tr.Points[1].Position,
                         tr.Points[2].Position,
                         0xFFFFFF,
                         RenderMode_);
          }
        }
      }

      PopMatrix();

      if (not Paused)
      {
        angle += (RotationSpeed * DeltaTime());
      }
    }

    // -------------------------------------------------------------------------

    void DrawAxes()
    {
      static Triangle tr;

      PushMatrix();

      Translate(DX, DY, (InitialTranslation + DZ));

      for (auto& obj : Axes.GetScene().Objects)
      {
        for (auto& face : obj.Faces)
        {
          for (size_t i = 0; i < 3; i++)
          {
            int32_t vertexInd = face.Indices[i][0];
            Vec3 v = Axes.GetScene().Vertices[vertexInd];

            //
            // NOTE: without parentheses it's actually an order of magnitude
            // slower.
            //
            tr.Points[i].Position = _projectionMatrix * (_modelViewMatrix * v);

            tr.Points[i].Position.X += 1;
            tr.Points[i].Position.Y += 1;

            tr.Points[i].Position.X /= 2.0;
            tr.Points[i].Position.Y /= 2.0;

            tr.Points[i].Position.X *= (double)FrameBufferSize();
            tr.Points[i].Position.Y *= (double)FrameBufferSize();
          }

          DrawTriangle(tr.Points[0].Position,
                       tr.Points[1].Position,
                       tr.Points[2].Position,
                       0xFFFFFF,
                       RenderMode_);
        }
      }

      PopMatrix();
    }

    // -------------------------------------------------------------------------

    void RenderingPipeline()
    {
      static double angle = 0.0;

      static Triangle tr;

      PushMatrix();

      //
      // Translation / rotation
      //

      RotateZ(0.5   * angle);
      RotateY(0.25  * angle);
      RotateX(0.125 * angle);

      Translate(DX, DY, (InitialTranslation + DZ));

      for (auto& obj : Loader.GetScene().Objects)
      {
        for (auto& tri : obj.Triangles)
        {
          //
          // Add to rendering queue with current modelview and projection
          // matrices.
          //
          Enqueue(tri);
        }
      }

      PopMatrix();

      PushMatrix();

      Translate(2.5, -4.0, InitialTranslation * 2);

      SetCullFaceMode(CullFaceMode::NONE);
      SetRenderMode(RenderMode::WIREFRAME);
      SetShadingMode(ShadingMode::NONE);

      for (auto& obj : Axes.GetScene().Objects)
      {
        for (auto& tri : obj.Triangles)
        {
          Enqueue(tri);
        }
      }

      SetShadingMode(ShadingMode::FLAT);
      SetCullFaceMode(CullFaceMode_);
      SetRenderMode(RenderMode_);

      PopMatrix();

      // *****************************
      //
      // Draw everything in the queue.
      //
      // *****************************
      CommenceDraw();

      if (not Paused)
      {
        angle += (RotationSpeed * DeltaTime());
      }
    }

    // -------------------------------------------------------------------------

    void TwoProjections()
    {
      static double angle = 0.0;

      static Triangle tr;

      SetMatrixMode(MatrixMode::PROJECTION);
      PushMatrix();

      SetOrthographic(-InitialTranslation * 0.5,  InitialTranslation * 0.5,
                       InitialTranslation * 0.5, -InitialTranslation * 0.5,
                       InitialTranslation * 0.5, -InitialTranslation * 0.5);

      SetMatrixMode(MatrixMode::MODELVIEW);
      PushMatrix();

      RotateZ(0.5   * angle);
      RotateY(0.25  * angle);
      RotateX(0.125 * angle);

      Translate(-1.0, 0.0, 0.0);

      for (auto& obj : Cube.GetScene().Objects)
      {
        for (auto& tri : obj.Triangles)
        {
          Enqueue(tri);
        }
      }

      PopMatrix();

      SetMatrixMode(MatrixMode::PROJECTION);
      PopMatrix();

      SetMatrixMode(MatrixMode::MODELVIEW);
      PushMatrix();

      Translate(5.0, 0.0, 15.0);

      for (auto& obj : Cube.GetScene().Objects)
      {
        for (auto& tri : obj.Triangles)
        {
          Enqueue(tri);
        }
      }

      PopMatrix();

      CommenceDraw();

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

        case AppMode::SHOW_AXES:
          DrawAxes();
          break;

        case AppMode::PIPELINE:
          RenderingPipeline();
          break;

        case AppMode::TWO_PROJECTIONS:
          TwoProjections();
          break;
      }
    }

    // -------------------------------------------------------------------------

    void DrawToScreen() override
    {
      IF::Instance().Printf(0, WindowHeight - 20,
                            IF::TextParams::Set(0xFFFF00,
                                                IF::TextAlignment::LEFT),
                            "%s",
                            AppModes.at(ApplicationMode).data());

      PRINTR(WindowWidth - 10, 10, "DX: %.2f", DX);
      PRINTR(WindowWidth - 10, 20, "DY: %.2f", DY);
      PRINTR(WindowWidth - 10, 30, "DZ: %.2f", DZ);

      PRINTR(WindowWidth - 10, 40,
            "Projection: %s", ProjectionModes.at(ProjectionMode_).data());

      PRINTR(WindowWidth - 10, 50,
            "Mode: %s", RenderModes.at(RenderMode_).data());

      PRINTR(WindowWidth - 10, WindowHeight - 140, "Projection matrix:");

      for (uint8_t x = 0; x < 4; x++)
      {
        for (uint8_t y = 0; y < 4; y++)
        {
          PRINTR(WindowWidth - 60 * (3 - y),
                 WindowHeight - 30 * (4 - x),
                 "%.2f  ",
                 _projectionMatrix[x][y]);
        }
      }

      if (ApplicationMode == AppMode::FROM_OBJ)
      {
        for (size_t i = 0; i < Loader.GetScene().Objects.size(); i++)
        {
          const auto& obj = Loader.GetScene().Objects[i];

          IF::Instance().Printf(WW / 2, 10 * (i + 1),
                                IF::TextParams::Set(0xFFFFFF, IF::TextAlignment::CENTER),
                                "%s (%llu polygons)",
                                obj.Name.data(), obj.Faces.size());
        }
      }

      if (ApplicationMode != AppMode::TEST)
      {
        PRINTL(10, 10, "%s", CullFaceModes.at(CullFaceMode_).data());
      }

      if (ApplicationMode == AppMode::PIPELINE)
      {
        PRINTL(10, 20, "draw time: %.2fms", DrawTime() * 1000.0);
      }

      if (Paused)
      {
        IF::Instance().Print(WindowWidth / 2,
                             WindowHeight - 20,
                             "PAUSED",
                             0x00FF00,
                             IF::TextAlignment::CENTER,
                             2.0);
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

// =============================================================================

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
