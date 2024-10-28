#include <cstdio>

#include "sw3d.h"
#include "instant-font.h"
#include "srtl.h"

using namespace SW3D;

const size_t QualityReductionFactor = 4;

bool Wireframe = false;

const double RotationSpeed = 50.0;

double AngleX = 0.0;
double AngleY = 0.0;
double AngleZ = 0.0;

// =============================================================================

class TLR : public DrawWrapper
{
  public:

    // -------------------------------------------------------------------------

    void PostInit() override
    {
      _rasterizer.Init(_renderer);

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

    // -------------------------------------------------------------------------

    void ApplyShading(const TriangleSimple& t)
    {
      static Vec3 v1, v2, n, fv;
      static double dp;

      v1 = t.Points[1] - t.Points[0];
      v2 = t.Points[0] - t.Points[2];
      n  = SW3D::CrossProduct(v1, v2);

      n.Normalize();

      fv = t.Points[0] - Vec3::Zero();

      fv.Normalize();

      dp = SW3D::DotProduct(fv, n);

      if (dp < 0.0)
      {
        dp = -dp;
      }

      uint8_t grayscalePart = (uint8_t)(255.0 * dp);

      //SDL_Log("%u", grayscalePart);
      SDL_SetRenderDrawColor(_renderer,
                             grayscalePart,
                             grayscalePart,
                             grayscalePart,
                             255);
    }

    // -------------------------------------------------------------------------

    bool ShouldCullFace(const TriangleSimple& t)
    {
      static Vec3 v1, v2, n, fv;
      static double dp;

      v1 = t.Points[1] - t.Points[0];
      v2 = t.Points[2] - t.Points[0];
      n  = SW3D::CrossProduct(v1, v2);

      //
      // Any vertex (since it's a triangle and they're all on the same plane)
      // minus camera pos (which is for now defaults to (0, 0, 0) ).
      //
      fv = t.Points[0] - Vec3::Zero();
      dp = SW3D::DotProduct(fv, n);

      //
      // In 3D we're going to kinda traditionally assume that CCW order for
      // vertices is a marker of a "front" face.
      //
      return (dp >= 0.0);
    }

    // -------------------------------------------------------------------------

    void DrawToFrameBuffer() override
    {
      SaveColor();

      SetWeakPerspective();

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
          tr.Points[i] = SW3D::RotateZ(tr.Points[i], AngleZ);
          tr.Points[i] = SW3D::RotateY(tr.Points[i], AngleY);
          tr.Points[i] = SW3D::RotateX(tr.Points[i], AngleX);
        }

        //
        // Translate.
        //
        TriangleSimple tt = tr;

        for (size_t i = 0; i < 3; i++)
        {
          tt.Points[i].Z = 2.5;
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
        }

        bool cf = ShouldCullFace(tp);
        if (not cf)
        {
          ApplyShading(tp);

          for (size_t i = 0; i < 3; i++)
          {
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

          _rasterizer.Rasterize(tp, Wireframe);
        }
      }

      RestoreColor();
    }

    // -------------------------------------------------------------------------

    void DrawToScreen() override
    {
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
              Wireframe = not Wireframe;
              break;

            case SDLK_a:
              AngleY -= (0.1 * RotationSpeed);
              break;

            case SDLK_d:
              AngleY += (0.1 * RotationSpeed);
              break;

            case SDLK_w:
              AngleX -= (0.1 * RotationSpeed);
              break;

            case SDLK_s:
              AngleX += (0.1 * RotationSpeed);
              break;

            default:
              break;
          }
        }
      }
    }

  private:
    Mesh _cube;

    //
    // BUG: some pixels are not filled.
    //
    SRTL _rasterizer;

    //ScanlineRasterizer _rasterizer;
};

// =============================================================================

int main(int argc, char* argv[])
{
  TLR c;

  if (c.Init(800, 800, QualityReductionFactor))
  {
    IF::Instance().Init(c.GetRenderer());
    c.Run(true);
  }

  return 0;
}
