#ifndef SW3D_H
#define SW3D_H

#include <string>
#include <vector>
#include <chrono>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <stack>
#include <fstream>
#include <deque>

#include <SDL2/SDL.h>

#include "types.h"
#include "model-loader.h"

#define INIT_CHECK()                                           \
  if (not _initialized)                                        \
  {                                                            \
    SDL_Log("Drawer is not initialized - call Init() first!"); \
    return;                                                    \
  }

namespace SW3D
{
  using Clock = std::chrono::steady_clock;
  using ns    = std::chrono::nanoseconds;

  // ===========================================================================

  class DrawWrapper
  {
    public:
      // -----------------------------------------------------------------------

      bool Init(uint16_t windowWidth,
                uint16_t windowHeight,
                uint16_t qualityReductionFactor = 1);

      void Run(bool debugMode = false);

      SDL_Renderer* GetRenderer() const;

      const double& DeltaTime() const;
      const double& DrawTime() const;

      const uint32_t& FrameBufferSize() const;

    // *************************************************************************
    //
    //                               PROTECTED
    //
    // *************************************************************************

    protected:
      virtual ~DrawWrapper();

      void Stop();

      void DrawPoint(const SDL_Point& p, uint32_t colorMask);

      void DrawLine(const SDL_Point& p1,
                    const SDL_Point& p2,
                    uint32_t colorMask);

      void DrawTriangle(const SDL_Point& p1,
                        const SDL_Point& p2,
                        const SDL_Point& p3,
                        uint32_t colorMask,
                        RenderMode mode = RenderMode::SOLID);

      void DrawTriangle(const Vec3& p1,
                        const Vec3& p2,
                        const Vec3& p3,
                        uint32_t colorMask,
                        RenderMode mode = RenderMode::SOLID);

      void SetWeakPerspective();

      void SetPerspective(double fov,
                          double aspectRatio,
                          double zNear,
                          double zFar);

      void SetOrthographic(double left, double right,
                           double top,  double bottom,
                           double near, double far);

      void ShouldCullFace(const Vec3& lookVector, Triangle& face);
      void ApplyShading(const Vec3& lookVector, Triangle& face);

      //
      // Add drawing task to pipeline.
      //
      void Enqueue(const Triangle& t);

      //
      // glFlush() (or more correcly glFinish() I guess)
      //
      void CommenceDraw();

      //
      // Simple rasterizer based on point inside triangle test.
      //
      void FillTriangle(const SDL_Point& p1,
                        const SDL_Point& p2,
                        const SDL_Point& p3,
                        uint32_t colorMask);

      void SetCullFaceMode(CullFaceMode modeToSet);
      void SetMatrixMode(MatrixMode modeToSet);
      void SetRenderMode(RenderMode modeToSet);
      void SetShadingMode(ShadingMode modeToSet);

      void PushMatrix();
      void PopMatrix();

      SDL_Renderer* _renderer = nullptr;

      std::string _windowName = "DrawService window";

      Matrix _modelViewMatrix;
      Matrix _projectionMatrix;

      uint16_t _windowWidth  = 0;
      uint16_t _windowHeight = 0;

      virtual void PostInit() {}

      virtual void DrawToFrameBuffer() = 0;
      virtual void HandleEvent(const SDL_Event& evt) = 0;

      void RotateX(double angle);
      void RotateY(double angle);
      void RotateZ(double angle);

      void Translate(double dx, double dy, double dz);

      //
      // Additional draw to render target nullptr.
      //
      virtual void DrawToScreen() {}

    // *************************************************************************
    //
    //                               PRIVATE
    //
    // *************************************************************************
    private:
      struct PipelineItem
      {
        Triangle Face;
        Matrix _matProj;
        Matrix _matView;
      };

      const SDL_Color& HTML2RGBA(const uint32_t& colorMask);
      uint32_t Array2Mask(const uint8_t (&color)[4]);

      void SaveColor();
      void RestoreColor();

      void DrawGrid();

      SDL_Window* _window = nullptr;

      SDL_Texture* _framebuffer = nullptr;

      uint32_t _frameBufferSize = 0;

      uint32_t _fps = 0;

      double _drawTime = 0.0;
      double _deltaTime = 0.0;
      double _dtAcc = 0.0;

      double _aspectRatio = 0.0;

      bool _initialized        = false;
      bool _faceCullingEnabled = true;

      const uint32_t _maskR = 0x00FF0000;
      const uint32_t _maskG = 0x0000FF00;
      const uint32_t _maskB = 0x000000FF;
      const uint32_t _maskA = 0xFF000000;

      SDL_Color _drawColor;
      SDL_Color _oldColor;

      SDL_Rect _rect;

      bool _running = true;

      ProjectionMode _projectionMode = ProjectionMode::PERSPECTIVE;
      MatrixMode     _matrixMode     = MatrixMode::PROJECTION;
      RenderMode     _renderMode     = RenderMode::SOLID;
      CullFaceMode   _cullFaceMode   = CullFaceMode::BACK;
      ShadingMode    _shadingMode    = ShadingMode::FLAT;

      //
      // To store all translations and rotations.
      //
      std::stack<Matrix> _modelViewStack;

      //
      // To store all projections that may be.
      // We also need to save projection type to restore proper backface culling
      // after PopMatrix() is used.
      //
      std::stack<std::pair<Matrix, ProjectionMode>> _projectionStack;

      //
      // Drawing pipeline.
      //
      std::deque<Triangle> _pipeline;
  };

  // ***************************************************************************
  //
  //                           HELPER FUNCTIONS
  //
  // ***************************************************************************

  extern Vec3 RotateX(const Vec3& p, double angle);
  extern Vec3 RotateY(const Vec3& p, double angle);
  extern Vec3 RotateZ(const Vec3& p, double angle);

  //
  // FIXME: doesn't work :-(
  //
  extern Vec3 Rotate(const Vec3& p, const Vec3& around, double angleDeg);

  extern std::string ToString(const Matrix& m);
  extern std::string ToString(const Vec2& v);
  extern std::string ToString(const Vec3& v);
  extern std::string ToString(const Vec4& v);

  extern std::string ToString(const ModelLoader::Scene& s);

  extern double DotProduct(const Vec3& v1, const Vec3& v2);
  extern Vec3 CrossProduct(const Vec3& v1, const Vec3& v2);
} // namespace sw3d

#endif // SW3D_H
