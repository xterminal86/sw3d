//
// Trying to write some sort of software 3D renderer.
//
// Learning materials:
//
// -----------------------------------------------------------------------------
//
// https://www.youtube.com/@javidx9
//
// 3D Graphics Engine (multipart series, not very detailed, but still helpful
//                     and the code is intentionally oversimplified and thus
//                     easy to follow):
//
// Part 1: https://www.youtube.com/watch?v=ih20l3pJoeU
// Part 2: https://www.youtube.com/watch?v=XgMWc6LumG4
// Part 3: https://www.youtube.com/watch?v=HXSuNxpCzdM
//
// -----------------------------------------------------------------------------
//
// https://www.youtube.com/@ChiliTomatoNoodle
//
// 3D Fundamentals playlist (quite long, lots of details and stuff, but could be
//                           quite overwhelming in terms of code):
//
// https://www.youtube.com/playlist?list=PLqCJpWy5Fohe8ucwhksiv9hTF5sfid8lA
//
// -----------------------------------------------------------------------------
//
// My advice is to watch OLC videos first, because he presents with the end
// result (that is code) right away and in the simplest form possible (with lots
// of intentional copypastas because of it), and then research additional topics
// by yourself as they arise.
//
#include "sw3d.h"
#include "instant-font.h"

#include <map>

#define PRINTL(x, y, format, ...)                                    \
  IF::Instance().Printf(x, y,                                        \
                        IF::TextParams::Set(0xFFFFFF,                \
                                            IF::TextAlignment::LEFT, \
                                            1.0),                    \
                        format, ##__VA_ARGS__);


#define PRINTR(x, y, format, ...)                                     \
  IF::Instance().Printf(x, y,                                         \
                        IF::TextParams::Set(0xFFFFFF,                 \
                                            IF::TextAlignment::RIGHT, \
                                            1.0),                     \
                        format, ##__VA_ARGS__);

#define PRINTC(x, y, format, ...)                                      \
  IF::Instance().Printf(x, y,                                          \
                        IF::TextParams::Set(0xFFFFFF,                  \
                                            IF::TextAlignment::CENTER, \
                                            1.0),                      \
                        format, ##__VA_ARGS__);

using namespace SW3D;

const uint16_t WW = 640;
const uint16_t WH = 480;

const uint16_t QualityReductionFactor = 1;

uint16_t WindowWidth  = WW;
uint16_t WindowHeight = WH;

double DX = 0.0;
double DY = 0.0;
double DZ = 0.0;

const double RotationSpeed = 100.0;

bool Paused    = false;
bool ShowHelp  = false;
bool DepthTest = true;

CullFaceMode CullFaceMode_ = CullFaceMode::BACK;

size_t CullFaceModeIndex = (size_t)CullFaceMode::BACK;
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

size_t ProjectionModeIndex = (size_t)ProjectionMode::PERSPECTIVE;
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
  TWO_PROJECTIONS,
  TEXTURED
};

const std::unordered_map<AppMode, std::string> AppModes =
{
  { AppMode::TEST,            "Test cube, manual rendering"        },
  { AppMode::FROM_OBJ,        "Loaded from .obj, manual rendering" },
  { AppMode::SHOW_AXES,       "Default axes"                       },
  { AppMode::PIPELINE,        "Rendering pipeline"                 },
  { AppMode::TWO_PROJECTIONS, "Two projections"                    },
  { AppMode::TEXTURED,        "Textured"                           }
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
  "models/teapot.obj"
};

const std::string kAxesFname = "models/axes.obj";
SW3D::ModelLoader Axes;

const std::string kCubeFname = "models/cube.obj";
SW3D::ModelLoader Cube;

const std::vector<std::string> HelpText =
{
  "ESC   - exit",
  "TAB   - cycle render modes",
  "1-5   - switch scenes",
  "WASD  - move objects (where applicable)",
  "Q E   - move object along Z",
  "SPACE - toggle pause",
  "C     - cycle face culling mode",
  "P     - cycle projection mode",
  "[ ]   - change models (where applicable)"
};

size_t HelpTextLongestLine = 0;

int CheckerBoardTexture = -1;

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

    // -------------------------------------------------------------------------

    void RunTextureTests()
    {
      using P = std::pair<uint8_t, uint8_t>;

      const std::map<P, uint32_t> testCases =
      {
        { {  9,  1 }, 0xFFFFFF },
        { { 18,  2 }, 0xFF0000 },
        { { 12, 14 }, 0x00FF00 },
        { { 26, 15 }, 0xFF00FF },
        { { 18, 19 }, 0x0000FF },
        { { 23, 17 }, 0x00FFFF },
        { { 19, 23 }, 0xFFFF00 }
      };
      // -----------------------------------------------------------------------
      {
        const std::string texName = "textures/test24.bmp";
        SDL_Log("starting tests of '%s'", texName.data());

        int h = LoadTexture(texName);
        if (h != -1)
        {
          //printf("%s", DumpPixels(GetTexture(h)->Surface).data());

          for (auto& kvp : testCases)
          {
            const P& p = kvp.first;
            uint32_t color = ReadTexel(h, p.first, p.second);
            if (color == kvp.second)
            {
              SDL_Log("%u %u - PASS", p.first, p.second);
            }
            else
            {
              SDL_Log("%u %u - FAIL! actual 0x%X, expected 0x%X",
                      p.first, p.second, color, kvp.second);
            }
          }
        }
      }
      // -----------------------------------------------------------------------
      {
        const std::string texName = "textures/test32.bmp";
        SDL_Log("starting tests of '%s'", texName.data());

        int h = LoadTexture(texName);
        if (h != -1)
        {
          //printf("%s", DumpPixels(GetTexture(h)->Surface).data());

          for (auto& kvp : testCases)
          {
            const P& p = kvp.first;
            uint32_t color = ReadTexel(h, p.first, p.second);
            if (color == kvp.second)
            {
              SDL_Log("%u %u - PASS", p.first, p.second);
            }
            else
            {
              SDL_Log("%u %u - FAIL! actual 0x%X, expected 0x%X",
                      p.first, p.second, color, kvp.second);
            }
          }
        }
      }
    }

    // -------------------------------------------------------------------------

    void PostInit() override
    {
      for (auto& line : HelpText)
      {
        if (line.length() > HelpTextLongestLine)
        {
          HelpTextLongestLine = line.length();
        }
      }

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

      CheckerBoardTexture = LoadTexture("textures/checker.bmp");
      if (CheckerBoardTexture == -1)
      {
        SDL_Log("Couldn't load texture!");
      }

      //RunTextureTests();

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
              ProjectionMode_ = ProjectionMode::PERSPECTIVE;
              ProjectionModeIndex = (size_t)ProjectionMode::PERSPECTIVE;
              ApplyProjection();
            }
            break;

            case SDLK_6:
            {
              ApplicationMode = AppMode::TEXTURED;
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

            case SDLK_h:
              ShowHelp = not ShowHelp;
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

            case SDLK_z:
            {
              if (ApplicationMode == AppMode::PIPELINE)
              {
                DepthTest = not DepthTest;
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
          // Some vertices can share several faces and in our manually defined
          // cube some of them are listed more than once which is redundant.
          // Instead we need to specify minimum number of vertices, and then
          // draw triangles based on those vertices using faces enumeration.
          // We'll do exactly like that when we load model from .obj file.
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

      if (DepthTest)
      {
        ClearDepthBuffer();
      }

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

      /*
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
      */

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

      PRINTR(WindowWidth - 10, 60, "Draw calls: %llu", DrawCalls());

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
        IF::Instance().Printf(WindowWidth - 10, 70,
                              IF::TextParams::Set(0xFFFFFF,
                                                  IF::TextAlignment::RIGHT),
                              "Depth test: %s",
                              DepthTest ? "ON" : "OFF");
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

      if (ShowHelp)
      {
        static SDL_Rect dim;
        dim.x = WindowWidth / 2 - (HelpTextLongestLine / 2) * 10;
        dim.y = WindowHeight / 2 - ( (HelpText.size() / 2) * 10);
        dim.w = (HelpTextLongestLine - 2) * 10;
        dim.h = HelpText.size() * 10 + 20;

        static SDL_BlendMode oldBlend;

        SaveColor();
        SDL_GetRenderDrawBlendMode(_renderer, &oldBlend);
        SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(_renderer, 0, 64, 64, 200);
        SDL_RenderFillRect(_renderer, &dim);

        for (size_t i = 0; i < HelpText.size(); i++)
        {
          IF::Instance().Print(WindowWidth / 2 - (HelpTextLongestLine / 2 - 1) * 10,
                               WindowHeight / 2 - ( (HelpText.size() / 2 - 1) * 10 - i * 10),
                               HelpText[i],
                               0xFFFFFF,
                               IF::TextAlignment::LEFT,
                               1.0);
        }

        SDL_SetRenderDrawBlendMode(_renderer, oldBlend);
        RestoreColor();
      }
    }

    // -------------------------------------------------------------------------

    void DrawToFrameBuffer() override
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
