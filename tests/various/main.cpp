#include <cstdio>

#include "sw3d.h"

#define EMIT_FAIL() \
  SDL_Log("FAIL!");  \
  return;

// =============================================================================

void CheckAssign()
{
  SDL_Log(__func__);

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

  SDL_Log(SW3D::ToString(m).data());

  SDL_Log("OK");
}

// =============================================================================

void MatrixNegativeCases()
{
  SDL_Log(__func__);

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

  SDL_Log("OK");
}

// =============================================================================

void MatrixPositiveCases()
{
  SDL_Log(__func__);

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

    SDL_Log(SW3D::ToString(r).data());

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

    SDL_Log(SW3D::ToString(r).data());
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

    SDL_Log(SW3D::ToString(r).data());
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

    SDL_Log(SW3D::ToString(r).data());
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

    SDL_Log(SW3D::ToString(m).data());
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

    SDL_Log(SW3D::ToString(r).data());
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

    SDL_Log("%s", SW3D::ToString(r).data());
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

  SDL_Log("OK");
}

// =============================================================================

void ProjectionTests()
{
  SDL_Log(__func__);

  // ---------------------------------------------------------------------------
  {
    SDL_Log("Orthographic\n");

    SW3D::Matrix m = SW3D::Matrix::Orthographic(-2.0,  2.0,
                                                 2.0, -2.0,
                                                -2.0,  2.0);

    SDL_Log("%s\n", SW3D::ToString(m).data());

    std::vector<SW3D::Vec3> tri =
    {
      { -1.0, 0.0, 0.0 },
      {  1.0, 0.0, 0.0 },
      {  0.0, 1.0, 0.0 }
    };

    SDL_Log("Triangle before:\n");

    for (auto& v : tri)
    {
      SDL_Log("%s", SW3D::ToString(v).data());
      v = m * v;
    }

    SDL_Log("Triangle after:\n");

    for (auto& v : tri)
    {
      SDL_Log("%s", SW3D::ToString(v).data());
    }
  }
  // ---------------------------------------------------------------------------
  {
    SDL_Log("Perspective\n");

    SW3D::Matrix m = SW3D::Matrix::Perspective(60.0,
                                               1.0,
                                               0.1,
                                               1000.0);

    SDL_Log("%s\n", SW3D::ToString(m).data());

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

    SDL_Log("Triangle before:\n");

    for (auto& v : tri)
    {
      SDL_Log("%s", SW3D::ToString(v).data());
      v = m * v;
    }

    SDL_Log("Triangle after:\n");

    for (auto& v : tri)
    {
      SDL_Log("%s", SW3D::ToString(v).data());
    }
  }
  // ---------------------------------------------------------------------------

  SDL_Log("OK");
}

// =============================================================================

void Various()
{
  const std::string ruler(80, '=');
  // ---------------------------------------------------------------------------
  {
    SDL_Log("Orthographic\n");

    SW3D::Matrix m = SW3D::Matrix::Orthographic(-2.0,  2.0,
                                                 2.0, -2.0,
                                                -2.0,  2.0);

    SDL_Log("%s\n", SW3D::ToString(m).data());

    for (size_t i = 0; i < 10; i++)
    {
      double offset = (double) i / 10;

      std::vector<SW3D::Vec3> tri =
      {
        { -1.5 - offset, 0.0, 1.0 + offset },
        {  1.5 + offset, 0.0, 1.0 + offset },
        {  0.0, 1.5 + offset, 1.0 + offset }
      };

      SDL_Log("Triangle before:\n");

      for (auto& v : tri)
      {
        SDL_Log("%s", SW3D::ToString(v).data());
        v = m * v;
      }

      SDL_Log("\nTriangle after:\n");

      for (auto& v : tri)
      {
        SDL_Log("%s", SW3D::ToString(v).data());
      }

      SDL_Log("%s", ruler.data());
    }
  }
  // ---------------------------------------------------------------------------
  {
    SDL_Log("Perspective\n");

    SW3D::Matrix m = SW3D::Matrix::Perspective(90.0, 1.0, 0.1, 1000.0);

    SDL_Log("%s\n", SW3D::ToString(m).data());

    for (size_t i = 0; i < 10; i++)
    {
      double offset = (double) i / 10;

      std::vector<SW3D::Vec3> tri =
      {
        { -1.5 - offset, 0.0, 1.0 + 2 * offset },
        {  1.5 + offset, 0.0, 1.0 + 2 * offset },
        {  0.0, 1.5 + offset, 1.0 + 2 * offset }
      };

      SDL_Log("Triangle before:\n");

      for (auto& v : tri)
      {
        SDL_Log("%s", SW3D::ToString(v).data());
        v = m * v;
      }

      SDL_Log("\nTriangle after:\n");

      for (auto& v : tri)
      {
        SDL_Log("%s", SW3D::ToString(v).data());
      }

      SDL_Log("%s", ruler.data());
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
  //Various();

  return 0;
}
