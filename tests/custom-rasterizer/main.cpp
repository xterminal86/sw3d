#include <cstdio>

#include "sw3d.h"
#include "instant-font.h"
#include "blg.h"

using namespace SW3D;

const size_t QualityReductionFactor = 4;

size_t SelectionIndex = 0;

bool SubpixelDrawing    = false;
bool ScanlineRasterizer = true;

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

enum class TriangleType
{
  UNDEFINED = 0,
  FLAT_TOP,
  FLAT_BOTTOM,
  MAJOR_RIGHT,
  MAJOR_LEFT
};

enum class WindingOrder
{
  CW = 0,
  CCW
};

const std::unordered_map<TriangleType, std::string> TriangleTypeByType =
{
  { TriangleType::UNDEFINED,   "UNDEFINED"   },
  { TriangleType::FLAT_TOP,    "FLAT_TOP"    },
  { TriangleType::FLAT_BOTTOM, "FLAT_BOTTOM" },
  { TriangleType::MAJOR_RIGHT, "MAJOR_RIGHT" },
  { TriangleType::MAJOR_LEFT,  "MAJOR_LEFT"  }
};

const std::unordered_map<WindingOrder, std::string> WindingOrderByType =
{
  { WindingOrder::CW,  "CW"  },
  { WindingOrder::CCW, "CCW" }
};

// =============================================================================

double CrossProduct2D(const Vec3& v1, const Vec3& v2)
{
  return (v1.X * v2.Y - v1.Y * v2.X);
}

// =============================================================================

WindingOrder GetWindingOrder(const TriangleSimple& t)
{
  double cp = CrossProduct2D(t.Points[1] - t.Points[0],
                             t.Points[2] - t.Points[1]);

  return cp > 0 ? WindingOrder::CW : WindingOrder::CCW;
}

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

void CrossProductTest()
{
  struct Indices
  {
    uint8_t Ind[3];
  };

  //
  // CW
  //
  {
    const std::vector<Indices> indices =
    {
      { 0, 1, 2 },
      { 1, 2, 0 },
      { 2, 0, 1 }
    };

    TriangleSimple t;
    t.Points[0] = {  5, 10, 0 };
    t.Points[1] = {  7,  5, 0 };
    t.Points[2] = { 15, 15, 0 };

    for (const Indices& i : indices)
    {
      TriangleSimple toTest;
      toTest.Points[0] = t.Points[i.Ind[0]];
      toTest.Points[1] = t.Points[i.Ind[1]];
      toTest.Points[2] = t.Points[i.Ind[2]];

      WindingOrder wo = GetWindingOrder(toTest);

      printf("CW check - %s\n", (wo == WindingOrder::CW) ? "OK" : "FAIL!");
    }
  }

  //
  // CCW
  //
  {
    const std::vector<Indices> indices =
    {
      { 0, 1, 2 },
      { 1, 2, 0 },
      { 2, 0, 1 }
    };

    TriangleSimple t;
    t.Points[0] = {  7,  5, 0 };
    t.Points[1] = {  5, 10, 0 };
    t.Points[2] = { 15, 15, 0 };

    for (const Indices& i : indices)
    {
      TriangleSimple toTest;
      toTest.Points[0] = t.Points[i.Ind[0]];
      toTest.Points[1] = t.Points[i.Ind[1]];
      toTest.Points[2] = t.Points[i.Ind[2]];

      WindingOrder wo = GetWindingOrder(toTest);

      printf("CCW check - %s\n", (wo == WindingOrder::CCW) ? "OK" : "FAIL!");
    }
  }

  //
  // Fix winding
  //
  {
    //
    // All possible vertex ordering input.
    //
    const std::vector<Indices> indices =
    {
      { 0, 1, 2 },
      { 1, 2, 0 },
      { 2, 0, 1 },
      { 2, 1, 0 },
      { 1, 0, 2 },
      { 0, 2, 1 }
    };

    TriangleSimple t;
    t.Points[0] = {  7,  5, 0 };
    t.Points[1] = {  5, 10, 0 };
    t.Points[2] = { 15, 15, 0 };

    for (const Indices& i : indices)
    {
      TriangleSimple toTest;
      toTest.Points[0] = t.Points[i.Ind[0]];
      toTest.Points[1] = t.Points[i.Ind[1]];
      toTest.Points[2] = t.Points[i.Ind[2]];

      printf("Before:\n%s", ToString(toTest).data());

      SortVertices(toTest);
      CheckAndFixWinding(toTest);

      printf("After:\n%s", ToString(toTest).data());

      printf("Result - %s\n", (GetWindingOrder(toTest) == WindingOrder::CW)
                              ? "OK"
                              : "FAIL!");
    }
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
  // Because we decided on CW ordering we need to swap two vertices in certain
  // cases: 1 <-> 2 for FB and C (MR). It doesn't matter which ones are swapped,
  // but like with winding order you have to decide on one way and stick with
  // it. Later in actual filling code of a triangle we'll use three points as
  // arguments to a function, ordering of which will be expected (like for FT
  // triangle we'll start from point 3 and go up, for FB - from point 1 and go
  // down) and since our sorting and winding correction will end up with one
  // unambiguous result, we can manually pass points into rasterization function
  // in proper order.
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

  TriangleType res = TriangleType::UNDEFINED;

  if (t.Points[0].Y == t.Points[1].Y)
  {
    res = TriangleType::FLAT_TOP;
  }
  else if (t.Points[1].Y == t.Points[2].Y)
  {
    res = TriangleType::FLAT_BOTTOM;
  }
  else if (t.Points[1].Y > t.Points[2].Y)
  {
    res = TriangleType::MAJOR_RIGHT;
  }
  else if (t.Points[1].Y < t.Points[2].Y)
  {
    res = TriangleType::MAJOR_LEFT;
  }

  return res;
}

// =============================================================================

//
// From these tests below it can easily be seen that by cyclic rotation of
// vertices left-to-right and right-to-left with the according enumeration we
// will exhaust all possible cases of 3 vertices definitions for a triangle.
//
void SortingTest()
{
  struct Indices
  {
    uint8_t Ind[3];
  };

  //
  // All possible 3-vertex combinations.
  //
  const std::vector<Indices> indices =
  {
    { 0, 1, 2 },
    { 1, 2, 0 },
    { 2, 0, 1 },
    { 2, 1, 0 },
    { 1, 0, 2 },
    { 0, 2, 1 }
  };

  // ---------------------------------------------------------------------------

  //
  // Composite triangle processing will be kinda unfolded into processing of two
  // already solved cases, so winding order here is not that important, we just
  // need to correctly pass those 2 pairs of resulting 3-vertices for further
  // processing of FT and FB triangles accordingly.
  //
  // Composite (Major Right)
  //
  {
    printf("=== Composite (Major Right) ===\n\n");

    TriangleSimple t;
    t.Points[0] = {  5, 10, 0 };
    t.Points[1] = {  7,  5, 0 };
    t.Points[2] = { 15, 15, 0 };

    //
    //      1
    //
    //  3
    //
    //
    //           2
    //
    TriangleSimple expected;
    expected.Points[0] = {  7,  5, 0 };
    expected.Points[1] = { 15, 15, 0 };
    expected.Points[2] = {  5, 10, 0 };

    for (const Indices& i : indices)
    {
      TriangleSimple toTest;
      toTest.Points[0] = t.Points[i.Ind[0]];
      toTest.Points[1] = t.Points[i.Ind[1]];
      toTest.Points[2] = t.Points[i.Ind[2]];

      SortVertices(toTest);
      CheckAndFixWinding(toTest);

      printf("%s\n", ToString(toTest).data());

      TriangleType tt = GetTriangleType(toTest);
      WindingOrder wo = GetWindingOrder(toTest);

      printf("Got expected vertices? %s\n",
             (toTest == expected) ? "OK" : "FAIL!");
      printf("TriangleType is: '%s' - %s\n",
             TriangleTypeByType.at(tt).data(),
             (tt == TriangleType::MAJOR_RIGHT) ? "OK" : "FAIL!");
      printf("Winding order: '%s' - %s\n",
             WindingOrderByType.at(wo).data(),
             (wo == WindingOrder::CW) ? "OK" : "FAIL!");
      printf("\n");
    }

    printf("\n");
  }

  // ---------------------------------------------------------------------------

  //
  // Composite (Major Left)
  //
  {
    printf("=== Composite (Major Left) ===\n\n");

    TriangleSimple t;
    t.Points[0] = {  2, 15, 0 };
    t.Points[1] = {  7,  5, 0 };
    t.Points[2] = { 10, 10, 0 };

    //
    //      1
    //
    //         2
    //
    //
    //  3
    //
    TriangleSimple expected;
    expected.Points[0] = {  7,  5, 0 };
    expected.Points[1] = { 10, 10, 0 };
    expected.Points[2] = {  2, 15, 0 };

    for (const Indices& i : indices)
    {
      TriangleSimple toTest;
      toTest.Points[0] = t.Points[i.Ind[0]];
      toTest.Points[1] = t.Points[i.Ind[1]];
      toTest.Points[2] = t.Points[i.Ind[2]];

      SortVertices(toTest);
      CheckAndFixWinding(toTest);

      printf("%s\n", ToString(toTest).data());

      TriangleType tt = GetTriangleType(toTest);
      WindingOrder wo = GetWindingOrder(toTest);

      printf("Got expected vertices? %s\n",
             (toTest == expected) ? "OK" : "FAIL!");
      printf("TriangleType is: '%s' - %s\n",
             TriangleTypeByType.at(tt).data(),
             (tt == TriangleType::MAJOR_LEFT) ? "OK" : "FAIL!");
      printf("Winding order: '%s' - %s\n",
             WindingOrderByType.at(wo).data(),
             (wo == WindingOrder::CW) ? "OK" : "FAIL!");
      printf("\n");
    }

    printf("\n");
  }

  // ---------------------------------------------------------------------------

  //
  // Flat Top
  //
  {
    printf("=== Flat Top ===\n\n");

    TriangleSimple t;
    t.Points[0] = {  7, 10, 0 };
    t.Points[1] = {  5,  5, 0 };
    t.Points[2] = { 10,  5, 0 };

    //
    //  1     2
    //
    //     3
    //
    TriangleSimple expected;
    expected.Points[0] = {  5,  5, 0 };
    expected.Points[1] = { 10,  5, 0 };
    expected.Points[2] = {  7, 10, 0 };

    for (const Indices& i : indices)
    {
      TriangleSimple toTest;
      toTest.Points[0] = t.Points[i.Ind[0]];
      toTest.Points[1] = t.Points[i.Ind[1]];
      toTest.Points[2] = t.Points[i.Ind[2]];

      SortVertices(toTest);
      CheckAndFixWinding(toTest);

      printf("%s\n", ToString(toTest).data());

      TriangleType tt = GetTriangleType(toTest);
      WindingOrder wo = GetWindingOrder(toTest);

      printf("Got expected vertices? %s\n",
             (toTest == expected) ? "OK" : "FAIL!");
      printf("TriangleType is: '%s' - %s\n",
             TriangleTypeByType.at(tt).data(),
             (tt == TriangleType::FLAT_TOP) ? "OK" : "FAIL!");
      printf("Winding order: '%s' - %s\n",
             WindingOrderByType.at(wo).data(),
             (wo == WindingOrder::CW) ? "OK" : "FAIL!");
      printf("\n");
    }

    printf("\n");
  }

  // ---------------------------------------------------------------------------

  //
  // Flat Bottom
  //
  {
    printf("=== Flat Bottom ===\n\n");

    TriangleSimple t;
    t.Points[0] = { 10, 10, 0 };
    t.Points[1] = {  7,  5, 0 };
    t.Points[2] = {  5, 10, 0 };

    //
    //     1
    //
    //  3     2
    //
    TriangleSimple expected;
    expected.Points[0] = {  7,  5, 0 };
    expected.Points[1] = { 10, 10, 0 };
    expected.Points[2] = {  5, 10, 0 };

    for (const Indices& i : indices)
    {
      TriangleSimple toTest;
      toTest.Points[0] = t.Points[i.Ind[0]];
      toTest.Points[1] = t.Points[i.Ind[1]];
      toTest.Points[2] = t.Points[i.Ind[2]];

      SortVertices(toTest);
      CheckAndFixWinding(toTest);

      printf("%s\n", ToString(toTest).data());

      TriangleType tt = GetTriangleType(toTest);
      WindingOrder wo = GetWindingOrder(toTest);

      printf("Got expected vertices? %s\n",
             (toTest == expected) ? "OK" : "FAIL!");
      printf("TriangleType is: '%s' - %s\n",
             TriangleTypeByType.at(tt).data(),
             (tt == TriangleType::FLAT_BOTTOM) ? "OK" : "FAIL!");
      printf("Winding order: '%s' - %s\n",
             WindingOrderByType.at(wo).data(),
             (wo == WindingOrder::CW) ? "OK" : "FAIL!");
      printf("\n");
    }

    printf("\n");
  }

  // ---------------------------------------------------------------------------
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

      Rasterize(first, second);
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

      Rasterize(first, second);
    }

    // -------------------------------------------------------------------------

    void Rasterize(BLG& first, BLG& second)
    {
      BLG::Point* p1 = first.Next();
      BLG::Point* p2 = second.Next();

      while (true)
      {
        //
        // TODO: figure out why 'and' doesn't work.
        //
        if (p1 == nullptr or p2 == nullptr)
        {
          break;
        }

        int y1 = p1->second;
        int y2 = p2->second;

        if (y1 == y2)
        {
          for (int x = p1->first; x <= p2->first; x++)
          {
            SDL_RenderDrawPoint(_renderer, x, y1);
          }

          p1 = first.Next();
          p2 = second.Next();
        }
        else
        {
          if (y1 < y2)
          {
            p1 = first.Next();
          }
          else
          {
            p2 = second.Next();
          }
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
      // check as it is done in general case because here we can just manually
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
    void FillTriangleCustom(TriangleSimple& t)
    {
      SortVertices(t);
      CheckAndFixWinding(t);

      TriangleType tt = GetTriangleType(t);
      switch (tt)
      {
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

    void DrawToFrameBuffer() override
    {
      SaveColor();

      if (ScanlineRasterizer)
      {
        //HighlightPoints();
        SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
        FillTriangleCustom(CurrentTriangle);
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
              break;

            case SDLK_d:
              dx += 0.1;
              break;

            case SDLK_w:
              dy -= 0.1;
              break;

            case SDLK_s:
              dy += 0.1;
              break;

            case SDLK_m:
              ScanlineRasterizer = not ScanlineRasterizer;
              break;

            case SDLK_SPACE:
              SubpixelDrawing = not SubpixelDrawing;
              break;

            case SDLK_TAB:
              SelectionIndex++;
              SelectionIndex %= Triangles.size();
              CurrentTriangle = Triangles[SelectionIndex];
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
  //CrossProductTest();
  //SortingTest();

  CTF c;

  if (c.Init(800, 800, QualityReductionFactor))
  {
    IF::Instance().Init(c.GetRenderer());
    c.Run(true);
  }

  return 0;
}
