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

#include <SDL2/SDL.h>

#define INIT_CHECK()                                           \
  if (not _initialized)                                        \
  {                                                            \
    SDL_Log("Drawer is not initialized - call Init() first!"); \
    return;                                                    \
  }

namespace SW3D
{
  enum class FrontFaceWinding
  {
    CW = 0,
    CCW
  };

  enum class FaceCullMode
  {
    FRONT = 0,
    BACK,
    BOTH
  };

  enum class MatrixMode_
  {
    PROJECTION = 0,
    MODELVIEW
  };

  enum class EngineError
  {
    OK = 0,
    DIVISION_BY_ZERO,
    MATRIX_NOT_SQUARE,
    MATRIX_DIMENSIONS_ERROR,
    STACK_OVERFLOW,
    STACK_UNDERFLOW,
    INVALID_MODE
  };

  enum class RenderMode
  {
    SOLID = 0,
    WIREFRAME,
    MIXED
  };

  EngineError Error = EngineError::OK;

  const char* ErrorToString()
  {
    switch (Error)
    {
      case EngineError::OK:
        return "OK";

      case EngineError::DIVISION_BY_ZERO:
        return "Division by zero";

      case EngineError::MATRIX_NOT_SQUARE:
        return "Matrix is not a square one";

      case EngineError::MATRIX_DIMENSIONS_ERROR:
        return "Matrix / matrices have incompatible dimensions";

      case EngineError::STACK_OVERFLOW:
        return "Stack overflow";

      case EngineError::STACK_UNDERFLOW:
        return "Stack underflow";

      case EngineError::INVALID_MODE:
        return "Invalid mode";

      default:
        return "Unknown error";
    }
  }

  constexpr double DEG2RAD    = M_PI / 180.0;
  constexpr double SQRT3OVER4 = 0.4330127018922193;

  const uint8_t kMatrixStackLimit = 32;

  using Clock = std::chrono::steady_clock;
  using ns    = std::chrono::nanoseconds;

  // ===========================================================================

  struct Vec2
  {
    double X = 0.0;
    double Y = 0.0;

    void operator*=(double value)
    {
      X *= value;
      Y *= value;
    }

    void operator+=(double value)
    {
      X += value;
      Y += value;
    }

    double Length()
    {
      return std::sqrt(X * X + Y * Y);
    }

    void Normalize()
    {
      double l = Length();

      if (l == 0)
      {
        SW3D::Error = EngineError::DIVISION_BY_ZERO;
        return;
      }

      X /= l;
      Y /= l;
    }

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

    void operator*=(double value)
    {
      X *= value;
      Y *= value;
      Z *= value;
    }

    void operator+=(double value)
    {
      X += value;
      Y += value;
      Z += value;
    }

    double Length()
    {
      return std::sqrt(X * X + Y * Y + Z * Z);
    }

    void Normalize()
    {
      double l = Length();

      if (l == 0.0)
      {
        SW3D::Error = EngineError::DIVISION_BY_ZERO;
        return;
      }

      X /= l;
      Y /= l;
      Z /= l;
    }

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
      static Vec3 v = { 0.0, 0.0, -1.0 };
      return v;
    }

    static Vec3 Out()
    {
      static Vec3 v = { 0.0, 0.0, 1.0 };
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

    void operator*=(double value)
    {
      X *= value;
      Y *= value;
      Z *= value;
      W *= value;
    }

    void operator+=(double value)
    {
      X += value;
      Y += value;
      Z += value;
      W += value;
    }

    double Length()
    {
      return std::sqrt(X * X + Y * Y + Z * Z + W * W);
    }

    void Normalize()
    {
      double l = Length();

      if (l == 0.0)
      {
        SW3D::Error = EngineError::DIVISION_BY_ZERO;
        return;
      }

      X /= l;
      Y /= l;
      Z /= l;
      W /= l;
    }
  };

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

  struct Triangle
  {
    Vec3 Points[3];
  };

  // ===========================================================================

  struct Mesh
  {
    std::vector<Triangle> Triangles;
  };

  // ===========================================================================

  using VV = std::vector<std::vector<double>>;

  struct Matrix
  {
    public:
      Matrix() : _rows(4), _cols(4)
      {
        Init();
      }

      // -----------------------------------------------------------------------

      Matrix(const Matrix& copy) :
        _rows(copy._rows), _cols(copy._cols), _matrix(copy._matrix)
      {
      }

      // -----------------------------------------------------------------------

      Matrix(const VV& data)
        : _matrix(data), _rows(data.size())
      {
        _cols = (data.size() != 0) ? data[0].size() : 0;
      }

      // -----------------------------------------------------------------------

      Matrix(uint32_t rows, uint32_t cols) : _rows(rows), _cols(cols)
      {
        Init();
      }

      // -----------------------------------------------------------------------

      void Init()
      {
        _matrix.resize(_rows);

        for (uint32_t i = 0; i < _rows; i++)
        {
          _matrix[i].resize(_cols);
        }

        Clear();
      }

      // -----------------------------------------------------------------------

      void Clear()
      {
        for (uint32_t x = 0; x < _rows; x++)
        {
          for (uint32_t y = 0; y < _cols; y++)
          {
            _matrix[x][y] = 0.0;
          }
        }
      }

      // -----------------------------------------------------------------------

      void SetIdentity()
      {
        if (_rows != _cols)
        {
          SW3D::Error = EngineError::MATRIX_NOT_SQUARE;
          return;
        }

        for (uint32_t x = 0; x < _rows; x++)
        {
          for (uint32_t y = 0; y < _cols; y++)
          {
            _matrix[x][y] = (x == y) ? 1.0 : 0.0;
          }
        }
      }

      // -----------------------------------------------------------------------

      const std::vector<double>& operator[](uint32_t row) const
      {
        return _matrix[row];
      }

      // -----------------------------------------------------------------------

      std::vector<double>& operator[](uint32_t row)
      {
        return _matrix[row];
      }

      // -----------------------------------------------------------------------

      const uint32_t& Rows() const
      {
        return _rows;
      }

      // -----------------------------------------------------------------------

      const uint32_t& Columns() const
      {
        return _cols;
      }

      // -----------------------------------------------------------------------

      Matrix operator*(const Matrix& rhs)
      {
        if (_cols != rhs._rows)
        {
          SW3D::Error = EngineError::MATRIX_DIMENSIONS_ERROR;
          return *this;
        }

        Matrix res(_rows, rhs._cols);

        for (uint32_t x = 0; x < res._rows; x++)
        {
          for (uint32_t y = 0; y < res._cols; y++)
          {
            for (uint32_t z = 0; z < _cols; z++) // or z < rhs._rows
            {
              res[x][y] += (_matrix[x][z] * rhs._matrix[z][y]);
            }
          }
        }

        return res;
      }

      // -----------------------------------------------------------------------

      Vec3 operator*(const Vec3& in)
      {
        Vec3 res;

        if (not (_rows == 3 or _rows == 4) )
        {
          SW3D::Error = EngineError::MATRIX_DIMENSIONS_ERROR;
          return res;
        }

        //
        // Simple multiplication.
        //
        if (_rows == 3)
        {
          res.X = in.X * _matrix[0][0] +
                  in.Y * _matrix[1][0] +
                  in.Z * _matrix[2][0];

          res.Y = in.X * _matrix[0][1] +
                  in.Y * _matrix[1][1] +
                  in.Z * _matrix[2][1];

          res.Z = in.X * _matrix[0][2] +
                  in.Y * _matrix[1][2] +
                  in.Z * _matrix[2][2];
        }
        //
        // Homo stuff.
        //
        else
        {
          //                   0 1 2 3
          //                 0 . . . .
          //                 1 . . . .
          // [ x y z 1 ]  X  2 . . . .
          //                 3 . . . .
          //
          res.X = in.X * _matrix[0][0] +
                  in.Y * _matrix[1][0] +
                  in.Z * _matrix[2][0] +
                         _matrix[3][0];

          res.Y = in.X * _matrix[0][1] +
                  in.Y * _matrix[1][1] +
                  in.Z * _matrix[2][1] +
                         _matrix[3][1];

          res.Z = in.X * _matrix[0][2] +
                  in.Y * _matrix[1][2] +
                  in.Z * _matrix[2][2] +
                         _matrix[3][2];

          //
          // Implicit conversion to so-called "homogeneous coordinates".
          // This will allow us to multiply 4x4 matrix by basically
          // Vec4(x, y, z, 1).
          //
          double w = in.X * _matrix[0][3] +
                     in.Y * _matrix[1][3] +
                     in.Z * _matrix[2][3] +
                            _matrix[3][3];

          //
          // Back to Cartesian.
          //
          if (w != 0.0)
          {
            res.X /= w;
            res.Y /= w;
            res.Z /= w;
          }
          else
          {
            SW3D::Error = EngineError::DIVISION_BY_ZERO;
          }
        }

        return res;
      }

      // -----------------------------------------------------------------------

      Vec4 operator*(const Vec4& in)
      {
        Vec4 res;

        if (_cols != 4)
        {
          SW3D::Error = EngineError::MATRIX_DIMENSIONS_ERROR;
          return res;
        }

        res.X = in.X * _matrix[0][0] +
                in.Y * _matrix[1][0] +
                in.Z * _matrix[2][0] +
                in.W * _matrix[3][0];

        res.Y = in.X * _matrix[0][1] +
                in.Y * _matrix[1][1] +
                in.Z * _matrix[2][1] +
                in.W * _matrix[3][1];

        res.Z = in.X * _matrix[0][2] +
                in.Y * _matrix[1][2] +
                in.Z * _matrix[2][2] +
                in.W * _matrix[3][2];

        res.W = in.X * _matrix[0][3] +
                in.Y * _matrix[1][3] +
                in.Z * _matrix[2][3] +
                in.W * _matrix[3][3];

        return res;
      }

      // -----------------------------------------------------------------------

      Matrix& operator*=(double value)
      {
        for (uint32_t x = 0; x < _rows; x++)
        {
          for (uint32_t y = 0; y < _cols; y++)
          {
            _matrix[x][y] *= value;
          }
        }

        return *this;
      }

      // -----------------------------------------------------------------------

      Matrix operator+(const Matrix& rhs)
      {
        if ( (_rows != rhs._rows) or (_cols != rhs._cols) )
        {
          SW3D::Error = EngineError::MATRIX_DIMENSIONS_ERROR;
          return *this;
        }

        Matrix res(_rows, _cols);

        for (uint32_t x = 0; x < _rows; x++)
        {
          for (uint32_t y = 0; y < _cols; y++)
          {
            res[x][y] = _matrix[x][y] + rhs._matrix[x][y];
          }
        }

        return res;
      }

      // -----------------------------------------------------------------------

      void operator=(const Matrix& rhs)
      {
        _matrix = rhs._matrix;
        _rows   = rhs._rows;
        _cols   = rhs._cols;
      }

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

        double f = 1.0 / std::tan( (fov * 0.5) * DEG2RAD );
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

  // ===========================================================================

  class DrawWrapper
  {
    public:
      virtual ~DrawWrapper()
      {
        SDL_Quit();
      }

      // -----------------------------------------------------------------------

      bool Init(uint16_t windowWidth,
                uint16_t windowHeight,
                uint16_t qualityReductionFactor = 1)
      {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
        {
          SDL_Log("SDL_Init() error: %s", SDL_GetError());
          return false;
        }

        if (qualityReductionFactor == 0)
        {
          SDL_Log("Resolution factor cannot be zero!");
          return false;
        }

        SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

        uint16_t canvasSize = std::min(windowWidth, windowHeight);

        //
        // Cannot add extra 1 to account for pretty debug grid size because
        // it will actually downscale texture into screen by 1 pixel and thus
        // introduce artifacts when in full resolution
        // (frameBufferSize == windowWidth == windowHeight).
        //
        _frameBufferSize = canvasSize / qualityReductionFactor;

        if (_frameBufferSize == 0)
        {
          SDL_Log("Canvas size is zero - increase quality!");
          return false;
        }

        _windowWidth  = windowWidth;
        _windowHeight = windowHeight;

        SDL_DisplayMode dm;
        if (SDL_GetCurrentDisplayMode(0, &dm) < 0)
        {
          SDL_Log("SDL_GetCurrentDisplayMode() error: %s", SDL_GetError());
          return false;
        }

        _window = SDL_CreateWindow(_windowName.data(),
                                   dm.w / 2 - _windowWidth / 2,
                                   dm.h / 2 - _windowHeight / 2,
                                   _windowWidth,
                                   _windowHeight,
                                   SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

        if (_window == nullptr)
        {
          SDL_Log("SDL_CreateWindow() error: %s", SDL_GetError());
          return false;
        }

        _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
        if (_renderer == nullptr)
        {
          SDL_Log("Failed to create renderer with SDL_RENDERER_ACCELERATED"
                  " - falling back to software");

          _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_SOFTWARE);
          if (_renderer == nullptr)
          {
            SDL_Log("Failed to create renderer: %s", SDL_GetError());
            return false;
          }
        }

        _framebuffer = SDL_CreateTexture(_renderer,
                                         SDL_PIXELFORMAT_RGBA32,
                                         SDL_TEXTUREACCESS_TARGET,
                                         _frameBufferSize,
                                         _frameBufferSize);
        if (_framebuffer == nullptr)
        {
          SDL_Log("Failed to create framebuffer: %s", SDL_GetError());
          return false;
        }

        _aspectRatio = (double)_windowHeight / (double)_windowWidth;

        SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);

        _projectionMatrix.SetIdentity();
        _modelViewMatrix.SetIdentity();

        _projectionStack.push(Matrix::Identity());
        _modelViewStack.push(Matrix::Identity());

        _initialized = true;

        PostInit();

        return true;
      }

      // -----------------------------------------------------------------------

      void Run(bool debugMode = false)
      {
        INIT_CHECK();

        SDL_Event evt;

        Clock::time_point measureStart;
        Clock::time_point measureEnd;

        ns dt = ns{0};

        while (_running)
        {
          measureStart = Clock::now();
          measureEnd = measureStart;

          while (SDL_PollEvent(&evt))
          {
            HandleEvent(evt);
          }

          SDL_SetRenderTarget(_renderer, _framebuffer);
          SDL_RenderClear(_renderer);

          if (debugMode)
          {
            DrawGrid();
          }

          Draw();

          SDL_SetRenderTarget(_renderer, nullptr);
          SDL_RenderCopy(_renderer, _framebuffer, nullptr, nullptr);

          SDL_RenderPresent(_renderer);

          _fps++;

          measureEnd = Clock::now();
          dt = measureEnd - measureStart;

          _deltaTime = std::chrono::duration<double>(dt).count();
          _dtAcc += _deltaTime;

          if (_dtAcc > 1.0)
          {
            static char buf[128];

            ::snprintf(buf, sizeof(buf), "FPS: %u", _fps);
            SDL_SetWindowTitle(_window, buf);

            _dtAcc = 0.0;
            _fps = 0;
          }
        }

        SDL_Log("Goodbye!");
      }

      // -----------------------------------------------------------------------

      void Stop()
      {
        _running = false;
      }

      // -----------------------------------------------------------------------

      void FrontFace(FrontFaceWinding windingToSet)
      {
        _frontFaceWinding = windingToSet;
      }

      // -----------------------------------------------------------------------

      void CullFace(FaceCullMode mode)
      {
        _faceCullMode = mode;
      }

      // -----------------------------------------------------------------------

      void MatrixMode(MatrixMode_ mode)
      {
        _matrixMode = mode;
      }

      // -----------------------------------------------------------------------

      void PushMatrix()
      {
        switch (_matrixMode)
        {
          case MatrixMode_::PROJECTION:
          {
            if (_projectionStack.size() < kMatrixStackLimit)
            {
              _projectionStack.push(_projectionMatrix);
            }
            else
            {
              SW3D::Error = EngineError::STACK_OVERFLOW;
            }
          }
          break;

          case MatrixMode_::MODELVIEW:
          {
            if (_modelViewStack.size() < kMatrixStackLimit)
            {
              _modelViewStack.push(_modelViewMatrix);
            }
            else
            {
              SW3D::Error = EngineError::STACK_OVERFLOW;
            }
          }
          break;

          default:
            SW3D::Error = EngineError::INVALID_MODE;
            break;
        }
      }

      // -----------------------------------------------------------------------

      void PopMatrix()
      {
        switch (_matrixMode)
        {
          case MatrixMode_::PROJECTION:
          {
            if (_projectionStack.size() > 1)
            {
              _projectionStack.pop();
              _projectionMatrix = _projectionStack.top();
            }
            else
            {
              SW3D::Error = EngineError::STACK_UNDERFLOW;
            }
          }
          break;

          case MatrixMode_::MODELVIEW:
          {
            if (_modelViewStack.size() > 1)
            {
              _modelViewStack.pop();
              _modelViewMatrix = _modelViewStack.top();
            }
            else
            {
              SW3D::Error = EngineError::STACK_UNDERFLOW;
            }
          }
          break;

          default:
            SW3D::Error = EngineError::INVALID_MODE;
            break;
        }
      }

      // -----------------------------------------------------------------------

      void DrawPoint(const SDL_Point& p, uint32_t colorMask)
      {
        INIT_CHECK();

        SaveColor();

        if (( colorMask & _maskA) != 0)
        {
          SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
        }
        else
        {
          SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_NONE);
        }

        HTML2RGBA(colorMask);

        SDL_SetRenderDrawColor(_renderer,
                                _drawColor.r,
                                _drawColor.g,
                                _drawColor.b,
                                _drawColor.a);

        SDL_RenderDrawPoint(_renderer, p.x, p.y);

        RestoreColor();
      }

      // -----------------------------------------------------------------------

      void DrawLine(const SDL_Point& p1,
                    const SDL_Point& p2,
                    uint32_t colorMask)
      {
        INIT_CHECK();

        SaveColor();

        if (( colorMask & _maskA ) != 0)
        {
          SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
        }
        else
        {
          SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_NONE);
        }

        HTML2RGBA(colorMask);

        SDL_SetRenderDrawColor(_renderer,
                                _drawColor.r,
                                _drawColor.g,
                                _drawColor.b,
                                _drawColor.a);

        SDL_RenderDrawLine(_renderer, p1.x, p1.y, p2.x, p2.y);

        RestoreColor();
      }

      // -----------------------------------------------------------------------

      void DrawTriangle(const SDL_Point& p1,
                        const SDL_Point& p2,
                        const SDL_Point& p3,
                        uint32_t colorMask,
                        RenderMode mode = RenderMode::SOLID)
      {
        INIT_CHECK();

        switch (mode)
        {
          case RenderMode::SOLID:
            FillTriangle(p1, p2, p3, colorMask);
            break;

          case RenderMode::WIREFRAME:
            DrawLine(p1, p2, colorMask);
            DrawLine(p2, p3, colorMask);
            DrawLine(p1, p3, colorMask);
            break;

          case RenderMode::MIXED:
            FillTriangle(p1, p2, p3, colorMask);
            DrawLine(p1, p2, 0);
            DrawLine(p2, p3, 0);
            DrawLine(p1, p3, 0);
            break;

          default:
            SDL_Log("Unexpected mode %d!", (int)mode);
            break;
        }
      }

      // -----------------------------------------------------------------------

      void DrawTriangle(const Vec3& p1,
                        const Vec3& p2,
                        const Vec3& p3,
                        uint32_t colorMask,
                        RenderMode mode = RenderMode::SOLID)
      {
        DrawTriangle(SDL_Point{ (int32_t)p1.X, (int32_t)p1.Y },
                     SDL_Point{ (int32_t)p2.X, (int32_t)p2.Y },
                     SDL_Point{ (int32_t)p3.X, (int32_t)p3.Y },
                     colorMask,
                     mode);
      }

      // -----------------------------------------------------------------------

      //
      // Simple rasterizer based on point inside triangle test.
      //
      void FillTriangle(const SDL_Point& p1,
                        const SDL_Point& p2,
                        const SDL_Point& p3,
                        uint32_t colorMask)
      {
        INIT_CHECK();

        int xMin = std::min( std::min(p1.x, p2.x), p3.x);
        int yMin = std::min( std::min(p1.y, p2.y), p3.y);
        int xMax = std::max( std::max(p1.x, p2.x), p3.x);
        int yMax = std::max( std::max(p1.y, p2.y), p3.y);

        for (int x = xMin; x <= xMax; x++)
        {
          for (int y = yMin; y <= yMax; y++)
          {
            SDL_Point p = { x, y };

            int w0 = CrossProduct(p1, p2, p);
            int w1 = CrossProduct(p2, p3, p);
            int w2 = CrossProduct(p3, p1, p);

            //
            // TODO: add user configurable front face vertex winding order into
            // account.
            //
            bool inside = (w0 <= 0 and w1 <= 0 and w2 <= 0)
                       or (w0 >= 0 and w1 >= 0 and w2 >= 0);

            if (inside)
            {
              DrawPoint(p, colorMask);
            }
          }
        }
      }

      // -----------------------------------------------------------------------

      SDL_Renderer* GetRenderer()
      {
        return _renderer;
      }

      // -----------------------------------------------------------------------

      const double& DeltaTime()
      {
        return _deltaTime;
      }

      // -----------------------------------------------------------------------

      const uint32_t& FrameBufferSize()
      {
        return _frameBufferSize;
      }

      // -----------------------------------------------------------------------

      void SetPerspective(double fov,
                          double aspectRatio,
                          double zNear,
                          double zFar)
      {
        _projectionMatrix = SW3D::Matrix::Perspective(fov,
                                                      aspectRatio,
                                                      zNear,
                                                      zFar);
      }

      // -----------------------------------------------------------------------

      void SetOrthographic(double left, double right,
                           double top,  double bottom,
                           double near, double far)
      {
        _projectionMatrix = SW3D::Matrix::Orthographic(left, right,
                                                        top, bottom,
                                                        near, far);
      }

      // -----------------------------------------------------------------------

    // *************************************************************************
    //
    //                               PROTECTED
    //
    // *************************************************************************

    protected:
      std::string _windowName = "DrawService window";

      Matrix _projectionMatrix;
      Matrix _modelViewMatrix;

      virtual void PostInit() {}

      virtual void Draw() = 0;
      virtual void HandleEvent(const SDL_Event& evt) = 0;

    // *************************************************************************
    //
    //                               PRIVATE
    //
    // *************************************************************************
    private:
      const SDL_Color& HTML2RGBA(const uint32_t& colorMask)
      {
        if (colorMask <= 0xFFFFFF)
        {
          _drawColor.r = (colorMask & _maskR) >> 16;
          _drawColor.g = (colorMask & _maskG) >> 8;
          _drawColor.b = (colorMask & _maskB);
          _drawColor.a = 0xFF;
        }
        else
        {
          _drawColor.a = (colorMask & _maskA) >> 24;
          _drawColor.r = (colorMask & _maskR) >> 16;
          _drawColor.g = (colorMask & _maskG) >> 8;
          _drawColor.b = (colorMask & _maskB);
        }

        return _drawColor;
      }

      // -----------------------------------------------------------------------

      int32_t CrossProduct(const SDL_Point& p1,
                           const SDL_Point& p2,
                           const SDL_Point& p)
      {
        static SDL_Point v1, v2;
        v1 = { p2.x - p1.x, p2.y - p1.y };
        v2 = { p.x  - p1.x, p.y  - p1.y };
        return (v1.x * v2.y - v1.y * v2.x);
      }

      // -----------------------------------------------------------------------

      void SaveColor()
      {
        SDL_GetRenderDrawColor(_renderer,
                               &_oldColor.r,
                               &_oldColor.g,
                               &_oldColor.b,
                               &_oldColor.a);
      }

      // -----------------------------------------------------------------------

      void RestoreColor()
      {
        SDL_SetRenderDrawColor(_renderer,
                               _oldColor.r,
                               _oldColor.g,
                               _oldColor.b,
                               _oldColor.a);
      }

      // -----------------------------------------------------------------------

      void DrawGrid()
      {
        SaveColor();

        SDL_SetRenderDrawColor(_renderer, 64, 64, 64, 255);

        for (int x = 0; x <= _frameBufferSize; x += 10)
        {
          for (int y = 0; y <= _frameBufferSize; y += 10)
          {
            SDL_RenderDrawPoint(_renderer, x, y);
          }
        }

        RestoreColor();
      }

      // -----------------------------------------------------------------------

      SDL_Renderer* _renderer = nullptr;
      SDL_Window* _window     = nullptr;

      SDL_Texture* _framebuffer = nullptr;

      uint32_t _frameBufferSize = 0;

      uint16_t _windowWidth  = 0;
      uint16_t _windowHeight = 0;

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

      FrontFaceWinding _frontFaceWinding = FrontFaceWinding::CCW;

      FaceCullMode _faceCullMode = FaceCullMode::BACK;

      std::stack<Matrix> _projectionStack;
      std::stack<Matrix> _modelViewStack;

      MatrixMode_ _matrixMode = MatrixMode_::PROJECTION;
  };

  // ***************************************************************************
  //
  //                           HELPER FUNCTIONS
  //
  // ***************************************************************************

  // ===========================================================================

  Vec3 RotateX(const Vec3& p, double angle)
  {
    Matrix r(3, 3);

    r[0][0] = 1.0;
    r[0][1] = 0.0;
    r[0][2] = 0.0;

    r[1][0] = 0.0;
    r[1][1] = std::cos(angle * DEG2RAD);
    r[1][2] = -std::sin(angle * DEG2RAD);

    r[2][0] = 0.0;
    r[2][1] = std::sin(angle * DEG2RAD);
    r[2][2] = std::cos(angle * DEG2RAD);

    return r * p;
  }

  // ===========================================================================

  Vec3 RotateY(const Vec3& p, double angle)
  {
    Matrix r(3, 3);

    r[0][0] = std::cos(angle * DEG2RAD);
    r[0][1] = 0.0;
    r[0][2] = std::sin(angle * DEG2RAD);

    r[1][0] = 0.0;
    r[1][1] = 1.0;
    r[1][2] = 0.0;

    r[2][0] = -std::sin(angle * DEG2RAD);
    r[2][1] = 0.0;
    r[2][2] = std::cos(angle * DEG2RAD);

    return r * p;
  }

  // ===========================================================================

  Vec3 RotateZ(const Vec3& p, double angle)
  {
    Matrix r(3, 3);

    r[0][0] = std::cos(angle * DEG2RAD);
    r[0][1] = -std::sin(angle * DEG2RAD);
    r[0][2] = 0.0;

    r[1][0] = std::sin(angle * DEG2RAD);
    r[1][1] = std::cos(angle * DEG2RAD);
    r[1][2] = 0.0;

    r[2][0] = 0.0;
    r[2][1] = 0.0;
    r[2][2] = 1.0;

    return r * p;
  }

  // ===========================================================================

  //
  // FIXME: doesn't work :-(
  //
  Vec3 Rotate(const Vec3& p, const Vec3& around, double angleDeg)
  {
    Vec3 res;

    Vec3 n = around;

    n.Normalize();

    double x = p.X;
    double y = p.Y;
    double z = p.Z;

    double x2 = p.X * p.X;
    double y2 = p.Y * p.Y;
    double z2 = p.Z * p.Z;

    double c = std::cos(angleDeg * DEG2RAD);
    double s = std::sin(angleDeg * DEG2RAD);

    double omc = 1.0 - c;

    double xs = p.X * s;
    double ys = p.Y * s;
    double zs = p.Z * s;

    Matrix r(3, 3);

    r[0][0] = x2 * omc + c;
    r[0][1] = x * y * omc - zs;
    r[0][2] = x * z * omc + ys;

    r[1][0] = y * x * omc + zs;
    r[1][1] = y2 * omc + c;
    r[1][2] = y * z * omc - xs;

    r[2][0] = x * z * omc - ys;
    r[2][1] = y * z * omc + xs;
    r[2][2] = z2 * omc + c;

    res = r * p;

    return res;
  }

  // ===========================================================================

  std::string ToString(const Matrix& m)
  {
    static std::stringstream ss;

    ss.str(std::string());

    static std::unordered_map<uint32_t, size_t> maxColumnLengthByColumn;

    maxColumnLengthByColumn.clear();

    size_t maxLength = 0;

    for (uint32_t y = 0; y < m.Columns(); y++)
    {
      maxLength = 0;

      for (uint32_t x = 0; x < m.Rows(); x++)
      {
        std::stringstream lss;

        lss << std::fixed << std::setprecision(4);

        lss << m[x][y];

        size_t ln = lss.str().length();
        if (ln > maxLength)
        {
          maxLength = ln;
        }
      }

      maxColumnLengthByColumn[y] = maxLength;
    }

    ss << std::fixed << std::setprecision(4);

    ss << "\n";

    for (uint32_t x = 0; x < m.Rows(); x++)
    {
      ss << "[ ";

      for (uint32_t y = 0; y < m.Columns(); y++)
      {
        std::stringstream lss;

        lss << std::fixed << std::setprecision(4);

        lss << m[x][y];

        for (uint32_t spaces = 0;
             spaces < (maxColumnLengthByColumn[y] - lss.str().length());
             spaces++)
        {
          ss << " ";
        }

        ss << m[x][y] << " ";
      }

      ss << "]\n";
    }

    return ss.str();
  }

  // ===========================================================================

  std::string ToString(const Vec2& v)
  {
    static std::stringstream ss;

    ss.str(std::string());

    ss << std::fixed << std::setprecision(4);

    ss << "< " << v.X << ", " << v.Y << " >\n";

    return ss.str();
  }

  // ===========================================================================

  std::string ToString(const Vec3& v)
  {
    static std::stringstream ss;

    ss.str(std::string());

    ss << std::fixed << std::setprecision(4);

    ss << "< " << v.X << ", " << v.Y << ", " << v.Z << " >\n";

    return ss.str();
  }

  // ===========================================================================

  std::string ToString(const Vec4& v)
  {
    static std::stringstream ss;

    ss.str(std::string());

    ss << std::fixed << std::setprecision(4);

    ss << "< " << v.X << ", " << v.Y << ", " << v.Z << ", " << v.W << " >\n";

    return ss.str();
  }

} // namespace sw3d

#endif // SW3D_H
