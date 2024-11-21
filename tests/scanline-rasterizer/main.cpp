#include <cstdio>

#include "sw3d.h"
#include "instant-font.h"
#include "blg.h"

using namespace SW3D;

const size_t QualityReductionFactor = 4;

size_t SelectionIndex    = 0;
size_t CurrentPointIndex = 0;

bool SubpixelDrawing    = false;
bool ScanlineRasterizer = true;
bool Wireframe          = false;
bool HideGizmos         = false;

const TriangleSimple FlatTop =
{
  {
    { 50, 100, 0 }, { 150, 100, 0 }, { 100, 150, 0 }
  }
};

const TriangleSimple FlatBottom =
{
  {
    { 50, 100, 0 }, { 150, 100, 0 }, { 100, 50, 0 }
  }
};

const TriangleSimple CompositeMR =
{
  {
    { 100, 50, 0 }, { 50, 100, 0 }, { 150, 150, 0 }
  }
};

const TriangleSimple CompositeML =
{
  {
    { 100, 50, 0 }, { 150, 100, 0 }, { 50, 150, 0 }
  }
};

std::vector<TriangleSimple> Triangles =
{
  FlatTop,
  FlatBottom,
  CompositeMR,
  CompositeML
};

TriangleSimple CurrentTriangle = FlatTop;

double dx = 0.0;
double dy = 0.0;

Vec3* CurrentPoint = &CurrentTriangle.Points[CurrentPointIndex];

// =============================================================================

bool SortVertices(TriangleSimple& t)
{
  bool swapPerformed = false;

  bool sorted = false;

  while (not sorted)
  {
    sorted = true;

    for (size_t i = 0; i < 2; i++)
    {
      bool sortingCondition = (t.Points[i].Y >  t.Points[i + 1].Y)
                           or (t.Points[i].Y == t.Points[i + 1].Y
                           and t.Points[i].X >  t.Points[i + 1].X);
      if (sortingCondition)
      {
        std::swap(t.Points[i].X, t.Points[i + 1].X);
        std::swap(t.Points[i].Y, t.Points[i + 1].Y);
        sorted = false;
        swapPerformed = true;
      }
    }
  }

  return swapPerformed;
}

// =============================================================================

void CheckAndFixWinding(TriangleSimple& t)
{
  WindingOrder wo = GetWindingOrder(t);
  if (wo == WindingOrder::CCW)
  {
    //
    // Since we decided to stick to CW winding order if we encountered CCW one
    // just swap two adjacent vertices to change it.
    // We will swap 2nd and 3rd because we decided on certain configurations
    // of vertices for all triangle types.
    //
    std::swap(t.Points[1].X, t.Points[2].X);
    std::swap(t.Points[1].Y, t.Points[2].Y);
  }
}

// =============================================================================

TriangleType GetTriangleType(const TriangleSimple& t)
{
  //
  // Vertex winding order actually is important not only in 3D but in 2D as well
  // because it will determine all cases that we need to consider during
  // rasterization as well as making sure our rasterization satisfies "top-left"
  // rasterization rule (more on that later). As with 3D it doesn't matter which
  // one is chosen as long as you choose one and stick to it. And since in 2D
  // there is no "other side" we can convert any user defined vertex
  // configuration to our desired winding order unambiguously. We'll use CW
  // order here as it's more intuitive and straightforward given that on screen
  // X goes to the right and Y goes down. As far as I understand some popular
  // implementations use this winding as well.
  // Thus we can list all possible triangle configurations that can appear from
  // any 3 vertices:
  //
  // 1) Flat Top
  // 2) Flat Bottom
  // 3) Composite (Major Right)
  // 4) Composite (Major Left)
  //
  // By doing sorting of vertices by Y and then X we'll ensure following
  // possible vertex enumerations:
  //
  //    FT    |     FB    |    C (MR)   |    C (ML)
  //          |           |             |
  // 1     2  |     1     |      1      |      1
  //          |           |             |
  //          |           | - 3- - x -  |  - x- - 2- -
  //    3     |  2     3  |             |
  //          |           |          2  |  3
  //
  // Because we decided on CW ordering we need to swap vertices 1 <---> 2 in FB.
  // It doesn't matter which ones are swapped, but like with winding order you
  // have to decide on one way and stick with it. Later in actual filling code
  // of a triangle we'll use three points as arguments to a function, ordering
  // of which will be expected and since our sorting and winding correction will
  // end up with one unambiguous result, we can manually pass points into
  // rasterization function in proper order.
  // So you see, it's everything about order here.
  //
  // We still need to find splitting point 'x' to be able to rasterize composite
  // triangle using two 'primitive' ones.
  //
  // From C (MR) triangle we can designate:
  //
  // v3.y - v1.y
  // ----------- = a
  // v2.y - v1.y
  //
  // From this equation splitting point x can be defined as:
  //
  // x = (1 - a)v1 + av2
  //
  // We can rewrite this expression:
  //
  // x = v1 - av1 + av2 = v1 + av2 - av1
  //
  // +---------------------+
  // | x = v1 + a(v2 - v1) |
  // +---------------------+
  //
  // And thus we have a formula to determine splitting point given two others.
  //

  const int& y1 = (int)t.Points[0].Y;
  const int& y2 = (int)t.Points[1].Y;
  const int& y3 = (int)t.Points[2].Y;

  const int& x1 = (int)t.Points[0].X;
  const int& x2 = (int)t.Points[1].X;
  const int& x3 = (int)t.Points[2].X;

  if (y1 == y2 and y2 == y3)
  {
    return TriangleType::HORIZONTAL_LINE;
  }

  if (x1 == x2 and x2 == x3)
  {
    return TriangleType::VERTICAL_LINE;
  }

  if (y1 == y2)
  {
    return TriangleType::FLAT_TOP;
  }

  if (y2 == y3)
  {
    return TriangleType::FLAT_BOTTOM;
  }

  if (y2 > y3)
  {
    return TriangleType::MAJOR_RIGHT;
  }

  if (y2 < y3)
  {
    return TriangleType::MAJOR_LEFT;
  }

  return TriangleType::UNDEFINED;
}

// =============================================================================

class CTF : public DrawWrapper
{
  public:
    void DrawTriangleContour(const TriangleSimple& t)
    {
      if (SubpixelDrawing)
      {
        SDL_RenderDrawLineF(_renderer,
                            t.Points[0].X, t.Points[0].Y,
                            t.Points[1].X, t.Points[1].Y);

        SDL_RenderDrawLineF(_renderer,
                           t.Points[1].X, t.Points[1].Y,
                           t.Points[2].X + dx, t.Points[2].Y + dy);

        SDL_RenderDrawLineF(_renderer,
                           t.Points[2].X + dx, t.Points[2].Y + dy,
                           t.Points[0].X, t.Points[0].Y);
      }
      else
      {
        SDL_RenderDrawLine(_renderer,
                            t.Points[0].X, t.Points[0].Y,
                            t.Points[1].X, t.Points[1].Y);

        SDL_RenderDrawLine(_renderer,
                           t.Points[1].X, t.Points[1].Y,
                           (int)(t.Points[2].X + dx), (int)(t.Points[2].Y + dy));

        SDL_RenderDrawLine(_renderer,
                           (int)(t.Points[2].X + dx), (int)(t.Points[2].Y + dy),
                           t.Points[0].X, t.Points[0].Y);
      }
    }

    // -------------------------------------------------------------------------

    void DrawFT(const TriangleSimple& t)
    {
      //
      // Vertices in 't' are always:
      //
      // 1     2
      //
      //
      //    3
      //
      BLG first;
      first.Init(t.Points[0].X, t.Points[0].Y, t.Points[2].X, t.Points[2].Y);

      BLG second;
      second.Init(t.Points[1].X, t.Points[1].Y, t.Points[2].X, t.Points[2].Y);

      if (Wireframe)
      {
        RasterizeWireframe(first, second, t, TriangleType::FLAT_TOP);
      }
      else
      {
        Rasterize(first, second, t, TriangleType::FLAT_TOP);
      }
    }

    // -------------------------------------------------------------------------

    void DrawFB(const TriangleSimple& t)
    {
      //
      // Vertices in 't' are always:
      //
      //    1
      //
      //
      // 3     2
      //

      //
      // Lines must be (1 - 3) and (1 - 2) in this order because we use for loop
      // from x1 to x2. So left line must be first.
      //
      BLG first;
      first.Init(t.Points[0].X, t.Points[0].Y, t.Points[2].X, t.Points[2].Y);

      BLG second;
      second.Init(t.Points[0].X, t.Points[0].Y, t.Points[1].X, t.Points[1].Y);

      if (Wireframe)
      {
        RasterizeWireframe(first, second, t, TriangleType::FLAT_BOTTOM);
      }
      else
      {
        Rasterize(first, second, t, TriangleType::FLAT_BOTTOM);
      }
    }

    // -------------------------------------------------------------------------

    //
    // TODO: implement top-left rule.
    //
    // Why it's important:
    //
    // 1) Reduces pixel overdraw: e.g. with classical case of 2 triangles with
    //    one shared edge we get 1 extra overdraw:
    //
    //    1      2
    //     -----
    //    |    /|
    //    |  /  |
    //    |/    |
    //     -----
    //    3     4
    //
    //    Without some kind of tie-breaking rule edge 2-3 gets drawn one more
    //    time by one of the triangles, either 1-2-3 or 2-3-4.
    //
    //    1    2
    //     -----
    //     \   /|
    //    | \C/ |
    //    |/   \|
    //     -----
    //    3     4
    //
    //    In this configuration it's 1-C, 2-C, 3-C and 4-C that get drawn extra.
    //    So here we have 4 triangles and 4 overdraws.
    //
    //    It's fucking madskillz now, but I hope you get the idea.
    //
    //    So with 1000 triangles we could theoretically (maybe, I didn't check)
    //    have 500 wasted draw calls. This is unacceptable.
    //
    // 2) Blending will be fucked up along the shared edge because when you're
    //    doing blending you actually rely on certain draw order. And overdraw
    //    just breaks it.
    //
    void Rasterize(BLG& first,
                   BLG& second,
                   const TriangleSimple& t,
                   TriangleType tt)
    {
      BLG::Point* p1 = first.Next();
      BLG::Point* p2 = second.Next();

      if (p1 == nullptr or p2 == nullptr)
      {
        return;
      }

      int x1 = p1->first;
      int x2 = p2->first;

      int y1 = t.Points[0].Y;
      int y2 = t.Points[0].Y;

      PointCaptureType ctLine1 = PointCaptureType::UNDEFINED;
      PointCaptureType ctLine2 = PointCaptureType::UNDEFINED;

      switch (tt)
      {
        case TriangleType::FLAT_BOTTOM:
        {
          //
          // For cases like:
          //
          // 1
          //
          //
          //     3     2
          //
          ctLine1 = (t.Points[2].X <= t.Points[0].X)
                    ? PointCaptureType::LAST
                    : PointCaptureType::FIRST;

          ctLine2 = (t.Points[1].X <= t.Points[0].X)
                    ? PointCaptureType::FIRST
                    : PointCaptureType::LAST;

          y2 = t.Points[1].Y;
        }
        break;

        case TriangleType::FLAT_TOP:
        {
          ctLine1 = (t.Points[2].X <= t.Points[0].X)
                    ? PointCaptureType::LAST
                    : PointCaptureType::FIRST;

          ctLine2 = (t.Points[2].X <= t.Points[1].X)
                    ? PointCaptureType::FIRST
                    : PointCaptureType::LAST;

          y2 = t.Points[2].Y;
        }
        break;
      }

      //
      // Consider following configuration:
      //
      //              1
      //
      // 3            2
      //
      // In this case there is an extremely gentle slope of one side of a
      // triangle (1-3) which will have more points with the same Y compared to
      // the other side (1-2) which has few points, all of which with different
      // Y.
      //
      // Generally it's obvious that we need to connect leftmost points of the
      // left line with rightmost points of the right line to avoid pixel
      // overdraw. For example:
      //
      //                 1
      //               *...*
      //          *....     ....*
      //     *....               ....*
      //     3                       2
      //
      // '*' are the points we need. Another example:
      //
      //
      //     1                       2
      //     *....               ....*
      //          *....     ....*
      //               *...*
      //                 3
      //
      // The little problem is that our implementation is general - it handles
      // all cases so we need to check for each type of case to determine what
      // kind of X we need to remember. That is our generator lines always go
      // down (from min Y to max Y) and thus in FT case we need to save leftmost
      // and rightmost points as explained above, which happen to be first
      // points of a generator. But in FB situation is a little bit different:
      // generator lines still go down but this time they produce points from
      // "inside-out", so we can't just save first point of any generator and
      // wait until Y scanline changes.
      //
      for (int currentScanline = y1; currentScanline <= y2; currentScanline++)
      {
        if (p1 != nullptr
        and p1->second == currentScanline
        and ctLine1 == PointCaptureType::FIRST)
        {
          x1 = p1->first;
        }

        //
        // Wait until Y for left line changes.
        //
        while (p1 != nullptr and p1->second == currentScanline)
        {
          if (ctLine1 == PointCaptureType::LAST)
          {
            x1 = p1->first;
          }

          p1 = first.Next();
        }

        if (p2 != nullptr
        and p2->second == currentScanline
        and ctLine2 == PointCaptureType::FIRST)
        {
          x2 = p2->first;
        }

        //
        // Same here for right line.
        //
        while (p2 != nullptr and p2->second == currentScanline)
        {
          if (ctLine2 == PointCaptureType::LAST)
          {
            x2 = p2->first;
          }

          p2 = second.Next();
        }

        //
        // Here we moved to the next scanline, so we can connect previous one.
        //
        for (int x = x1; x <= x2; x++)
        {
          SDL_RenderDrawPoint(_renderer, x, currentScanline);
        }
      }
    }

    // -------------------------------------------------------------------------

    void RasterizeWireframe(BLG& first,
                            BLG& second,
                            const TriangleSimple& t,
                            TriangleType tt)
    {
      BLG::Point* p1 = first.Next();
      BLG::Point* p2 = second.Next();

      if (p1 == nullptr or p2 == nullptr)
      {
        return;
      }

      int x1 = p1->first;
      int x2 = p2->first;

      int y1 = t.Points[0].Y;
      int y2 = t.Points[0].Y;

      if (tt == TriangleType::FLAT_TOP)
      {
        y2 = t.Points[2].Y;

        for (int x = t.Points[0].X; x <= t.Points[1].X; x++)
        {
          SDL_RenderDrawPoint(_renderer, x, t.Points[0].Y);
        }
      }
      else if (tt == TriangleType::FLAT_BOTTOM)
      {
        y2 = t.Points[1].Y;

        for (int x = t.Points[2].X; x <= t.Points[1].X; x++)
        {
          SDL_RenderDrawPoint(_renderer, x, t.Points[1].Y);
        }
      }

      for (int currentScanline = y1; currentScanline <= y2; currentScanline++)
      {
        while (p1 != nullptr and p1->second == currentScanline)
        {
          SDL_RenderDrawPoint(_renderer, p1->first, p1->second);
          p1 = first.Next();
        }

        while (p2 != nullptr and p2->second == currentScanline)
        {
          SDL_RenderDrawPoint(_renderer, p2->first, p2->second);
          p2 = second.Next();
        }
      }
    }

    // -------------------------------------------------------------------------

    void DrawMR(const TriangleSimple& t)
    {
      //
      // Vertices in 't' are always:
      //
      //      1
      //
      // - 3- - x -
      //
      //          2
      //

      //
      // First we need to find splitting point. Recall:
      //
      // v3.y - v1.y
      // ----------- = a
      // v2.y - v1.y
      //
      // From this equation splitting point x can be defined as:
      //
      // x = (1 - a)v1 + av2
      //
      // We can rewrite this expression:
      //
      // x = v1 - av1 + av2 = v1 + av2 - av1
      //
      // +---------------------+
      // | x = v1 + a(v2 - v1) |
      // +---------------------+
      //

      double a = (t.Points[2].Y - t.Points[0].Y) / (t.Points[1].Y - t.Points[0].Y);

      //
      // Because multiplication from the left doesn't fucking work.
      //
      Vec3 x = t.Points[0] + (t.Points[1] - t.Points[0]) * a;

      //
      // We have successfully reduced task to already solved ones.
      //
      // So just delegate drawing of these cases to already workign methods,
      // just be sure to pass vertices in proper order. Since we control the
      // appearance of the new vertex there's no need to do sorting and winding
      // check as it was done in general case because here we can just manually
      // pass them properly.
      //
      //    1
      //
      // 3     x (2)
      //
      TriangleSimple fb = { t.Points[0], x, t.Points[2] };
      DrawFB(fb);

      //
      // 3 (1)    x (2)
      //
      //
      //
      //     2 (3)
      //
      TriangleSimple ft = { t.Points[2], x, t.Points[1] };
      DrawFT(ft);
    }

    // -------------------------------------------------------------------------

    void DrawML(const TriangleSimple& t)
    {
      //
      // Vertices in 't' are always:
      //
      //     1
      //
      // - x- - 2- -
      //
      // 3
      //

      double a = (t.Points[1].Y - t.Points[0].Y) / (t.Points[2].Y - t.Points[0].Y);
      Vec3 x = t.Points[0] + (t.Points[2] - t.Points[0]) * a;

      TriangleSimple fb = { t.Points[0], t.Points[1], x };
      DrawFB(fb);

      TriangleSimple ft = { x, t.Points[1], t.Points[2] };
      DrawFT(ft);
    }

    // -------------------------------------------------------------------------

    void DrawVL(const TriangleSimple& t)
    {
      //
      // Vertices in 't' are always:
      //
      // 1
      //
      // 3
      //
      // 2
      //

      int x  = t.Points[0].X;

      int y1 = t.Points[0].Y;
      int y2 = t.Points[1].Y;

      for (int scanline = y1; scanline <= y2; scanline++)
      {
        SDL_RenderDrawPoint(_renderer, x, scanline);
      }
    }

    // -------------------------------------------------------------------------

    void DrawHL(const TriangleSimple& t)
    {
      //
      // Vertices in 't' are always:
      //
      // 1  3  2
      //

      int scanline = t.Points[0].Y;

      int x1 = t.Points[0].X;
      int x2 = t.Points[1].X;

      for (int x = x1; x <= x2; x++)
      {
        SDL_RenderDrawPoint(_renderer, x, scanline);
      }
    }

    // -------------------------------------------------------------------------

    //
    // From https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage.html
    //
    // "In certain cases, a pixel may overlap more than one triangle, especially
    // when it lies precisely on an edge shared by two triangles. Such a pixel
    // would pass the coverage test for both triangles. If the triangles are
    // semi-transparent, a dark edge may appear where the pixels overlap the two
    // triangles due to the way semi-transparent objects are combined (imagine
    // two superimposed semi-transparent sheets of plastic. The surface becomes
    // more opaque and appears darker than the individual sheets).
    //
    // To address this issue, a rule is needed to ensure that a pixel can never
    // overlap two triangles sharing an edge twice. Most graphics APIs, such as
    // OpenGL and DirectX, define what they call the top-left rule. This rule
    // stipulates that a pixel or point is considered to overlap a triangle if
    // it is either inside the triangle or lies on a triangle's top edge or any
    // edge considered to be a left edge. What constitutes a top and left edge?
    //
    // 1) A top edge is an edge that is perfectly horizontal and whose defining
    //    vertices are positioned above the third one. Technically, this means
    //    that the y-coordinates of the vector V[ (X+1)%3 ] - V[X] are equal to
    //    0, and its x-coordinates are positive (greater than 0).
    // 2) A left edge is essentially an edge that is ascending. Given that
    //    vertices are defined in clockwise order in our context, an edge is
    //    considered ascending if its respective vector V[ (X+1)%3 ] - V[X]
    //    (where X can be 0, 1, 2) has a positive y-coordinate."
    //
    // --------------------------------------------------------------------
    //
    // Not to self: also from your own question at StackOverflow:
    //
    // https://stackoverflow.com/questions/78684610/need-help-understanding-top-left-rasterization-rule
    //
    // "The answer is in the poorly worded rule: "If two edges from the
    // same triangle touch the pixel center, then if both edges are "top"
    // or "left" then the sample is inside the triangle."
    // That means that corners are, by default, NOT drawn unless the
    // segments forming it are "top and left" or "left and left". (Top and
    // top isn't possible as that would make a degenerate triangle)."
    //
    void FillTriangleCustom(TriangleSimple& t)
    {
      SortVertices(t);
      CheckAndFixWinding(t);

      TriangleType tt = GetTriangleType(t);
      switch (tt)
      {
        case TriangleType::VERTICAL_LINE:
          DrawVL(t);
          break;

        case TriangleType::HORIZONTAL_LINE:
          DrawHL(t);
          break;

        case TriangleType::FLAT_TOP:
          DrawFT(t);
          break;

        case TriangleType::FLAT_BOTTOM:
          DrawFB(t);
          break;

        case TriangleType::MAJOR_RIGHT:
          DrawMR(t);
          break;

        case TriangleType::MAJOR_LEFT:
          DrawML(t);
          break;

        default:
          break;
      }
    }

    // -------------------------------------------------------------------------

    void HighlightPoint()
    {
      //
      // For some reason SDL_RenderDrawRect() draws like this:
      //
      // |
      // |-----
      // |     |
      // |     |
      // |     |
      //  -----
      //
      SDL_SetRenderDrawColor(_renderer, 0, 255, 255, 255);
      SDL_RenderDrawLine(_renderer,
                         CurrentPoint->X - 5,
                         CurrentPoint->Y - 5,
                         CurrentPoint->X + 5,
                         CurrentPoint->Y - 5);
      SDL_RenderDrawLine(_renderer,
                         CurrentPoint->X + 5,
                         CurrentPoint->Y - 5,
                         CurrentPoint->X + 5,
                         CurrentPoint->Y + 5);
      SDL_RenderDrawLine(_renderer,
                         CurrentPoint->X - 5,
                         CurrentPoint->Y - 5,
                         CurrentPoint->X - 5,
                         CurrentPoint->Y + 5);
      SDL_RenderDrawLine(_renderer,
                         CurrentPoint->X - 5,
                         CurrentPoint->Y + 5,
                         CurrentPoint->X + 5,
                         CurrentPoint->Y + 5);
    }

    // -------------------------------------------------------------------------

    void DrawToFrameBuffer() override
    {
      SaveColor();

      if (ScanlineRasterizer)
      {
        if (not HideGizmos)
        {
          HighlightPoint();
        }

        SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
        static TriangleSimple tmp;
        tmp = CurrentTriangle;
        FillTriangleCustom(tmp);
      }
      else
      {
        SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
        SDL_RenderDrawPointF(_renderer, dx, dy);

        SDL_SetRenderDrawColor(_renderer, 255, 0, 0, 255);
        SDL_RenderDrawPoint(_renderer, dx, dy + 6);

        SDL_SetRenderDrawColor(_renderer, 0, 255, 255, 255);
        DrawTriangleContour(CurrentTriangle);
      }

      RestoreColor();
    }

    // -------------------------------------------------------------------------

    void DrawToScreen() override
    {
      if (not HideGizmos)
      {
        if (not ScanlineRasterizer)
        {
          IF::Instance().Printf((int)(dx * QualityReductionFactor),
                                std::ceil(dy * QualityReductionFactor) + 20,
                                IF::TextParams::Set(0xFFFFFF,
                                                    IF::TextAlignment::LEFT,
                                                    2.0),
                                "dx = %.2f dy = %.2f (%d %d)",
                                dx, dy,
                                (int)std::round(dx),
                                (int)std::round(dy));

          IF::Instance().Printf(0, 680,
                                IF::TextParams::Set(0xFFFFFF,
                                                    IF::TextAlignment::LEFT,
                                                    2.0),
                                "subpixel mode %s",
                                SubpixelDrawing ? "ON" : "OFF");
        }
        else
        {
          IF::Instance().Printf(0, 0,
                                IF::TextParams::Set(0xFFFFFF,
                                                    IF::TextAlignment::LEFT,
                                                    2.0),
                                "CurrentPoint (%d %d)",
                                (int)CurrentPoint->X, (int)CurrentPoint->Y);
        }
      }
    }

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

            case SDLK_a:
              dx -= 0.1;
              CurrentPoint->X--;
              break;

            case SDLK_d:
              dx += 0.1;
              CurrentPoint->X++;
              break;

            case SDLK_w:
              dy -= 0.1;
              CurrentPoint->Y--;
              break;

            case SDLK_s:
              dy += 0.1;
              CurrentPoint->Y++;
              break;

            case SDLK_q:
              CurrentPoint->X--;
              CurrentPoint->Y--;
              break;

            case SDLK_e:
              CurrentPoint->X++;
              CurrentPoint->Y--;
              break;

            case SDLK_c:
              CurrentPoint->X++;
              CurrentPoint->Y++;
              break;

            case SDLK_z:
              CurrentPoint->X--;
              CurrentPoint->Y++;
              break;

            case SDLK_m:
              ScanlineRasterizer = not ScanlineRasterizer;
              break;

            case SDLK_f:
              Wireframe = not Wireframe;
              break;

            case SDLK_h:
              HideGizmos = not HideGizmos;
              break;

            case SDLK_SPACE:
              SubpixelDrawing = not SubpixelDrawing;
              break;

            case SDLK_TAB:
              SelectionIndex++;
              SelectionIndex %= Triangles.size();
              CurrentTriangle = Triangles[SelectionIndex];
              break;

            case SDLK_1:
              CurrentPointIndex = 0;
              CurrentPoint = &CurrentTriangle.Points[CurrentPointIndex];
              break;

            case SDLK_2:
              CurrentPointIndex = 1;
              CurrentPoint = &CurrentTriangle.Points[CurrentPointIndex];
              break;

            case SDLK_3:
              CurrentPointIndex = 2;
              CurrentPoint = &CurrentTriangle.Points[CurrentPointIndex];
              break;

            default:
              break;
          }
        }
      }
    }
};

// =============================================================================

int main(int argc, char* argv[])
{
  CTF c;

  if (c.Init(800, 800, QualityReductionFactor))
  {
    IF::Instance().Init(c.GetRenderer());
    c.Run(true);
  }

  return 0;
}
