#include <cstdio>

#include "sw3d.h"
#include "scanline-rasterizer.h"

const std::string kRuler(80, '=');

#define EMIT_FAIL()  \
  printf("FAIL!\n"); \
  return;

#define HEADER()                   \
  do {                             \
    printf("\n");                  \
    printf("%s\n", kRuler.data()); \
    printf("%s\n", __func__);      \
    printf("%s\n", kRuler.data()); \
    printf("\n");                  \
  } while (false)

const std::unordered_map<TriangleType, std::string> TriangleTypeByType =
{
  { TriangleType::UNDEFINED,       "UNDEFINED"       },
  { TriangleType::FLAT_TOP,        "FLAT_TOP"        },
  { TriangleType::FLAT_BOTTOM,     "FLAT_BOTTOM"     },
  { TriangleType::MAJOR_RIGHT,     "MAJOR_RIGHT"     },
  { TriangleType::MAJOR_LEFT,      "MAJOR_LEFT"      },
  { TriangleType::HORIZONTAL_LINE, "HORIZONTAL LINE" },
  { TriangleType::VERTICAL_LINE,   "VERTICAL LINE"   },
};

const std::unordered_map<WindingOrder, std::string> WindingOrderByType =
{
  { WindingOrder::CW,  "CW"  },
  { WindingOrder::CCW, "CCW" }
};

// =============================================================================

void CheckAssign()
{
  HEADER();

  SW3D::Matrix m
  {
    {
      {  1,  2,  3,  4 },
      {  5,  6,  7,  8 },
      {  9, 10, 11, 12 },
      { 13, 14, 15, 16 },
    }
  };

  size_t counter = 1;
  for (size_t x = 0; x < 4; x++)
  {
    for (size_t y = 0; y < 4; y++)
    {
      if ((int)m[x][y] != counter)
      {
        EMIT_FAIL();
      }

      counter++;
    }
  }

  printf("%s\n", SW3D::ToString(m).data());

  printf("OK\n");
}

// =============================================================================

void MatrixNegativeCases()
{
  HEADER();

  // ---------------------------------------------------------------------------
  {
    SW3D::Matrix m
    {
      {
        { 1, 2, 3 },
        { 4, 5, 6 }
      }
    };

    SW3D::Vec3 v = { 1, 2, 3 };

    m * v;

    if (SW3D::Error != SW3D::EngineError::MATRIX_DIMENSIONS_ERROR)
    {
      EMIT_FAIL();
    }
  }
  // ---------------------------------------------------------------------------
  {
    SW3D::Matrix m
    {
      {
        { 1, 2, 3 },
        { 4, 5, 6 }
      }
    };

    SW3D::Vec4 v = { 1, 2, 3, 4 };

    m * v;

    if (SW3D::Error != SW3D::EngineError::MATRIX_DIMENSIONS_ERROR)
    {
      EMIT_FAIL();
    }
  }
  // ---------------------------------------------------------------------------
  {
    SW3D::Matrix m1
    {
      {
        { 1, 2, 3 },
        { 4, 5, 6 }
      }
    };

    SW3D::Matrix m2
    {
      {
        { 1, 2, 3 },
        { 4, 5, 6 },
      }
    };

    m1 * m2;

    if (SW3D::Error != SW3D::EngineError::MATRIX_DIMENSIONS_ERROR)
    {
      EMIT_FAIL();
    }
  }
  // ---------------------------------------------------------------------------
  {
    SW3D::Matrix m
    {
      {
        { 1, 2, 3 },
        { 4, 5, 6 }
      }
    };

    m.SetIdentity();

    if (SW3D::Error != SW3D::EngineError::MATRIX_NOT_SQUARE)
    {
      EMIT_FAIL();
    }
  }
  // ---------------------------------------------------------------------------
  {
    SW3D::Matrix m1
    {
      {
        { 1, 2, 3 },
        { 4, 5, 6 }
      }
    };

    SW3D::Matrix m2
    {
      {
        { 1, 2 },
        { 4, 5 }
      }
    };

    m1 + m2;

    if (SW3D::Error != SW3D::EngineError::MATRIX_DIMENSIONS_ERROR)
    {
      EMIT_FAIL();
    }
  }

  printf("OK\n");
}

// =============================================================================

void MatrixPositiveCases()
{
  HEADER();

  // ---------------------------------------------------------------------------
  {
    SW3D::Matrix m1(3, 3);

    m1.SetIdentity();

    SW3D::Matrix m2
    {
      {
        { 1, 2, 3 },
        { 4, 5, 6 },
        { 7, 8, 9 },
      }
    };

    SW3D::Matrix r = m1 * m2;

    if (r.Columns() != 3 and r.Rows() != 3)
    {
      EMIT_FAIL();
    }

    size_t counter = 1;
    for (size_t x = 0; x < 3; x++)
    {
      for (size_t y = 0; y < 3; y++)
      {
        if ((int)r[x][y] != counter)
        {
          EMIT_FAIL();
        }

        counter++;
      }
    }

    printf(SW3D::ToString(r).data());

    r = m2 * m1;

    if (r.Columns() != 3 and r.Rows() != 3)
    {
      EMIT_FAIL();
    }

    counter = 1;
    for (size_t x = 0; x < 3; x++)
    {
      for (size_t y = 0; y < 3; y++)
      {
        if ((int)r[x][y] != counter)
        {
          EMIT_FAIL();
        }

        counter++;
      }
    }

    printf("%s\n", SW3D::ToString(r).data());
  }
  // ---------------------------------------------------------------------------
  {
    SW3D::Matrix m1
    {
      {
        { 1, 1, 1 }
      }
    };

    SW3D::Matrix m2
    {
      {
        { 1 },
        { 1 },
        { 1 }
      }
    };

    SW3D::Matrix r = m1 * m2;

    if (r.Columns() != 1 and r.Rows() != 1)
    {
      EMIT_FAIL();
    }

    if ((int)r[0][0] != 3)
    {
      EMIT_FAIL();
    }

    printf("%s\n", SW3D::ToString(r).data());
  }
  // ---------------------------------------------------------------------------
  {
    SW3D::Matrix m1
    {
      {
        { 1, 1, 1 },
        { 1, 1, 1 }
      }
    };

    SW3D::Matrix m2
    {
      {
        { 1, 1 },
        { 1, 1 },
        { 1, 1 }
      }
    };

    SW3D::Matrix r = m1 * m2;

    if (r.Rows() != 2 and r.Columns() != 2)
    {
      EMIT_FAIL();
    }

    for (size_t x = 0; x < 2; x++)
    {
      for (size_t y = 0; y < 2; y++)
      {
        if ((int)r[x][y] != 3)
        {
          EMIT_FAIL();
        }
      }
    }

    printf("%s\n", SW3D::ToString(r).data());
  }
  // ---------------------------------------------------------------------------
  {
    SW3D::Matrix m
    {
      {
        { 2,  4 },
        { 8, 16 }
      }
    };

    m *= 2;

    size_t counter = 4;

    for (size_t x = 0; x < 2; x++)
    {
      for (size_t y = 0; y < 2; y++)
      {
        if ((int)m[x][y] != counter)
        {
          EMIT_FAIL();
        }

        counter *= 2;
      }
    }

    printf("%s\n", SW3D::ToString(m).data());
  }
  // ---------------------------------------------------------------------------
  {
    SW3D::Matrix m1
    {
      {
        { 1, 2, 3, 4 },
        { 5, 6, 7, 8 }
      }
    };

    SW3D::Matrix m2
    {
      {
        {  1,  2,  3 },
        {  4,  5,  6 },
        {  7,  8,  9 },
        { 10, 11, 12 }
      }
    };

    SW3D::Matrix r = m1 * m2;

    if (r.Rows() != 2 and r.Columns() != 3)
    {
      EMIT_FAIL();
    }

    if ((int)r[0][0] != 70
    and (int)r[0][1] != 80
    and (int)r[0][2] != 90
    and (int)r[1][0] != 158
    and (int)r[1][1] != 184
    and (int)r[1][2] != 210)
    {
      EMIT_FAIL();
    }

    printf("%s\n", SW3D::ToString(r).data());
  }
  // ---------------------------------------------------------------------------
  {
    SW3D::Matrix m
    {
      {
        { 2, 0, 0, 0 },
        { 0, 2, 0, 0 },
        { 0, 0, 2, 0 },
        { 0, 0, 0, 1 },
      }
    };

    SW3D::Vec3 v = { 1, 2, 3 };

    SW3D::Vec3 r = m * v;

    if ((int)r.X != 2 and (int)r.Y != 4 and (int)r.Z != 6)
    {
      EMIT_FAIL();
    }

    printf("%s\n", SW3D::ToString(r).data());
  }
  // ---------------------------------------------------------------------------
  {
    SW3D::Matrix m
    {
      {
        { 3, 0, 0 },
        { 0, 4, 0 },
        { 0, 0, 5 },
      }
    };

    SW3D::Vec3 v = { 1, 2, 3 };

    SW3D::Vec3 r = m * v;

    if ((int)r.X != 3 and (int)r.Y != 8 and (int)r.Z != 15)
    {
      EMIT_FAIL();
    }
  }

  printf("OK\n");
}

// =============================================================================

void ProjectionTests()
{
  HEADER();

  // ---------------------------------------------------------------------------
  {
    printf("Orthographic\n");

    SW3D::Matrix m = SW3D::Matrix::Orthographic(-2.0,  2.0,
                                                 2.0, -2.0,
                                                -2.0,  2.0);

    printf("%s\n", SW3D::ToString(m).data());

    std::vector<SW3D::Vec3> tri =
    {
      { -1.0, 0.0, 0.0 },
      {  1.0, 0.0, 0.0 },
      {  0.0, 1.0, 0.0 }
    };

    printf("Triangle before:\n");

    for (auto& v : tri)
    {
      printf("%s\n", SW3D::ToString(v).data());
      v = m * v;
    }

    printf("Triangle after:\n");

    for (auto& v : tri)
    {
      printf("%s\n", SW3D::ToString(v).data());
    }
  }
  // ---------------------------------------------------------------------------
  {
    printf("Perspective\n");

    SW3D::Matrix m = SW3D::Matrix::Perspective(60.0,
                                               1.0,
                                               0.1,
                                               1000.0);

    printf("%s\n", SW3D::ToString(m).data());

    //
    // The bigger the value of z in object space, the closer it is to 1 in world
    // space which can lead to infamous Z-fighting if two objects are too close
    // to far plane even though quite apart from each other in object space.
    // Like in example below: two points differ by the whole 10 in Z axis, but
    // projected values yield:
    //
    // < -0.0693, 0.0000, 0.9981 >
    // <  0.0866, 0.0000, 0.9976 >
    // < -0.0693, 0.0693, 0.9981 >
    //
    // which makes the difference minuscule in world space.
    //
    std::vector<SW3D::Vec3> tri =
    {
      { -2.0, 0.0, 50 },
      {  2.0, 0.0, 40 },
      { -2.0, 2.0, 50 }
    };

    printf("Triangle before:\n");

    for (auto& v : tri)
    {
      printf("%s\n", SW3D::ToString(v).data());
      v = m * v;
    }

    printf("Triangle after:\n");

    for (auto& v : tri)
    {
      printf("%s\n", SW3D::ToString(v).data());
    }
  }
  // ---------------------------------------------------------------------------

  printf("OK\n");
}

// =============================================================================

void Various()
{
  HEADER();

  const std::string ruler(80, '=');
  // ---------------------------------------------------------------------------
  {
    printf("Orthographic\n");

    SW3D::Matrix m = SW3D::Matrix::Orthographic(-2.0,  2.0,
                                                 2.0, -2.0,
                                                -2.0,  2.0);

    printf("%s\n", SW3D::ToString(m).data());

    for (size_t i = 0; i < 10; i++)
    {
      double offset = (double) i / 10;

      std::vector<SW3D::Vec3> tri =
      {
        { -1.5 - offset, 0.0, 1.0 + offset },
        {  1.5 + offset, 0.0, 1.0 + offset },
        {  0.0, 1.5 + offset, 1.0 + offset }
      };

      printf("Triangle before:\n");

      for (auto& v : tri)
      {
        printf("%s\n", SW3D::ToString(v).data());
        v = m * v;
      }

      printf("\nTriangle after:\n");

      for (auto& v : tri)
      {
        printf("%s\n", SW3D::ToString(v).data());
      }

      printf("%s\n", ruler.data());
    }
  }
  // ---------------------------------------------------------------------------
  {
    printf("Perspective\n");

    SW3D::Matrix m = SW3D::Matrix::Perspective(90.0, 1.0, 0.1, 1000.0);

    printf("%s\n", SW3D::ToString(m).data());

    for (size_t i = 0; i < 10; i++)
    {
      double offset = (double) i / 10;

      std::vector<SW3D::Vec3> tri =
      {
        { -1.5 - offset, 0.0, 1.0 + 2 * offset },
        {  1.5 + offset, 0.0, 1.0 + 2 * offset },
        {  0.0, 1.5 + offset, 1.0 + 2 * offset }
      };

      printf("Triangle before:\n");

      for (auto& v : tri)
      {
        printf("%s\n", SW3D::ToString(v).data());
        v = m * v;
      }

      printf("\nTriangle after:\n");

      for (auto& v : tri)
      {
        printf("%s\n", SW3D::ToString(v).data());
      }

      printf("%s\n", ruler.data());
    }
  }
  // ---------------------------------------------------------------------------
  {
    SW3D::Vec3 v1 = { 1.0, 0.0, 0.0 };
    SW3D::Vec3 v2 = { 0.0, 1.0, 0.0 };
    SW3D::Vec3 res = SW3D::CrossProduct(v1, v2);
    printf("%s x %s = %s\n",
            SW3D::ToString(v1).data(),
            SW3D::ToString(v2).data(),
            SW3D::ToString(res).data());
  }
  // ---------------------------------------------------------------------------
  {
    SW3D::Vec3 v1 = { 1.0, 0.0, 0.0 };
    SW3D::Vec3 v2 = { 0.0, 1.0, 0.0 };
    SW3D::Vec3 res = SW3D::CrossProduct(v2, v1);
    printf("%s x %s = %s\n",
            SW3D::ToString(v2).data(),
            SW3D::ToString(v1).data(),
            SW3D::ToString(res).data());
  }
  // ---------------------------------------------------------------------------
  {
    SW3D::Vec3 v1 = { 0.0, 1.0, 0.0 };
    SW3D::Vec3 v2 = { 0.0, 1.0, 0.0 };
    SW3D::Vec3 res = SW3D::CrossProduct(v1, v2);
    printf("%s x %s = %s\n",
            SW3D::ToString(v1).data(),
            SW3D::ToString(v2).data(),
            SW3D::ToString(res).data());
  }
  // ---------------------------------------------------------------------------
  {
    SW3D::Vec3 v1 = { 1.0, 0.0, 0.0 };
    SW3D::Vec3 v2 = { -1.0, 0.0, 0.0 };
    SW3D::Vec3 res = SW3D::CrossProduct(v1, v2);
    printf("%s x %s = %s\n",
            SW3D::ToString(v1).data(),
            SW3D::ToString(v2).data(),
            SW3D::ToString(res).data());
  }
  // ---------------------------------------------------------------------------
  {
    printf("%s\n", ruler.data());

    SW3D::Vec3 v1 = { 1.0, 0.0, 0.0 };
    for (double x = -1.0; x < 1.0; x += 0.1)
    {
      for (double y = -1.0; y < 1.0; y += 0.1)
      {
        SW3D::Vec3 v2 = { x, y, 0.0 };
        SW3D::Vec3 res = SW3D::CrossProduct(v1, v2);
        printf("%s x %s = %s\n",
                SW3D::ToString(v1).data(),
                SW3D::ToString(v2).data(),
                SW3D::ToString(res).data());
      }
    }
  }
}

// =============================================================================

void CrossProductTest()
{
  HEADER();

  ScanlineRasterizer r;

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

      WindingOrder wo = r.GetWindingOrder(toTest);

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

      WindingOrder wo = r.GetWindingOrder(toTest);

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

      r.SortVertices(toTest);
      r.CheckAndFixWinding(toTest);

      printf("After:\n%s", ToString(toTest).data());

      printf("Result - %s\n", (r.GetWindingOrder(toTest) == WindingOrder::CW)
                              ? "OK"
                              : "FAIL!");
    }
  }
}

// =============================================================================

//
// From these tests below it can easily be seen that by cyclic rotation of
// vertices left-to-right and right-to-left with the according enumeration we
// will exhaust all possible cases of 3 vertices definitions for a triangle.
//
void VertexSortingTest()
{
  HEADER();

  ScanlineRasterizer r;

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

      r.SortVertices(toTest);
      r.CheckAndFixWinding(toTest);

      printf("%s\n", ToString(toTest).data());

      TriangleType tt = r.GetTriangleType(toTest);
      WindingOrder wo = r.GetWindingOrder(toTest);

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

      r.SortVertices(toTest);
      r.CheckAndFixWinding(toTest);

      printf("%s\n", ToString(toTest).data());

      TriangleType tt = r.GetTriangleType(toTest);
      WindingOrder wo = r.GetWindingOrder(toTest);

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

      r.SortVertices(toTest);
      r.CheckAndFixWinding(toTest);

      printf("%s\n", ToString(toTest).data());

      TriangleType tt = r.GetTriangleType(toTest);
      WindingOrder wo = r.GetWindingOrder(toTest);

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

      r.SortVertices(toTest);
      r.CheckAndFixWinding(toTest);

      printf("%s\n", ToString(toTest).data());

      TriangleType tt = r.GetTriangleType(toTest);
      WindingOrder wo = r.GetWindingOrder(toTest);

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

  //
  // Edge case - horizontal line
  //
  {
    printf("=== Horizontal line ===\n\n");

    TriangleSimple t;
    t.Points[0] = { 10, 10, 0 };
    t.Points[1] = { 15, 10, 0 };
    t.Points[2] = { 20, 10, 0 };

    //
    //  1  3  2
    //
    TriangleSimple expected;
    expected.Points[0] = { 10, 10, 0 };
    expected.Points[1] = { 20, 10, 0 };
    expected.Points[2] = { 15, 10, 0 };

    for (const Indices& i : indices)
    {
      TriangleSimple toTest;
      toTest.Points[0] = t.Points[i.Ind[0]];
      toTest.Points[1] = t.Points[i.Ind[1]];
      toTest.Points[2] = t.Points[i.Ind[2]];

      r.SortVertices(toTest);
      r.CheckAndFixWinding(toTest);

      printf("%s\n", ToString(toTest).data());

      TriangleType tt = r.GetTriangleType(toTest);
      WindingOrder wo = r.GetWindingOrder(toTest);

      printf("Got expected vertices? %s\n",
             (toTest == expected) ? "OK" : "FAIL!");
      printf("TriangleType is: '%s' - %s\n",
             TriangleTypeByType.at(tt).data(),
             (tt == TriangleType::HORIZONTAL_LINE) ? "OK" : "FAIL!");
      printf("Winding order: '%s' - %s\n",
             WindingOrderByType.at(wo).data(),
             (wo == WindingOrder::CCW) ? "OK" : "FAIL!");
      printf("\n");
    }
  }

  // ---------------------------------------------------------------------------

  //
  // Edge case - horizontal line
  //
  {
    printf("=== Vertical line ===\n\n");

    TriangleSimple t;
    t.Points[0] = { 10, 10, 0 };
    t.Points[1] = { 10, 15, 0 };
    t.Points[2] = { 10, 20, 0 };

    //
    //  1
    //
    //  3
    //
    //  2
    //
    TriangleSimple expected;
    expected.Points[0] = { 10, 10, 0 };
    expected.Points[1] = { 10, 20, 0 };
    expected.Points[2] = { 10, 15, 0 };

    for (const Indices& i : indices)
    {
      TriangleSimple toTest;
      toTest.Points[0] = t.Points[i.Ind[0]];
      toTest.Points[1] = t.Points[i.Ind[1]];
      toTest.Points[2] = t.Points[i.Ind[2]];

      r.SortVertices(toTest);
      r.CheckAndFixWinding(toTest);

      printf("%s\n", ToString(toTest).data());

      TriangleType tt = r.GetTriangleType(toTest);
      WindingOrder wo = r.GetWindingOrder(toTest);

      printf("Got expected vertices? %s\n",
             (toTest == expected) ? "OK" : "FAIL!");
      printf("TriangleType is: '%s' - %s\n",
             TriangleTypeByType.at(tt).data(),
             (tt == TriangleType::VERTICAL_LINE) ? "OK" : "FAIL!");
      printf("Winding order: '%s' - %s\n",
             WindingOrderByType.at(wo).data(),
             (wo == WindingOrder::CCW) ? "OK" : "FAIL!");
      printf("\n");
    }
  }
}

// =============================================================================

int main(int argc, char* argv[])
{
  CheckAssign();
  MatrixNegativeCases();
  MatrixPositiveCases();
  ProjectionTests();
  Various();
  CrossProductTest();
  VertexSortingTest();

  return 0;
}
