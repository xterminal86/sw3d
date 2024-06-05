#include <cstdio>

#include "sw3d.h"

#define EMIT_FAIL() \
  SDL_Log("FAIL!");  \
  return;

// =============================================================================

void DumpVector(const SW3D::Vec3& v)
{
  static char buf[1024];

  ::snprintf(buf, sizeof(buf), "<%.2f, %.2f, %.2f>\n", v.X, v.Y, v.Z);

  SDL_Log(buf);
}

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

  SDL_Log(m.ToString().data());

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

    SDL_Log(r.ToString().data());

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

    SDL_Log(r.ToString().data());
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

    SDL_Log(r.ToString().data());
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

    SDL_Log(r.ToString().data());
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

    SDL_Log(m.ToString().data());
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

    SDL_Log(r.ToString().data());
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

    DumpVector(r);
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

int main(int argc, char* argv[])
{
  CheckAssign();
  MatrixNegativeCases();
  MatrixPositiveCases();

  return 0;
}
