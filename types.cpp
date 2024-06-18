#include "types.h"

namespace SW3D
{
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

      case EngineError::FAILED_TO_LOAD_MODEL:
        return "Failed to load model from file";

      default:
        return "Unknown error";
    }
  }

  // ===========================================================================

  void Vec2::operator*=(double value)
  {
    X *= value;
    Y *= value;
  }

  void Vec2::operator+=(double value)
  {
    X += value;
    Y += value;
  }

  double Vec2::Length()
  {
    return std::sqrt(X * X + Y * Y);
  }

  void Vec2::Normalize()
  {
    double l = Length();

    if (l == 0)
    {
      Error = EngineError::DIVISION_BY_ZERO;
      return;
    }

    X /= l;
    Y /= l;
  }

  // ===========================================================================

  void Vec3::operator*=(double value)
  {
    X *= value;
    Y *= value;
    Z *= value;
  }

  void Vec3::operator+=(double value)
  {
    X += value;
    Y += value;
    Z += value;
  }

  double Vec3::Length()
  {
    return std::sqrt(X * X + Y * Y + Z * Z);
  }

  void Vec3::Normalize()
  {
    double l = Length();

    if (l == 0.0)
    {
      Error = EngineError::DIVISION_BY_ZERO;
      return;
    }

    X /= l;
    Y /= l;
    Z /= l;
  }

  // ===========================================================================

  void Vec4::operator*=(double value)
  {
    X *= value;
    Y *= value;
    Z *= value;
    W *= value;
  }

  void Vec4::operator+=(double value)
  {
    X += value;
    Y += value;
    Z += value;
    W += value;
  }

  double Vec4::Length()
  {
    return std::sqrt(X * X + Y * Y + Z * Z + W * W);
  }

  void Vec4::Normalize()
  {
    double l = Length();

    if (l == 0.0)
    {
      Error = EngineError::DIVISION_BY_ZERO;
      return;
    }

    X /= l;
    Y /= l;
    Z /= l;
    W /= l;
  }

  // ===========================================================================

  Matrix::Matrix() : _rows(4), _cols(4)
  {
    Init();
  }

  Matrix::Matrix(const Matrix& copy) :
    _rows(copy._rows),
    _cols(copy._cols),
    _matrix(copy._matrix) {}

  Matrix::Matrix(const VV& data)
    : _matrix(data), _rows(data.size())
  {
    _cols = (data.size() != 0) ? data[0].size() : 0;
  }

  Matrix::Matrix(uint32_t rows, uint32_t cols) : _rows(rows), _cols(cols)
  {
    Init();
  }

  // ---------------------------------------------------------------------------

  void Matrix::Init()
  {
    _matrix.resize(_rows);

    for (uint32_t i = 0; i < _rows; i++)
    {
      _matrix[i].resize(_cols);
    }

    Clear();
  }

  // ---------------------------------------------------------------------------

  void Matrix::Clear()
  {
    for (uint32_t x = 0; x < _rows; x++)
    {
      for (uint32_t y = 0; y < _cols; y++)
      {
        _matrix[x][y] = 0.0;
      }
    }
  }

  // ---------------------------------------------------------------------------

  void Matrix::SetIdentity()
  {
    if (_rows != _cols)
    {
      Error = EngineError::MATRIX_NOT_SQUARE;
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

  // ---------------------------------------------------------------------------

  const std::vector<double>& Matrix::operator[](uint32_t row) const
  {
    return _matrix[row];
  }

  // ---------------------------------------------------------------------------

  std::vector<double>& Matrix::operator[](uint32_t row)
  {
    return _matrix[row];
  }

  // ---------------------------------------------------------------------------

  Matrix Matrix::operator*(const Matrix& rhs)
  {
    if (_cols != rhs._rows)
    {
      Error = EngineError::MATRIX_DIMENSIONS_ERROR;
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

  // ---------------------------------------------------------------------------

  Vec3 Matrix::operator*(const Vec3& in)
  {
    Vec3 res;

    if (not (_rows == 3 or _rows == 4) )
    {
      Error = EngineError::MATRIX_DIMENSIONS_ERROR;
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
        Error = EngineError::DIVISION_BY_ZERO;
      }
    }

    return res;
  }

  // ---------------------------------------------------------------------------

  Vec4 Matrix::operator*(const Vec4& in)
  {
    Vec4 res;

    if (_cols != 4)
    {
      Error = EngineError::MATRIX_DIMENSIONS_ERROR;
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

  // ---------------------------------------------------------------------------

  Matrix& Matrix::operator*=(double value)
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

  // ---------------------------------------------------------------------------

  Matrix Matrix::operator+(const Matrix& rhs)
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

  // ---------------------------------------------------------------------------

  void Matrix::operator=(const Matrix& rhs)
  {
    _matrix = rhs._matrix;
    _rows   = rhs._rows;
    _cols   = rhs._cols;
  }

  // ---------------------------------------------------------------------------

  const uint32_t& Matrix::Rows() const
  {
    return _rows;
  }

  const uint32_t& Matrix::Columns() const
  {
    return _cols;
  }
}
