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

      //
      // Simple rasterizer based on point inside triangle test.
      //
      void FillTriangle(const SDL_Point& p1,
                        const SDL_Point& p2,
                        const SDL_Point& p3,
                        uint32_t colorMask);

      SDL_Renderer* GetRenderer() const;

      const double& DeltaTime() const;

      const uint32_t& FrameBufferSize() const;

      void SetWeakPerspective();

      void SetPerspective(double fov,
                          double aspectRatio,
                          double zNear,
                          double zFar);

      void SetOrthographic(double left, double right,
                           double top,  double bottom,
                           double near, double far);

      // -----------------------------------------------------------------------

    // *************************************************************************
    //
    //                               PROTECTED
    //
    // *************************************************************************

    protected:
      virtual ~DrawWrapper();

      SDL_Renderer* _renderer = nullptr;

      std::string _windowName = "DrawService window";

      Matrix _projectionMatrix;

      uint16_t _windowWidth  = 0;
      uint16_t _windowHeight = 0;

      virtual void PostInit() {}

      virtual void DrawToFrameBuffer() = 0;
      virtual void HandleEvent(const SDL_Event& evt) = 0;

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
      const SDL_Color& HTML2RGBA(const uint32_t& colorMask);

      int32_t CrossProduct(const SDL_Point& p1,
                           const SDL_Point& p2,
                           const SDL_Point& p);

      void SaveColor();
      void RestoreColor();

      void DrawGrid();

      SDL_Window* _window = nullptr;

      SDL_Texture* _framebuffer = nullptr;

      uint32_t _frameBufferSize = 0;

      uint32_t _fps = 0;

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

      std::stack<Matrix> _projectionStack;
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

  extern std::string ToString(const ModelLoader::Model& m);

} // namespace sw3d

#endif // SW3D_H
