#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include <cstdint>
#include <cmath>

namespace SW3D
{
  namespace Constants
  {
    constexpr double DEG2RAD    = M_PI / 180.0;
    constexpr double SQRT3OVER4 = 0.4330127018922193;

    const uint8_t kMatrixStackLimit = 32;
  }

  enum class ProjectionMode
  {
    ORTHOGRAPHIC,
    WEAK_PERSPECTIVE,
    PERSPECTIVE
  };

  enum class CullFaceMode
  {
    FRONT = 0,
    BACK,
    NONE
  };

  enum class ShadingMode
  {
    NONE = 0,
    FLAT
  };

  enum class MatrixMode
  {
    PROJECTION = 0,
    MODELVIEW
  };

  enum class RenderMode
  {
    SOLID = 0,
    WIREFRAME,
    MIXED
  };

  enum class EngineError
  {
    OK = 0,
    NOT_INITIALIZED,
    DIVISION_BY_ZERO,
    MATRIX_NOT_SQUARE,
    MATRIX_DIMENSIONS_ERROR,
    STACK_OVERFLOW,
    STACK_UNDERFLOW,
    INVALID_MODE,
    FAILED_TO_LOAD_MODEL
  };

  enum class PointCaptureType
  {
    UNDEFINED = 0,
    FIRST,
    LAST
  };

  enum class TriangleType
  {
    UNDEFINED = 0,
    FLAT_TOP,
    FLAT_BOTTOM,
    MAJOR_RIGHT,
    MAJOR_LEFT,
    VERTICAL_LINE,
    HORIZONTAL_LINE
  };

  enum class WindingOrder
  {
    CW = 0,
    CCW
  };

  extern EngineError Error;

  const char* ErrorToString();

  // ===========================================================================

  struct Vec2
  {
    double X = 0.0;
    double Y = 0.0;

    Vec2 operator+(const Vec2& rhs) const;
    Vec2 operator-(const Vec2& rhs) const;
    Vec2 operator*(double value) const;

    void operator*=(double value);
    void operator+=(double value);

    double Length();

    void Normalize();

    static Vec2 Up()
    {
      static Vec2 v = { 0.0, 1.0 };
      return v;
    }

    static Vec2 Down()
    {
      static Vec2 v = { 0.0, -1.0 };
      return v;
    }

    static Vec2 Left()
    {
      static Vec2 v = { -1.0, 0.0 };
      return v;
    }

    static Vec2 Right()
    {
      static Vec2 v = { 1.0, 0.0 };
      return v;
    }
  };

  // ===========================================================================

  struct Vec3
  {
    double X = 0.0;
    double Y = 0.0;
    double Z = 0.0;

    Vec3 operator+(const Vec3& rhs) const;
    Vec3 operator-(const Vec3& rhs) const;
    Vec3 operator*(double value)    const;

    void operator*=(double value);
    void operator+=(double value);

    bool operator==(const Vec3& rhs);

    double Length();

    void Normalize();

    static Vec3 Zero()
    {
      static Vec3 v = { 0.0, 0.0, 0.0 };
      return v;
    }

    static Vec3 Up()
    {
      static Vec3 v = { 0.0, 1.0, 0.0 };
      return v;
    }

    static Vec3 Down()
    {
      static Vec3 v = { 0.0, -1.0, 0.0 };
      return v;
    }

    static Vec3 Left()
    {
      static Vec3 v = { -1.0, 0.0, 0.0 };
      return v;
    }

    static Vec3 Right()
    {
      static Vec3 v = { 1.0, 0.0, 0.0 };
      return v;
    }

    static Vec3 In()
    {
      static Vec3 v = { 0.0, 0.0, 1.0 };
      return v;
    }

    static Vec3 Out()
    {
      static Vec3 v = { 0.0, 0.0, -1.0 };
      return v;
    }
  };

  // ===========================================================================

  struct Vec4
  {
    double X = 0.0;
    double Y = 0.0;
    double Z = 0.0;
    double W = 1.0;

    Vec4() = default;
    Vec4(double x, double y, double z) : X(x), Y(y), Z(z), W(1.0) {}
    Vec4(double x, double y, double z, double w) : X(x), Y(y), Z(z), W(w) {}

    void operator*=(double value);
    void operator+=(double value);

    double Length();

    void Normalize();
  };

  // ===========================================================================

  namespace Directions
  {
    const Vec3 UP    = {  0.0,  1.0,  0.0 };
    const Vec3 DOWN  = {  0.0, -1.0,  0.0 };
    const Vec3 RIGHT = {  1.0,  0.0,  0.0 };
    const Vec3 LEFT  = { -1.0,  0.0,  0.0 };
    const Vec3 IN    = {  0.0,  0.0, -1.0 };
    const Vec3 OUT   = {  0.0,  0.0,  1.0 };
  }

  // ===========================================================================

  struct Vertex
  {
    Vec3 Position;
    Vec3 Normal;
    Vec2 UV;

    uint8_t Color[4] = { 255, 255, 255, 255 };
  };

  struct Triangle
  {
    Vertex Points[3];

    bool CullFlag = false;
    RenderMode  RenderMode_  = RenderMode::SOLID;
    ShadingMode ShadingMode_ = ShadingMode::FLAT;
  };

  struct TriangleSimple
  {
    Vec3 Points[3];

    bool operator==(const TriangleSimple& rhs);
  };

  struct Mesh
  {
    std::vector<TriangleSimple> Triangles;
  };

  // ===========================================================================

  using VV = std::vector<std::vector<double>>;

  struct Matrix
  {
    public:
      Matrix();
      Matrix(const Matrix& copy);
      Matrix(const VV& data);
      Matrix(uint32_t rows, uint32_t cols);

      void Init();
      void Clear();
      void SetIdentity();

      const std::vector<double>& operator[](uint32_t row) const;
      std::vector<double>& operator[](uint32_t row);

      // -----------------------------------------------------------------------

      const uint32_t& Rows() const;
      const uint32_t& Columns() const;

      // -----------------------------------------------------------------------

      Matrix operator*(const Matrix& rhs);
      Vec3 operator*(const Vec3& in);
      Vec4 operator*(const Vec4& in);

      Matrix& operator*=(double value);

      Matrix operator+(const Matrix& rhs);

      void operator=(const Matrix& rhs);

      // -----------------------------------------------------------------------

      static Matrix Identity()
      {
        static Matrix m(4, 4);
        return m;
      }

      // -----------------------------------------------------------------------

      static Matrix Orthographic(double left, double right,
                                 double top,  double bottom,
                                 double near, double far)
      {
        static Matrix m(4, 4);

        m.SetIdentity();

        if ( (right - left   == 0.0)
          or (top   - bottom == 0.0)
          or (far   - near   == 0.0) )
        {
          SW3D::Error = EngineError::DIVISION_BY_ZERO;
          return m;
        }

        m[0][0] = 2.0               / (right - left);
        m[0][3] = -( (right + left) / (right - left) );
        m[1][1] = 2.0               / (top   - bottom);
        m[1][3] = -( (top + bottom) / (top   - bottom) );
        m[2][2] = -2.0              / (far   - near);
        m[2][3] = -( (far + near)   / (far   - near) );
        m[3][3] = 1.0;

        return m;
      }

      // -----------------------------------------------------------------------

      //
      // This is actually surprisignly enough to produce descent perspective
      // effect, but it's a little bit different compared to "classic"
      // perspective projection (lines converge more profoundly and this
      // projection doesn't take into account display aspect ratio). This is
      // called "weak perspective projection" and according to Wikipedia:
      //
      // "The weak-perspective model thus approximates perspective projection
      // while using a simpler model, similar to the pure (unscaled)
      // orthographic perspective. It is a reasonable approximation when the
      // depth of the object along the line of sight is small compared to the
      // distance from the camera, and the field of view is small. With these
      // conditions, it can be assumed that all points on a 3D object are at the
      // same distance Zavg from the camera without significant errors in the
      // projection (compared to the full perspective model).
      //
      // Px = x / Zavg
      // Py = y / Zavg
      //
      // assuming focal length f = 1."
      //
      // Where Px and Py are "projected X" and "projected Y" accordingly.
      //
      static Matrix WeakPerspective()
      {
        static Matrix m(4, 4);

        m.SetIdentity();

        m[2][3] = 1.0;
        m[3][3] = 0.0;

        return m;
      }

      // -----------------------------------------------------------------------

      // ***********************************************************************
      //
      // OK, so I just wanted to make a detailed comment on why projection
      // matrix looks the way it is, but this is turning into a fucking article
      // now...
      //
      // Anyway, I'm kinda following along with OneLoneCoder videos on software
      // 3D renderer and recreating what he did, if possible without "cheating"
      // (not looking into his source code), and maybe try to add something from
      // myself as well.
      //
      // ***********************************************************************
      //
      //                           +------------+
      //                           |  PROLOGUE  |
      //                           +------------+
      //
      // Usually there's always a shortcut. You want to write a software 3D
      // renderer? Well, that's simple: you just take this matrix and
      // multiply every point / vertex by it, and you'll effectively put those
      // vertices onto the computer screen in proper places. #makingapoint
      // And while such approach might be acceptable or even required at certain
      // times, you won't get understanding from it.
      // I (personally) find it very counterintuitive because it doesn't answer
      // the question "why?". Why do you need to use a matrix and more
      // importantly where did all those values come from? Also (as it hopefully
      // will be demonstrated further and not even to a half of a possible
      // extent), the whole process of producing pixels on the screen from some
      // 3D vertices defined in a std::vector of doubles or something is not
      // that simple, and you'll have to follow down the rabbit hole for quite a
      // while if you want to understand the theory behind everything, and it
      // might hurt your head / be a waste of time or be a satisfying experience
      // depending on your background.
      // So, let's start the journey, shall we? :-)
      //
      //                              +---------+
      //                              |  INTRO  |
      //                              +---------+
      //
      // During research of the subject I found that it's quite common to
      // introduce the concept of so-called "pinhole camera model" to explain
      // how you can project 3D objects onto 2D screen.
      // If you have a darkroom with a "pinhole" (a hole of small size) in the
      // wall, rays of light that come through it will form an upside down image
      // on the opposite wall. The clarity of the resulting image will depend on
      // pinhole size, with it being too large results in blurry image due to
      // several light rays ending up at around the same point on the
      // "projection" wall. And also if pinhole is too small, resulting image
      // will also become blurry due to laws of physics (light can't go properly
      // through a very small hole due to diffraction).
      //
      // More detailed explanation:
      // (https://www.youtube.com/watch?v=_EhY31MSbNM)
      //
      // Looking from the side, it looks like this:
      //
      //        darkroom     world
      //         _______
      //        |       |
      //        |-----  |  ----
      //        I     --|--  #
      //       ###      o   ###
      //        #     --|--  I
      //        |-----  |  ----
      //        |       |
      //         -------
      //
      // We can try and work something out in terms of mathematics and shit if
      // we really want to, but that's actually not that important for this
      // particular topic of software 3D rendering:
      //
      // image
      // plane
      // |                                  P0
      // |                                 ----
      // |                         r0  ---- #  .
      // |optical             |    ----    ### .
      // |axis           z    |----         I  .
      // |---------------<----o--------------------
      // |.               ----|pinhole
      // |.           ----    |
      // |.       ----
      // |.   ----    ri
      // |----
      //  Pi
      //
      // |--------------------|
      //            f
      // _
      // r0 = (x0, y0, z0)
      //
      // It is obvious that zi = f, so any z of original point will have the
      // same projected z.
      // _
      // ri = (xi, yi, f)
      //
      // Using similar triangles:
      //
      // ri   r0      xi   x0  yi   y0
      // -- = --  ->  -- = --, -- = --
      // f    z0      f    z0  f    z0
      //
      // So objects exist in 3D space, but our screen is 2D space.
      // In order to draw 3D object onto the screen we need to find a way to
      // transform 3D coordinates to the 2D screen.
      // This is called a projection.
      //
      // Continuing with pinhole camera model analogy, we already can project
      // 3D object on the screen ("wall" that is), but it ends up upside down.
      // If only we could somehow capture light rays *before* they invert
      // themselves after passing through the hole... Obviously we can't do that
      // in real world - we can't put anything inside the hole, and we can't put
      // a wall in front of it. But we can do it in virtual world. So, suppose
      // we have some magic material (maybe kinda like photographic film) that
      // only captures light from our object of interest, that we can cut and
      // make a piece of which we can then put before the hole and "capture"
      // image from the world before it goes into the hole and invert itself.
      //
      // Now we get something like this:
      //
      //      magic     world
      // _    plane -----
      //  |   ------   ##
      //  |--- # |    ####
      //  o   ###|    ####
      //  |--- I |     II
      //  |   ------   II
      // -          -----
      //
      // Out "magic plane" actually becomes our computer screen.
      //
      //                      +----------------------+
      //                      |  RENDERING PIPELINE  |
      //                      +----------------------+
      //
      // To have a conceptual understanding on how 3D vertex becomes a pixel on
      // the screen one has to consider some stages that every vertex goes
      // through. Although this might look and sound like some general bullshit
      // (that's what I thought) accompanied by some not so fancy ASCII
      // graphics, they're actually very important. Some of which are so
      // important that without it we'll have one pixel instead of our 3D object
      // or we won't get any image at all. I'll specifically mention specific
      // stages in the code.
      //
      // +-------------+ 3D coordinates of an object relative to its own local
      // | MODEL SPACE | origin (which usually is the center of an object
      // +-------------+ itself). These are the coordinates of _cube.Triangles
      //       ||        in this project, for example, or coordinates of a 3D
      //       ||        model loaded from .obj file. Basically this is your
      //       ||        aforementioned "std::vector of doubles".
      //       ||
      //       \/
      // +-------------+ This is where your vertices end up after applying
      // | WORLD SPACE | translation / rotation matrices. This is where you
      // +-------------+ position your object inside the scene (aka "world").
      //       ||
      //       ||
      //       \/
      // +------------+ Also known as "camera space" or "eye space".
      // | VIEW SPACE | Additional rotation that's applied to world space
      // +------------+ coordinates that sets up the virtual camera. Basically
      //       ||       this stage is kinda optional, nothing stops you from
      //       ||       defining object the way you want in the first place, but
      //       ||       if you plan to move around it with virtual camera, it is
      //       ||       more convenient to apply additional step instead of
      //       ||       manually redefining vertices every frame so to speak.
      //       ||
      //       \/
      // +------------+ This is actually where your vertices end up after
      // | CLIP SPACE | multiplication with projection matrix. This is *not* the
      // +------------+ same as Normalized Device Coordinates (or NDC, more on
      //       ||       that later). Here you can check if your vertices go
      //       ||       outside view volume which is defined by -w <= x <= w,
      //       ||       -w <= y <= w, 0 <= z <= w and recreate additional
      //       ||       vertices on clip boundaries if needed and only *after*
      //       ||       that you can compress everything to NDC by dividing by
      //       ||       'w'. For some reason in OLC videos he implies that
      //       ||       projection itself gets you to NDC which is not true, but
      //       ||       since he uses 1 unit cube with all vertex components
      //       ||       having 0 to 1 values it "kinda" works, but actually it
      //       ||       might be really confusing if you're trying to understand
      //       ||       the whole theory behind rendering. Especially since he
      //       ||       introduces conecpt of clipping only in part 3 or
      //       ||       something of his video series.
      //       ||
      //       ||       More details:
      //       ||       https://learnopengl.com/Getting-started/Coordinate-Systems
      //       ||       https://carmencincotti.com/2022-05-02/homogeneous-coordinates-clip-space-ndc/
      //       \/
      // +------------+ Since now all our vertices are normalized to [ -1; 1 ]
      // | SCREEN MAP | we need to scale them back up to fit to the screen. To
      // +------------+ do that we need to shift coordinates by 1 first to bring
      //                them from [ -1 ; 1 ] to [ 0 ; 2 ] and then divide by 2
      //                to clamp them back to normalized screen space. Now we
      //                can just treat 'x' and 'y' components like a scaling
      //                coefficients for 2D point and multiply them by screen
      //                width and height respectively to get the final screen
      //                coordinates of a vertex where it ends up as a pixel.
      //                E.g. given x = [ 0.3 ; 0.1 ] in NDC space and screen
      //                dimensions of 600x600:
      //
      //                (not to scale)
      //
      //                -1          1
      //                 +----------+  1
      //                 |          |
      //                 |      x   |
      //                 |          |
      //                 |          |
      //                 +----------+ -1
      //
      //                x += 1.0   -> x = [ 1.3  ; 1.1  ]
      //                x /= 2.0   -> x = [ 0.65 ; 0.55 ]
      //                x * Screen -> x = [ 390  ; 330  ]
      //                DrawPixel(390, 330);
      //
      // All these stage transformations are done using matrices and they can
      // all be easily combined into one matrix by successive multiplication.
      // And since order of multiplication for matrices is important, the result
      // will look roughly like this:
      //
      // pixel = Mp * Mv * Mw * vertex
      //
      // where
      //
      // Mw - model to world matrix
      // Mv - world to view matrix
      // Mp - projection matrix
      //
      // We need to apply transformations in reverse order to that in which we
      // want them applied. So, if our desired order is MODEL-VIEW-PROJECTION,
      // we need to multiply by Mp first, then Mv and finally Mw. It is
      // perfectly fine to combine however many transformations you like this
      // way, like three translations followed by three rotations and so on,
      // just be mindful of the correct order of operations. For example, IIRC,
      // it was common to combine model and view matrix into one back in the
      // days of so-called "fixed function pipeline" (no, I'm not going down
      // this one!).
      //
      //                    +-------------------------+
      //                    |  INFAMOUS ASPECT RATIO  |
      //                    +-------------------------+
      //
      // Because displays have different aspect ratios we need to convert
      // object's coordinates to so-called Normalized Device
      // Coordinates (or NDC for short). In NDC everything is clamped in
      // [ -1 ; 1 ] range on every axis except 'z', where it's from 0 to 1.
      // Roughly speaking, this is done so that objects maintain the same
      // proportions on any device screen.
      //
      // For example, suppose we have display 2000x1000 which means its width is
      // two times greater than its height, thus giving us aspect ratio of 2.
      // This means that vertical number of pixels is less than number of
      // horizontal ones, so our image on the screen will be stretched
      // horizontally which is the same as being squished vertically. So we need
      // to compensate for this stretch / squish for an object to continue
      // remain "square" by making X coordinates smaller. The same principle
      // goes for vertical screen - this time we need to compensate for stretch
      // across screen height / squish across screen width by making X
      // coordinates larger.
      //
      // Since desktop screens usually have their width greater than their
      // height we'll define aspect ratio as 'w' / 'h'. It's really just a
      // matter of convention, we could've easily defined aspect ratio as
      // 'h' / 'w', just like in OLC video and some others I saw, it would just
      // resulted in multiplication of aspect ratio by coordinate in the matrix
      // instead of dividing coordinate over it. I.e.:
      //
      //      h          h           w
      // a = --- -> x * --- <=> x / ---
      //      w          w           h
      //
      // I like 'w' / 'h' better so that's what we're going to use.
      //
      // So if we have a tirangle (-1, -1), (1, -1), (0, 1)
      //
      // a = 2   -> (-0.5, -1) (0.5, -1), (0, 1)
      // a = 0.5 -> (-2,   -1) (2,   -1), (0, 1)
      //
      // So if our screen is vertical, triangle actually becomes "bigger" (it
      // will most likely go outside the screen), but proportions remain the
      // same.
      //
      //                 +--------------------------------+
      //                 |  PERSPECTIVE MATRIX EXPLAINED  |
      //                 +--------------------------------+
      //
      // First step is to divide 'x' coordinate of a given 3D point over aspect
      // ratio to take into account different screen width.
      //
      //                x
      // [x, y, z] = [ ---, y, z];
      //                a
      //
      // Next, we need to take into account Field Of View (FOV), which is
      // defined by angle theta (TH). You can also define perspective projection
      // matrix using different method (e.g. check glFrustum at docs.gl) by
      // specifying 6 planes, but using field of view is much more intuitive and
      // I believe uniquitous.
      //
      // -1                  +1
      //  ___________________    far plane
      //  \        |C       / D
      //   \       |       /
      //    \      |      /
      //     \     |     /
      //      \    |    /
      //       \   |A  / B
      //     -1 \-----/+1        near plane
      //         \ TH/
      //          \|/
      //           o
      //          eye
      //
      // Using intercept theorem we can see that:
      //
      // AB   oB
      // -- = --
      // CD   oD
      //
      //
      // By doing some mathmagics:
      //
      //
      // AB * oD = oB * CD     | / oB
      //
      //
      // AB * oD
      // ------- = CD          | / oD
      //    oB
      //
      //
      // AB   CD
      // -- = --
      // oB   oD
      //
      //
      // which is a tangent of (TH / 2).
      //
      // One can think of FOV as zooming in (decreasing FOV) and zooming out
      // (increasing FOV). When zoomed in our objects occupy more space and
      // appear larger. When zoomed out it's the opposite. But if we use just
      // the tan value we'd displace all our objects outside of FOV if it's
      // increasing, and scale them less conversely, which contradicts to
      // previous statements. So we actually need to use inverse of the tan
      // function. Basically one can think of it as just a global scaling
      // factor so to speak.
      //
      //                1          1              1
      // [x, y, z] = [ ---  *  --------- * x, --------- * y, z];
      //                a      tan(TH/2)      tan(TH/2)
      //
      // We could also normalize 'z' at this point as well, but it's better to
      // leave it unaffected so that it may participate in further calculations
      // (z-buffer and transparency).
      //
      // Nevertheless, we need to consider position of object in depth:
      //
      //           x - point in space
      //
      //      10
      // Z+ ^    ----------------- Zfar  -
      //    |    \               /    |   |
      //    |     \   x         /     |   |
      //    |      \           /      |   | (Zfar - Znear)
      //    |       \         /       |   |
      //    |        \_______/        |  _|
      //      1             | Znear   |
      //                    |         |
      //                 *__|_________|
      //                 ^
      //               (eye)
      //
      // To work out where position of point 'x' in a plane really is, we need
      // to scale it to normalized system. We do this by dividing over
      // (Zfar - Znear):
      //
      //       z
      // --------------
      // (Zfar - Znear)
      //
      // which will give us 'z' in range from 0 to 1. But our Zfar in this
      // example is 10, so we need to scale it back up again. We do this by
      // multiplying over Zfar:
      //
      //   z * Zfar
      // --------------
      // (Zfar - Znear)
      //
      // But we also need to offset our scaled point back closer to the eye
      // by the distance amount from the eye to Znear, which is Znear itself.
      // Since we've already normalized 'z' the offset should be in "normalized
      // mode" as well:
      //
      //   (Znear * Zfar)
      // - --------------
      //   (Zfar - Znear)
      //
      //
      // The end result will look like this:
      //
      //
      //         Zfar         (Znear * Zfar)
      // z * -------------- - --------------
      //     (Zfar - Znear)   (Zfar - Znear)
      //
      //
      // So, our total projection so far looks like this:
      //
      //                1          1
      // [x, y, z] = [ --- *  ---------- * x,
      //                a      tan(TH/2)
      //
      //                         1
      //                    ---------- * y,
      //                     tan(TH/2)
      //
      //                       Zfar         (Znear * Zfar)
      //               z * -------------- - -------------- ];
      //                   (Zfar - Znear)   (Zfar - Znear)
      //
      //
      // When things are further away, they appear to move less, so this implies
      // a change in 'x' coordinate is somehow related to 'z' depth, more
      // specifically it's inversely proportional, as 'z' gets larger it makes
      // changes in 'x' smaller:
      //
      //           1
      // x' = x * ---
      //           z
      //
      // The same goes for y:
      //
      //           1
      // y' = y * ---
      //           z
      //
      //
      // So our final scaling that we need to do to 'x' and 'y' coordinates is
      // to divide them by 'z', and so our formula becomes:
      //
      //                1        1        x
      // [x, y, z] = [ --- * --------- * ---,
      //                a    tan(TH/2)    z
      //
      //                         1        y
      //                    ---------- * ---,
      //                     tan(TH/2)    z
      //
      //                       Zfar         (Znear * Zfar)
      //               z * -------------- - -------------- ];
      //                   (Zfar - Znear)   (Zfar - Znear)
      //
      //
      // Let's simplify this a bit by making aliases:
      //
      //
      //         1
      // F = ---------
      //     tan(TH/2)
      //
      //
      //         Zfar
      // q = --------------
      //     (Zfar - Znear)
      //
      //
      // With this we can rewrite the transformations above as:
      //
      //
      //                Fx     Fy
      // [x, y, z] = [ ---- , ---- , q * (z - Znear) ]
      //                az     z
      //
      //
      // We can implement these equations directly, but in 3D graphics it's
      // common to use matrix multiplication, so we'll convert this to matrix
      // form.
      //
      //      -                 -
      //     |                   |
      //     | F/a 0  0          |
      //     |                   |
      //     | 0   F  0          |
      // M = |                   |
      //     | 0   0  q          |
      //     |                   |
      //     | 0   0  -Znear * q |
      //     |                   |
      //      -                 -
      //
      // Given like this, it is called the projection matrix. By multiplying
      // our 3D coordinates by this matrix we will transform them into
      // coordinates on the screen. But there is a problem. Our dimensions are
      // wrong: we cannot multiply 1x3 vector by 4x3 matrix. To solve this we
      // need to add another column to our matrix, thus making it 4 dimensional,
      // as well as add another component to our original 3D vector, which is
      // conventionally called 'w' (yes, this is that 'w' from above), and set
      // it equal to 1. It is said that such vector is now in "homogeneous
      // coordinates". We will also put 1 into cell [3][4] (one based index,
      // row-major order) of the projection matrix which will allow us to save
      // original 'z' value of a vector into 4th element 'w' of a resulting
      // vector after multiplication. Then we can divide by it to correct for
      // depth. We can explicitly add another coordinate into vector class or
      // calculate 'w' implicitly during matrix-vector multiplication and
      // perform divide by 'w' there, which exactly how it's done in this
      // project.
      //
      // v = [ x, y, z, 1 ]
      //
      //      -                    -
      //     | F                    |
      //     | -   0  0           0 |
      //     | a                    |
      //     |                      |
      //     | 0   F  0           0 |
      // M = |                      |
      //     | 0   0  q           1 |
      //     |                      |
      //     | 0   0  -Znear * q  0 |
      //     |                      |
      //      -                    -
      //
      // projected = v * M;
      //
      //        F
      // [ x * ---     y * F     (z * q - Znear * q)     z ]
      //        a
      //
      // Divide everything over 4th coordinate 'w' (which is effectively 'z') to
      // get back from homogeneous coordinates to Cartesian space.
      //
      //      xF
      // [ ( ---- ) / z     (y * F) / z     (z * q - z * (Znear * q)) / z     1 ]
      //      a
      //
      static Matrix Perspective(double fov,
                                double aspectRatio,
                                double zNear,
                                double zFar)
      {
        static Matrix m(4, 4);

        m.SetIdentity();

        double f = 1.0 / std::tan( (fov * 0.5) * Constants::DEG2RAD );
        double q = zFar / (zFar - zNear);

        m[0][0] = (f / aspectRatio);
        m[1][1] = f;
        m[2][2] = q;
        m[3][2] = -zNear * q;
        m[2][3] = 1.0;
        m[3][3] = 0.0;

        return m;
      }

    private:
      VV _matrix;

      uint32_t _rows;
      uint32_t _cols;
  };
}

#endif // TYPES_H
