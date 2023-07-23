#pragma once

#include "ink/math/vector.h"

namespace ink {

/// @brief
///   Column major 2x2 matrix.
struct Matrix2 {
    Vector2 column[2];

    /// @brief
    ///   Create a zero matrix.
    constexpr Matrix2() noexcept : column() {}

    /// @brief
    ///   Create a diagonal matrix with the specified values.
    ///
    /// @param v00
    ///   Value at position (0, 0).
    /// @param v11
    ///   Value at position (1, 1).
    constexpr Matrix2(float v00, float v11) noexcept : column{Vector2(v00, 0), Vector2(0, v11)} {}

    /// @brief
    ///   Create a diagonal matrix with the specified value.
    ///
    /// @param v
    ///   Value to be filled at the diagonal of this matrix.
    explicit constexpr Matrix2(float v) noexcept : column{Vector2(v, 0), Vector2(0, v)} {}

    /// @brief
    ///   Create a matrix with the specified values.
    /// @note
    ///   This is column major matrix.
    ///
    /// @param v00
    ///   Value at row 0 and column 0 of this matrix.
    /// @param v10
    ///   Value at row 1 and column 0 of this matrix.
    /// @param v01
    ///   Value at row 0 and column 1 of this matrix.
    /// @param v11
    ///   Value at row 1 and column 1 of this matrix.
    constexpr Matrix2(float v00, float v10, float v01, float v11) noexcept
        : column{Vector2(v00, v10), Vector2(v01, v11)} {}

    /// @brief
    ///   Create a matrix with the specified values.
    /// @note
    ///   This is column major matrix.
    ///
    /// @param c0
    ///   Values at column 0 of this matrix.
    /// @param c1
    ///   Values at column 1 of this matrix.
    constexpr Matrix2(Vector2 c0, Vector2 c1) noexcept : column{c0, c1} {}

    /// @brief
    ///   Random access columns of this matrix.
    ///
    /// @param i
    ///   Index of the column to be accessed.
    ///
    /// @return
    ///   Reference to the specified column of elements.
    constexpr auto operator[](std::size_t i) noexcept -> Vector2 & {
        return column[i];
    }

    /// @brief
    ///   Random access columns of this matrix.
    ///
    /// @param i
    ///   Index of the column to be accessed.
    ///
    /// @return
    ///   Reference to the specified column of elements.
    constexpr auto operator[](std::size_t i) const noexcept -> const Vector2 & {
        return column[i];
    }

    /// @brief
    ///   Calculate determinant of this matrix.
    ///
    /// @return
    ///   Determinant of this matrix.
    [[nodiscard]]
    constexpr auto determinant() const noexcept -> float {
        return column[0][0] * column[1][1] - column[0][1] * column[1][0];
    }

    /// @brief
    ///   Transpose this matrix.
    /// @remark
    ///   To get transposed matrix without modifying this matrix, use @p transposed() instead.
    ///
    /// @return
    ///   Reference to this matrix.
    constexpr auto transpose() noexcept -> Matrix2 & {
        float temp   = column[0][1];
        column[0][1] = column[1][0];
        column[1][0] = temp;
        return *this;
    }

    /// @brief
    ///   Get transposed matrix of this one.
    /// @remark
    ///   To transpose this matrix, use @p transpose() instead.
    ///
    /// @return
    ///   A new matrix that represents the transposed matrix.
    [[nodiscard]]
    constexpr auto transposed() const noexcept -> Matrix2 {
        return Matrix2(column[0][0], column[1][0], column[0][1], column[1][1]);
    }

    /// @brief
    ///   Inverse this matrix.
    /// @remark
    ///   To get inversed matrix without modifying this matrix, use @p inversed() instead.
    ///
    /// @return
    ///   Reference to this matrix.
    constexpr auto inverse() noexcept -> Matrix2 & {
        const float det    = this->determinant();
        const float invDet = 1.0f / det;

        const float v00 = column[1][1] * invDet;
        const float v11 = column[0][0] * invDet;

        column[0][1] *= -invDet;
        column[1][0] *= -invDet;
        column[0][0] = v00;
        column[1][1] = v11;

        return *this;
    }

    /// @brief
    ///   Get inversed matrix of this one.
    /// @remark
    ///   To inverse this matrix, use @p inverse() instead.
    ///
    /// @return
    ///   A new matrix that represents the inversed matrix.
    [[nodiscard]]
    constexpr auto inversed() const noexcept -> Matrix2 {
        const float det    = this->determinant();
        const float invDet = 1.0f / det;

        return Matrix2(column[1][1] * invDet, column[0][1] * -invDet, column[1][0] * -invDet,
                       column[0][0] * invDet);
    }

    constexpr auto operator+() const noexcept -> Matrix2 {
        return *this;
    }

    constexpr auto operator-() const noexcept -> Matrix2 {
        return Matrix2(-column[0], -column[1]);
    }

    constexpr auto operator+=(float rhs) noexcept -> Matrix2 & {
        column[0] += rhs;
        column[1] += rhs;
        return *this;
    }

    constexpr auto operator+=(const Matrix2 &rhs) noexcept -> Matrix2 & {
        column[0] += rhs[0];
        column[1] += rhs[1];
        return *this;
    }

    constexpr auto operator-=(float rhs) noexcept -> Matrix2 & {
        column[0] -= rhs;
        column[1] -= rhs;
        return *this;
    }

    constexpr auto operator-=(const Matrix2 &rhs) noexcept -> Matrix2 & {
        column[0] -= rhs[0];
        column[1] -= rhs[1];
        return *this;
    }

    constexpr auto operator*=(float rhs) noexcept -> Matrix2 & {
        column[0] *= rhs;
        column[1] *= rhs;
        return *this;
    }

    constexpr auto operator*=(const Matrix2 &rhs) noexcept -> Matrix2 & {
        float v00 = column[0][0] * rhs[0][0] + column[1][0] * rhs[0][1];
        float v01 = column[0][1] * rhs[0][0] + column[1][1] * rhs[0][1];
        float v10 = column[0][0] * rhs[1][0] + column[1][0] * rhs[1][1];
        float v11 = column[0][1] * rhs[1][0] + column[1][1] * rhs[1][1];

        column[0][0] = v00;
        column[0][1] = v01;
        column[1][0] = v10;
        column[1][1] = v11;

        return *this;
    }

    constexpr auto operator/=(float rhs) noexcept -> Matrix2 & {
        column[0] /= rhs;
        column[1] /= rhs;
        return *this;
    }

    constexpr auto operator/=(const Matrix2 &rhs) noexcept -> Matrix2 & {
        *this *= rhs.inversed();
        return *this;
    }
};

constexpr auto operator==(const Matrix2 &lhs, const Matrix2 &rhs) noexcept -> bool {
    return lhs[0] == rhs[0] && lhs[1] == rhs[1];
}

constexpr auto operator!=(const Matrix2 &lhs, const Matrix2 &rhs) noexcept -> bool {
    return lhs[0] != rhs[0] || lhs[1] != rhs[1];
}

constexpr auto operator+(const Matrix2 &lhs, const Matrix2 &rhs) noexcept -> Matrix2 {
    return Matrix2(lhs[0] + rhs[0], lhs[1] + rhs[1]);
}

constexpr auto operator+(const Matrix2 &lhs, float rhs) noexcept -> Matrix2 {
    return Matrix2(lhs[0] + rhs, lhs[1] + rhs);
}

constexpr auto operator+(float lhs, const Matrix2 &rhs) noexcept -> Matrix2 {
    return Matrix2(lhs + rhs[0], lhs + rhs[1]);
}

constexpr auto operator-(const Matrix2 &lhs, const Matrix2 &rhs) noexcept -> Matrix2 {
    return Matrix2(lhs[0] - rhs[0], lhs[1] - rhs[1]);
}

constexpr auto operator-(const Matrix2 &lhs, float rhs) noexcept -> Matrix2 {
    return Matrix2(lhs[0] - rhs, lhs[1] - rhs);
}

constexpr auto operator-(float lhs, const Matrix2 &rhs) noexcept -> Matrix2 {
    return Matrix2(lhs - rhs[0], lhs - rhs[1]);
}

constexpr auto operator*(const Matrix2 &lhs, const Matrix2 &rhs) noexcept -> Matrix2 {
    return Matrix2{
        lhs[0][0] * rhs[0][0] + lhs[1][0] * rhs[0][1],
        lhs[0][1] * rhs[0][0] + lhs[1][1] * rhs[0][1],
        lhs[0][0] * rhs[1][0] + lhs[1][0] * rhs[1][1],
        lhs[0][1] * rhs[1][0] + lhs[1][1] * rhs[1][1],
    };
}

constexpr auto operator*(const Matrix2 &lhs, float rhs) noexcept -> Matrix2 {
    return Matrix2(lhs[0] * rhs, lhs[1] * rhs);
}

constexpr auto operator*(float lhs, const Matrix2 &rhs) noexcept -> Matrix2 {
    return Matrix2(lhs * rhs[0], lhs * rhs[1]);
}

constexpr auto operator*(const Matrix2 &lhs, Vector2 rhs) noexcept -> Vector2 {
    return Vector2(lhs[0][0] * rhs.x + lhs[1][0] * rhs.y, lhs[0][1] * rhs.x + lhs[1][1] * rhs.y);
}

constexpr auto operator*(Vector2 lhs, const Matrix2 &rhs) noexcept -> Vector2 {
    return Vector2(lhs.x * rhs[0][0] + lhs.y * rhs[0][1], lhs.x * rhs[1][0] + lhs.y * rhs[1][1]);
}

constexpr auto operator*=(Vector2 &lhs, const Matrix2 &rhs) noexcept -> Vector2 & {
    lhs = (lhs * rhs);
    return lhs;
}

constexpr auto operator/(const Matrix2 &lhs, const Matrix2 &rhs) noexcept -> Matrix2 {
    return lhs * rhs.inversed();
}

constexpr auto operator/(const Matrix2 &lhs, float rhs) noexcept -> Matrix2 {
    return Matrix2(lhs[0] / rhs, lhs[1] / rhs);
}

constexpr auto operator/(float lhs, const Matrix2 &rhs) noexcept -> Matrix2 {
    return Matrix2(lhs / rhs[0], lhs / rhs[1]);
}

constexpr auto operator/(Vector2 lhs, const Matrix2 &rhs) noexcept -> Vector2 {
    return lhs * rhs.inversed();
}

constexpr auto operator/=(Vector2 &lhs, const Matrix2 &rhs) noexcept -> Vector2 & {
    lhs = (lhs / rhs);
    return lhs;
}

/// @brief
///   Column major 3x3 matrix.
struct Matrix3 {
    Vector3 column[3];

    /// @brief
    ///   Create a zero matrix.
    constexpr Matrix3() noexcept : column() {}

    /// @brief
    ///   Create a diagonal matrix with the specified values.
    ///
    /// @param v00
    ///   Value at position (0, 0).
    /// @param v11
    ///   Value at position (1, 1).
    /// @param v22
    ///   Value at position (2, 2).
    constexpr Matrix3(float v00, float v11, float v22) noexcept
        : column{Vector3(v00, 0, 0), Vector3(0, v11, 0), Vector3(0, 0, v22)} {}

    /// @brief
    ///   Create a diagonal matrix with the specified value.
    ///
    /// @param v
    ///   Value to be filled at the diagonal of this matrix.
    explicit constexpr Matrix3(float v) noexcept
        : column{Vector3(v, 0, 0), Vector3(0, v, 0), Vector3(0, 0, v)} {}

    // clang-format off
    /// @brief
    ///   Create a matrix with the specified values.
    /// @note
    ///   This is column major matrix.
    ///
    /// @param v00
    ///   Value at row 0 and column 0 of this matrix.
    /// @param v10
    ///   Value at row 1 and column 0 of this matrix.
    /// @param v20
    ///   Value at row 2 and column 0 of this matrix.
    /// @param v01
    ///   Value at row 0 and column 1 of this matrix.
    /// @param v11
    ///   Value at row 1 and column 1 of this matrix.
    /// @param v21
    ///   Value at row 2 and column 1 of this matrix.
    /// @param v02
    ///   Value at row 0 and column 2 of this matrix.
    /// @param v12
    ///   Value at row 1 and column 2 of this matrix.
    /// @param v22
    ///   Value at row 2 and column 2 of this matrix.
    constexpr Matrix3(float v00, float v10, float v20,
                      float v01, float v11, float v21,
                      float v02, float v12, float v22) noexcept
        : column{Vector3(v00, v10, v20), Vector3(v01, v11, v21), Vector3(v02, v12, v22)} {}
    // clang-format on

    /// @brief
    ///   Create a matrix with the specified values.
    /// @note
    ///   This is column major matrix.
    ///
    /// @param c0
    ///   Values at column 0 of this matrix.
    /// @param c1
    ///   Values at column 1 of this matrix.
    /// @param c2
    ///   Values at column 2 of this matrix.
    constexpr Matrix3(Vector3 c0, Vector3 c1, Vector3 c2) noexcept : column{c0, c1, c2} {}

    /// @brief
    ///   Random access columns of this matrix.
    ///
    /// @param i
    ///   Index of the column to be accessed.
    ///
    /// @return
    ///   Reference to the specified column of elements.
    constexpr auto operator[](std::size_t i) noexcept -> Vector3 & {
        return column[i];
    }

    /// @brief
    ///   Random access columns of this matrix.
    ///
    /// @param i
    ///   Index of the column to be accessed.
    ///
    /// @return
    ///   Reference to the specified column of elements.
    constexpr auto operator[](std::size_t i) const noexcept -> const Vector3 & {
        return column[i];
    }

    /// @brief
    ///   Calculate determinant of this matrix.
    ///
    /// @return
    ///   Determinant of this matrix.
    [[nodiscard]]
    constexpr auto determinant() const noexcept -> float {
        return column[0][0] * (column[1][1] * column[2][2] - column[1][2] * column[2][1]) -
               column[0][1] * (column[1][0] * column[2][2] - column[1][2] * column[2][0]) +
               column[0][2] * (column[1][0] * column[2][1] - column[1][1] * column[2][0]);
    }

    /// @brief
    ///   Transpose this matrix.
    /// @remark
    ///   To get transposed matrix without modifying this matrix, use @p transposed() instead.
    ///
    /// @return
    ///   Reference to this matrix.
    constexpr auto transpose() noexcept -> Matrix3 & {
        float temp   = column[0][1];
        column[0][1] = column[1][0];
        column[1][0] = temp;

        temp         = column[0][2];
        column[0][2] = column[2][0];
        column[2][0] = temp;

        temp         = column[1][2];
        column[1][2] = column[2][1];
        column[2][1] = temp;
        return *this;
    }

    /// @brief
    ///   Get transposed matrix of this one.
    /// @remark
    ///   To transpose this matrix, use @p transpose() instead.
    ///
    /// @return
    ///   A new matrix that represents the transposed matrix.
    [[nodiscard]]
    constexpr auto transposed() const noexcept -> Matrix3 {
        return Matrix3(column[0][0], column[1][0], column[2][0], column[0][1], column[1][1],
                       column[2][1], column[0][2], column[1][2], column[2][2]);
    }

    /// @brief
    ///   Inverse this matrix.
    /// @remark
    ///   To get inversed matrix without modifying this matrix, use @p inversed() instead.
    ///
    /// @return
    ///   Reference to this matrix.
    constexpr auto inverse() noexcept -> Matrix3 & {
        const Matrix3 temp{
            column[1][1] * column[2][2] - column[2][1] * column[1][2],
            column[2][1] * column[0][2] - column[0][1] * column[2][2],
            column[0][1] * column[1][2] - column[1][1] * column[0][2],
            column[2][0] * column[1][2] - column[1][0] * column[2][2],
            column[0][0] * column[2][2] - column[0][2] * column[2][0],
            column[1][0] * column[0][2] - column[0][0] * column[1][2],
            column[1][0] * column[2][1] - column[2][0] * column[1][1],
            column[2][0] * column[0][1] - column[0][0] * column[2][1],
            column[0][0] * column[1][1] - column[1][0] * column[0][1],
        };

        const float invDet = 1.0f / this->determinant();

        column[0] = temp[0] * invDet;
        column[1] = temp[1] * invDet;
        column[2] = temp[2] * invDet;

        return *this;
    }

    /// @brief
    ///   Get inversed matrix of this one.
    /// @remark
    ///   To inverse this matrix, use @p inverse() instead.
    ///
    /// @return
    ///   A new matrix that represents the transposed matrix.
    [[nodiscard]]
    constexpr auto inversed() const noexcept -> Matrix3 {
        const float invDet = 1.0f / this->determinant();
        return Matrix3{
            invDet * (column[1][1] * column[2][2] - column[2][1] * column[1][2]),
            invDet * (column[2][1] * column[0][2] - column[0][1] * column[2][2]),
            invDet * (column[0][1] * column[1][2] - column[1][1] * column[0][2]),
            invDet * (column[2][0] * column[1][2] - column[1][0] * column[2][2]),
            invDet * (column[0][0] * column[2][2] - column[0][2] * column[2][0]),
            invDet * (column[1][0] * column[0][2] - column[0][0] * column[1][2]),
            invDet * (column[1][0] * column[2][1] - column[2][0] * column[1][1]),
            invDet * (column[2][0] * column[0][1] - column[0][0] * column[2][1]),
            invDet * (column[0][0] * column[1][1] - column[1][0] * column[0][1]),
        };
    }

    /// @brief
    ///   Apply a 2D translate transform to this matrix.
    /// @remark
    ///   To get translated matrix without modifying this matrix, use @p translated() instead.
    /// @note
    ///   It is assumed that this is already a valid 2D transform matrix.
    ///
    /// @param x
    ///   Offset along x axis.
    /// @param y
    ///   Offset along y axis.
    ///
    /// @return
    ///   Reference to this matrix.
    constexpr auto translate(float x, float y) noexcept -> Matrix3 & {
        column[0][2] += x;
        column[1][2] += y;
        return *this;
    }

    /// @brief
    ///   Get 2D translated matrix of this one.
    /// @remark
    ///   To apply translate transform to this matrix, use @p translate() instead.
    /// @note
    ///   It is assumed that this is already a valid 2D transform matrix.
    ///
    /// @param x
    ///   Offset along x axis.
    /// @param y
    ///   Offset along y axis.
    ///
    /// @return
    ///   The translated matrix of this one.
    [[nodiscard]]
    constexpr auto translated(float x, float y) noexcept -> Matrix3 {
        return Matrix3(column[0][0], column[0][1], column[0][2] + x, column[1][0], column[1][1],
                       column[1][2] + y, column[2][0], column[2][1], column[2][2]);
    }

    /// @brief
    ///   Apply a 2D translate transform to this matrix.
    /// @remark
    ///   To get translated matrix without modifying this matrix, use @p translated() instead.
    /// @note
    ///   It is assumed that this is already a valid 2D transform matrix.
    ///
    /// @param offset
    ///   Translate offset to be applied.
    ///
    /// @return
    ///   Reference to this matrix.
    constexpr auto translate(Vector2 offset) noexcept -> Matrix3 & {
        column[0][2] += offset.x;
        column[1][2] += offset.y;
        return *this;
    }

    /// @brief
    ///   Get 2D translated matrix of this one.
    /// @remark
    ///   To apply translate transform to this matrix, use @p translate() instead.
    /// @note
    ///   It is assumed that this is already a valid 2D transform matrix.
    ///
    /// @param offset
    ///   Translate offset to be applied.
    ///
    /// @return
    ///   The translated matrix of this one.
    constexpr auto translated(Vector2 offset) noexcept -> Matrix3 {
        return Matrix3(column[0][0], column[0][1], column[0][2] + offset.x, column[1][0],
                       column[1][1], column[1][2] + offset.y, column[2][0], column[2][1],
                       column[2][2]);
    }

    /// @brief
    ///   Apply 2D rotate transform to this matrix.
    /// @remark
    ///   To get rotated matrix without modifying this matrix, use @p rotated() instead.
    /// @note
    ///   It is assumed that this is already a valid 2D transform matrix.
    ///
    /// @param radian
    ///   Radian to rotate.
    ///
    /// @return
    ///   Reference to this matrix.
    auto rotate(float radian) noexcept -> Matrix3 & {
        const float s = std::sin(radian);
        const float c = std::cos(radian);

        const Vector3 c0 = column[0] * c - column[1] * s;
        const Vector3 c1 = column[0] * s + column[1] * c;

        column[0] = c0;
        column[1] = c1;

        return *this;
    }

    /// @brief
    ///   Get 2D rotated matrix of this one.
    /// @remark
    ///   To apply rotate transform to this matrix, use @p rotate() instead.
    /// @note
    ///   It is assumed that this is already a valid 2D transform matrix.
    ///
    /// @param radian
    ///   Radian to rotate.
    ///
    /// @return
    ///   Rotated matrix of this one.
    [[nodiscard]]
    auto rotated(float radian) const noexcept -> Matrix3 {
        const float s = std::sin(radian);
        const float c = std::cos(radian);

        return Matrix3(column[0] * c - column[1] * s, column[0] * s + column[1] * c, column[2]);
    }

    /// @brief
    ///   Apply 2D scale transform to this matrix.
    /// @remark
    ///   To get scaled matrix without modifying this matrix, use @p scaled() instead.
    /// @note
    ///   It is assumed that this is already a valid 2D transform matrix.
    ///
    /// @param x
    ///   Scale factor along x axis.
    /// @param y
    ///   Scale factor along y axis.
    ///
    /// @return
    ///   Reference to this matrix.
    constexpr auto scale(float x, float y) noexcept -> Matrix3 & {
        column[0] *= x;
        column[1] *= y;
        return *this;
    }

    /// @brief
    ///   Get 2D scaled matrix of this one.
    /// @remark
    ///   To apply scale transform to this matrix, use @p scale() instead.
    ///
    /// @param x
    ///   Scale factor along x axis.
    /// @param y
    ///   Scale factor along y axis.
    ///
    /// @return
    ///   The scaled matrix.
    [[nodiscard]]
    constexpr auto scaled(float x, float y) const noexcept -> Matrix3 {
        return Matrix3(column[0] * x, column[1] * y, column[2]);
    }

    /// @brief
    ///   Apply 2D scale transform to this matrix.
    /// @remark
    ///   To get scaled matrix without modifying this matrix, use @p scaled() instead.
    /// @note
    ///   It is assumed that this is already a valid 2D transform matrix.
    ///
    /// @param factor
    ///   Scale factors.
    ///
    /// @return
    ///   Reference to this matrix.
    constexpr auto scale(Vector2 factor) noexcept -> Matrix3 & {
        column[0] *= factor.x;
        column[1] *= factor.y;
        return *this;
    }

    /// @brief
    ///   Get 2D scaled matrix of this one.
    /// @remark
    ///   To apply scale transform to this matrix, use @p scale() instead.
    /// @note
    ///   It is assumed that this is already a valid 2D transform matrix.
    ///
    /// @param factor
    ///   Scale factors.
    ///
    /// @return
    ///   The scaled matrix.
    [[nodiscard]]
    constexpr auto scaled(Vector2 factor) const noexcept -> Matrix3 {
        return Matrix3(column[0] * factor.x, column[1] * factor.y, column[2]);
    }

    /// @brief
    ///   Apply a 2D shear transform along X axis.
    /// @remark
    ///   To get sheared matrix without modifying this matrix, use @p shearedX() instead.
    /// @note
    ///   It is assumed that this is already a valid 2D transform matrix.
    ///
    /// @param factor
    ///   Shear factor along X axis.
    ///
    /// @return
    ///   Reference to this matrix.
    constexpr auto shearX(float factor) noexcept -> Matrix3 & {
        column[0] += factor * column[1];
        return *this;
    }

    /// @brief
    ///   Get 2D sheared matrix along X axis.
    /// @remark
    ///   To apply shear transform to this matrix, use @p shearX() instead.
    /// @note
    ///   It is assumed that this is already a valid 2D transform matrix.
    ///
    /// @param factor
    ///   Shear factor along X axis.
    ///
    /// @return
    ///   Sheared matrix along X axis.
    [[nodiscard]]
    constexpr auto shearedX(float factor) const noexcept -> Matrix3 {
        return Matrix3(column[0] + factor * column[1], column[1], column[2]);
    }

    /// @brief
    ///   Apply a 2D shear transform along Y axis.
    /// @remark
    ///   To get sheared matrix without modifying this matrix, use @p shearedY() instead.
    /// @note
    ///   It is assumed that this is already a valid 2D transform matrix.
    ///
    /// @param factor
    ///   Shear factor along Y axis.
    ///
    /// @return
    ///   Reference to this matrix.
    constexpr auto shearY(float factor) noexcept -> Matrix3 & {
        column[1] += factor * column[0];
        return *this;
    }

    /// @brief
    ///   Get 2D sheared matrix along Y axis.
    /// @remark
    ///   To apply shear transform to this matrix, use @p shearY() instead.
    /// @note
    ///   It is assumed that this is already a valid 2D transform matrix.
    ///
    /// @param factor
    ///   Shear factor along Y axis.
    ///
    /// @return
    ///   Sheared matrix along Y axis.
    [[nodiscard]]
    constexpr auto shearedY(float factor) const noexcept -> Matrix3 {
        return Matrix3(column[0], column[1] + factor * column[0], column[2]);
    }

    constexpr auto operator+() const noexcept -> Matrix3 {
        return *this;
    }

    constexpr auto operator-() const noexcept -> Matrix3 {
        return Matrix3(-column[0], -column[1], -column[2]);
    }

    constexpr auto operator+=(float rhs) noexcept -> Matrix3 & {
        column[0] += rhs;
        column[1] += rhs;
        column[2] += rhs;
        return *this;
    }

    constexpr auto operator+=(const Matrix3 &rhs) noexcept -> Matrix3 & {
        column[0] += rhs[0];
        column[1] += rhs[1];
        column[2] += rhs[2];
        return *this;
    }

    constexpr auto operator-=(float rhs) noexcept -> Matrix3 & {
        column[0] -= rhs;
        column[1] -= rhs;
        column[2] -= rhs;
        return *this;
    }

    constexpr auto operator-=(const Matrix3 &rhs) noexcept -> Matrix3 & {
        column[0] -= rhs[0];
        column[1] -= rhs[1];
        column[2] -= rhs[2];
        return *this;
    }

    constexpr auto operator*=(float rhs) noexcept -> Matrix3 & {
        column[0] *= rhs;
        column[1] *= rhs;
        column[2] *= rhs;
        return *this;
    }

    constexpr auto operator*=(const Matrix3 &rhs) noexcept -> Matrix3 & {
        // clang-format off
        const float v00 = column[0][0] * rhs[0][0] + column[1][0] * rhs[0][1] + column[2][0] * rhs[0][2];
        const float v01 = column[0][1] * rhs[0][0] + column[1][1] * rhs[0][1] + column[2][1] * rhs[0][2];
        const float v02 = column[0][2] * rhs[0][0] + column[1][2] * rhs[0][1] + column[2][2] * rhs[0][2];
        const float v10 = column[0][0] * rhs[1][0] + column[1][0] * rhs[1][1] + column[2][0] * rhs[1][2];
        const float v11 = column[0][1] * rhs[1][0] + column[1][1] * rhs[1][1] + column[2][1] * rhs[1][2];
        const float v12 = column[0][2] * rhs[1][0] + column[1][2] * rhs[1][1] + column[2][2] * rhs[1][2];
        const float v20 = column[0][0] * rhs[2][0] + column[1][0] * rhs[2][1] + column[2][0] * rhs[2][2];
        const float v21 = column[0][1] * rhs[2][0] + column[1][1] * rhs[2][1] + column[2][1] * rhs[2][2];
        const float v22 = column[0][2] * rhs[2][0] + column[1][2] * rhs[2][1] + column[2][2] * rhs[2][2];
        // clang-format on

        column[0][0] = v00;
        column[0][1] = v01;
        column[0][2] = v02;
        column[1][0] = v10;
        column[1][1] = v11;
        column[1][2] = v12;
        column[2][0] = v20;
        column[2][1] = v21;
        column[2][2] = v22;

        return *this;
    }

    constexpr auto operator/=(float rhs) noexcept -> Matrix3 & {
        column[0] /= rhs;
        column[1] /= rhs;
        column[2] /= rhs;
        return *this;
    }

    constexpr auto operator/=(const Matrix3 &rhs) noexcept -> Matrix3 & {
        *this *= rhs.inversed();
        return *this;
    }
};

constexpr auto operator==(const Matrix3 &lhs, const Matrix3 &rhs) noexcept -> bool {
    return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2];
}

constexpr auto operator!=(const Matrix3 &lhs, const Matrix3 &rhs) noexcept -> bool {
    return lhs[0] != rhs[0] || lhs[1] != rhs[1] || lhs[2] != rhs[2];
}

constexpr auto operator+(const Matrix3 &lhs, const Matrix3 &rhs) noexcept -> Matrix3 {
    return Matrix3(lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2]);
}

constexpr auto operator+(const Matrix3 &lhs, float rhs) noexcept -> Matrix3 {
    return Matrix3(lhs[0] + rhs, lhs[1] + rhs, lhs[2] + rhs);
}

constexpr auto operator+(float lhs, const Matrix3 &rhs) noexcept -> Matrix3 {
    return Matrix3(lhs + rhs[0], lhs + rhs[1], lhs + rhs[2]);
}

constexpr auto operator-(const Matrix3 &lhs, const Matrix3 &rhs) noexcept -> Matrix3 {
    return Matrix3(lhs[0] - rhs[0], lhs[1] - rhs[1], lhs[2] - rhs[2]);
}

constexpr auto operator-(const Matrix3 &lhs, float rhs) noexcept -> Matrix3 {
    return Matrix3(lhs[0] - rhs, lhs[1] - rhs, lhs[2] - rhs);
}

constexpr auto operator-(float lhs, const Matrix3 &rhs) noexcept -> Matrix3 {
    return Matrix3(lhs - rhs[0], lhs - rhs[1], lhs - rhs[2]);
}

constexpr auto operator*(const Matrix3 &lhs, const Matrix3 &rhs) noexcept -> Matrix3 {
    return Matrix3{
        lhs[0][0] * rhs[0][0] + lhs[1][0] * rhs[0][1] + lhs[2][0] * rhs[0][2],
        lhs[0][1] * rhs[0][0] + lhs[1][1] * rhs[0][1] + lhs[2][1] * rhs[0][2],
        lhs[0][2] * rhs[0][0] + lhs[1][2] * rhs[0][1] + lhs[2][2] * rhs[0][2],
        lhs[0][0] * rhs[1][0] + lhs[1][0] * rhs[1][1] + lhs[2][0] * rhs[1][2],
        lhs[0][1] * rhs[1][0] + lhs[1][1] * rhs[1][1] + lhs[2][1] * rhs[1][2],
        lhs[0][2] * rhs[1][0] + lhs[1][2] * rhs[1][1] + lhs[2][2] * rhs[1][2],
        lhs[0][0] * rhs[2][0] + lhs[1][0] * rhs[2][1] + lhs[2][0] * rhs[2][2],
        lhs[0][1] * rhs[2][0] + lhs[1][1] * rhs[2][1] + lhs[2][1] * rhs[2][2],
        lhs[0][2] * rhs[2][0] + lhs[1][2] * rhs[2][1] + lhs[2][2] * rhs[2][2],
    };
}

constexpr auto operator*(const Matrix3 &lhs, float rhs) noexcept -> Matrix3 {
    return Matrix3(lhs[0] * rhs, lhs[1] * rhs, lhs[2] * rhs);
}

constexpr auto operator*(float lhs, const Matrix3 &rhs) noexcept -> Matrix3 {
    return Matrix3(lhs * rhs[0], lhs * rhs[1], lhs * rhs[2]);
}

constexpr auto operator*(const Matrix3 &lhs, Vector3 rhs) noexcept -> Vector3 {
    return Vector3{
        lhs[0][0] * rhs.x + lhs[1][0] * rhs.y + lhs[2][0] * rhs.z,
        lhs[0][1] * rhs.x + lhs[1][1] * rhs.y + lhs[2][1] * rhs.z,
        lhs[0][2] * rhs.x + lhs[1][2] * rhs.y + lhs[2][2] * rhs.z,
    };
}

constexpr auto operator*(Vector3 lhs, const Matrix3 &rhs) noexcept -> Vector3 {
    return Vector3{
        lhs.x * rhs[0][0] + lhs.y * rhs[0][1] + lhs.z * rhs[0][2],
        lhs.x * rhs[1][0] + lhs.y * rhs[1][1] + lhs.z * rhs[1][2],
        lhs.x * rhs[2][0] + lhs.y * rhs[2][1] + lhs.z * rhs[2][2],
    };
}

constexpr auto operator*=(Vector3 &lhs, const Matrix3 &rhs) noexcept -> Vector3 & {
    lhs = (lhs * rhs);
    return lhs;
}

constexpr auto operator/(const Matrix3 &lhs, const Matrix3 &rhs) noexcept -> Matrix3 {
    return lhs * rhs.inversed();
}

constexpr auto operator/(const Matrix3 &lhs, float rhs) noexcept -> Matrix3 {
    return Matrix3(lhs[0] / rhs, lhs[1] / rhs, lhs[2] / rhs);
}

constexpr auto operator/(float lhs, const Matrix3 &rhs) noexcept -> Matrix3 {
    return Matrix3(lhs / rhs[0], lhs / rhs[1], lhs / rhs[2]);
}

constexpr auto operator/(Vector3 lhs, const Matrix3 &rhs) noexcept -> Vector3 {
    return lhs * rhs.inversed();
}

constexpr auto operator/=(Vector3 &lhs, const Matrix3 &rhs) noexcept -> Vector3 & {
    lhs = (lhs / rhs);
    return lhs;
}

struct Quaternion;

/// @brief
///   Column major 4x4 matrix.
struct Matrix4 {
    Vector4 column[4];

    /// @brief
    ///   Create a zero matrix.
    constexpr Matrix4() noexcept : column() {}

    /// @brief
    ///   Create a diagonal matrix with the specified values.
    ///
    /// @param v00
    ///   Value at position (0, 0).
    /// @param v11
    ///   Value at position (1, 1).
    /// @param v22
    ///   Value at position (2, 2).
    /// @param v33
    ///   Value at position (3, 3).
    constexpr Matrix4(float v00, float v11, float v22, float v33) noexcept
        : column{Vector4(v00, 0.0f, 0.0f, 0.0f), Vector4(0.0f, v11, 0.0f, 0.0f),
                 Vector4(0.0f, 0.0f, v22, 0.0f), Vector4(0.0f, 0.0f, 0.0f, v33)} {}

    /// @brief
    ///   Create a diagonal matrix with the specified value.
    ///
    /// @param v
    ///   Value to be filled at the diagonal of this matrix.
    explicit constexpr Matrix4(float v) noexcept
        : column{Vector4(v, 0.0f, 0.0f, 0.0f), Vector4(0.0f, v, 0.0f, 0.0f),
                 Vector4(0.0f, 0.0f, v, 0.0f), Vector4(0.0f, 0.0f, 0.0f, v)} {}

    // clang-format off
    /// @brief
    ///   Create a matrix with the specified values.
    /// 
    /// @param v00
    ///   Value at row 0 and column 0 of this matrix.
    /// @param v10
    ///   Value at row 1 and column 0 of this matrix.
    /// @param v20
    ///   Value at row 2 and column 0 of this matrix.
    /// @param v30
    ///   Value at row 3 and column 0 of this matrix.
    /// @param v01
    ///   Value at row 0 and column 1 of this matrix.
    /// @param v11
    ///   Value at row 1 and column 1 of this matrix.
    /// @param v21
    ///   Value at row 2 and column 1 of this matrix.
    /// @param v31
    ///   Value at row 3 and column 1 of this matrix.
    /// @param v02
    ///   Value at row 0 and column 2 of this matrix.
    /// @param v12
    ///   Value at row 1 and column 2 of this matrix.
    /// @param v22
    ///   Value at row 2 and column 2 of this matrix.
    /// @param v32
    ///   Value at row 3 and column 2 of this matrix.
    /// @param v03
    ///   Value at row 0 and column 3 of this matrix.
    /// @param v13
    ///   Value at row 1 and column 3 of this matrix.
    /// @param v23
    ///   Value at row 2 and column 3 of this matrix.
    /// @param v33
    ///   Value at row 3 and column 3 of this matrix.
    constexpr Matrix4(float v00, float v10, float v20, float v30,
                      float v01, float v11, float v21, float v31,
                      float v02, float v12, float v22, float v32,
                      float v03, float v13, float v23, float v33) noexcept
        : column{{v00, v10, v20, v30}, {v01, v11, v21, v31},
                 {v02, v12, v22, v32}, {v03, v13, v23, v33}} {}
    // clang-format on

    /// @brief
    ///   Create a matrix with the specified values.
    /// @note
    ///   This is column major matrix.
    ///
    /// @param c0
    ///   Values at column 0 of this matrix.
    /// @param c1
    ///   Values at column 1 of this matrix.
    /// @param c2
    ///   Values at column 2 of this matrix.
    /// @param c3
    ///   Values at column 3 of this matrix.
    constexpr Matrix4(Vector4 c0, Vector4 c1, Vector4 c2, Vector4 c3) noexcept
        : column{c0, c1, c2, c3} {}

    /// @brief
    ///   Random access columns of this matrix.
    ///
    /// @param i
    ///   Index of the column to be accessed.
    ///
    /// @return
    ///   Reference to the specified column of elements.
    constexpr auto operator[](std::size_t i) noexcept -> Vector4 & {
        return column[i];
    }

    /// @brief
    ///   Random access columns of this matrix.
    ///
    /// @param i
    ///   Index of the column to be accessed.
    ///
    /// @return
    ///   Reference to the specified column of elements.
    constexpr auto operator[](std::size_t i) const noexcept -> const Vector4 & {
        return column[i];
    }

    /// @brief
    ///   Calculate determinant of this matrix.
    ///
    /// @return
    ///   Determinant of this matrix.
    [[nodiscard]]
    constexpr auto determinant() const noexcept -> float {
        const float t00 = column[2][2] * column[3][3] - column[3][2] * column[2][3];
        const float t01 = column[2][1] * column[3][3] - column[3][1] * column[2][3];
        const float t02 = column[2][1] * column[3][2] - column[3][1] * column[2][2];
        const float t03 = column[2][0] * column[3][3] - column[3][0] * column[2][3];
        const float t04 = column[2][0] * column[3][2] - column[3][0] * column[2][2];
        const float t05 = column[2][0] * column[3][1] - column[3][0] * column[2][1];

        const float cof0 = column[1][1] * t00 - column[1][2] * t01 + column[1][3] * t02;
        const float cof1 = column[1][0] * t00 - column[1][2] * t03 + column[1][3] * t04;
        const float cof2 = column[1][0] * t01 - column[1][1] * t03 + column[1][3] * t05;
        const float cof3 = column[1][0] * t02 - column[1][1] * t04 + column[1][2] * t05;

        return column[0][0] * cof0 - column[0][1] * cof1 + column[0][2] * cof2 -
               column[0][3] * cof3;
    }

    /// @brief
    ///   Transpose this matrix.
    /// @remark
    ///   To get transposed matrix without modifying this matrix, use @p transposed() instead.
    ///
    /// @return
    ///   Reference to this matrix.
    constexpr auto transpose() noexcept -> Matrix4 & {
        float temp   = column[0][1];
        column[0][1] = column[1][0];
        column[1][0] = temp;

        temp         = column[0][2];
        column[0][2] = column[2][0];
        column[2][0] = temp;

        temp         = column[0][3];
        column[0][3] = column[3][0];
        column[3][0] = temp;

        temp         = column[1][2];
        column[1][2] = column[2][1];
        column[2][1] = temp;

        temp         = column[1][3];
        column[1][3] = column[3][1];
        column[3][1] = temp;

        temp         = column[2][3];
        column[2][3] = column[3][2];
        column[3][2] = temp;

        return *this;
    }

    /// @brief
    ///   Get transposed matrix of this one.
    /// @remark
    ///   To transpose this matrix, use @p transpose() instead.
    ///
    /// @return
    ///   A new matrix that represents the transposed matrix.
    [[nodiscard]]
    constexpr auto transposed() const noexcept -> Matrix4 {
        // clang-format off
        return Matrix4(
            column[0][0], column[1][0], column[2][0], column[3][0],
            column[0][1], column[1][1], column[2][1], column[3][1],
            column[0][2], column[1][2], column[2][2], column[3][2],
            column[0][3], column[1][3], column[2][3], column[3][3]
        );
        // clang-format on
    }

    /// @brief
    ///   Inverse this matrix.
    /// @remark
    ///   To get inversed matrix without modifying this matrix, use @p inversed() instead.
    ///
    /// @return
    ///   Reference to this matrix.
    constexpr auto inverse() noexcept -> Matrix4 & {
        const float coef00 = column[2][2] * column[3][3] - column[3][2] * column[2][3];
        const float coef02 = column[1][2] * column[3][3] - column[3][2] * column[1][3];
        const float coef03 = column[1][2] * column[2][3] - column[2][2] * column[1][3];
        const float coef04 = column[2][1] * column[3][3] - column[3][1] * column[2][3];
        const float coef06 = column[1][1] * column[3][3] - column[3][1] * column[1][3];
        const float coef07 = column[1][1] * column[2][3] - column[2][1] * column[1][3];
        const float coef08 = column[2][1] * column[3][2] - column[3][1] * column[2][2];
        const float coef10 = column[1][1] * column[3][2] - column[3][1] * column[1][2];
        const float coef11 = column[1][1] * column[2][2] - column[2][1] * column[1][2];
        const float coef12 = column[2][0] * column[3][3] - column[3][0] * column[2][3];
        const float coef14 = column[1][0] * column[3][3] - column[3][0] * column[1][3];
        const float coef15 = column[1][0] * column[2][3] - column[2][0] * column[1][3];
        const float coef16 = column[2][0] * column[3][2] - column[3][0] * column[2][2];
        const float coef18 = column[1][0] * column[3][2] - column[3][0] * column[1][2];
        const float coef19 = column[1][0] * column[2][2] - column[2][0] * column[1][2];
        const float coef20 = column[2][0] * column[3][1] - column[3][0] * column[2][1];
        const float coef22 = column[1][0] * column[3][1] - column[3][0] * column[1][1];
        const float coef23 = column[1][0] * column[2][1] - column[2][0] * column[1][1];

        const Vector4 fac0(coef00, coef00, coef02, coef03);
        const Vector4 fac1(coef04, coef04, coef06, coef07);
        const Vector4 fac2(coef08, coef08, coef10, coef11);
        const Vector4 fac3(coef12, coef12, coef14, coef15);
        const Vector4 fac4(coef16, coef16, coef18, coef19);
        const Vector4 fac5(coef20, coef20, coef22, coef23);

        const Vector4 v0(column[1][0], column[0][0], column[0][0], column[0][0]);
        const Vector4 v1(column[1][1], column[0][1], column[0][1], column[0][1]);
        const Vector4 v2(column[1][2], column[0][2], column[0][2], column[0][2]);
        const Vector4 v3(column[1][3], column[0][3], column[0][3], column[0][3]);

        const Vector4 inv0(v1 * fac0 - v2 * fac1 + v3 * fac2);
        const Vector4 inv1(v0 * fac0 - v2 * fac3 + v3 * fac4);
        const Vector4 inv2(v0 * fac1 - v1 * fac3 + v3 * fac5);
        const Vector4 inv3(v0 * fac2 - v1 * fac4 + v2 * fac5);

        const Vector4 sgn0(1.0f, -1.0f, 1.0f, -1.0f);
        const Vector4 sgn1(-1.0f, 1.0f, -1.0f, 1.0f);

        const Matrix4 result(inv0 * sgn0, inv1 * sgn1, inv2 * sgn0, inv3 * sgn1);
        const Vector4 r0(result[0][0], result[1][0], result[2][0], result[3][0]);
        const Vector4 dot0(column[0] * r0);

        const float dot1   = (dot0.x + dot0.y) + (dot0.z + dot0.w);
        const float invDet = 1.0f / dot1;

        column[0] = result[0] * invDet;
        column[1] = result[1] * invDet;
        column[2] = result[2] * invDet;
        column[3] = result[3] * invDet;

        return *this;
    }

    /// @brief
    ///   Get inversed matrix of this one.
    /// @remark
    ///   To inverse this matrix, use @p inverse() instead.
    ///
    /// @return
    ///   A new matrix that represents the transposed matrix.
    [[nodiscard]]
    constexpr auto inversed() const noexcept -> Matrix4 {
        const float coef00 = column[2][2] * column[3][3] - column[3][2] * column[2][3];
        const float coef02 = column[1][2] * column[3][3] - column[3][2] * column[1][3];
        const float coef03 = column[1][2] * column[2][3] - column[2][2] * column[1][3];
        const float coef04 = column[2][1] * column[3][3] - column[3][1] * column[2][3];
        const float coef06 = column[1][1] * column[3][3] - column[3][1] * column[1][3];
        const float coef07 = column[1][1] * column[2][3] - column[2][1] * column[1][3];
        const float coef08 = column[2][1] * column[3][2] - column[3][1] * column[2][2];
        const float coef10 = column[1][1] * column[3][2] - column[3][1] * column[1][2];
        const float coef11 = column[1][1] * column[2][2] - column[2][1] * column[1][2];
        const float coef12 = column[2][0] * column[3][3] - column[3][0] * column[2][3];
        const float coef14 = column[1][0] * column[3][3] - column[3][0] * column[1][3];
        const float coef15 = column[1][0] * column[2][3] - column[2][0] * column[1][3];
        const float coef16 = column[2][0] * column[3][2] - column[3][0] * column[2][2];
        const float coef18 = column[1][0] * column[3][2] - column[3][0] * column[1][2];
        const float coef19 = column[1][0] * column[2][2] - column[2][0] * column[1][2];
        const float coef20 = column[2][0] * column[3][1] - column[3][0] * column[2][1];
        const float coef22 = column[1][0] * column[3][1] - column[3][0] * column[1][1];
        const float coef23 = column[1][0] * column[2][1] - column[2][0] * column[1][1];

        const Vector4 fac0(coef00, coef00, coef02, coef03);
        const Vector4 fac1(coef04, coef04, coef06, coef07);
        const Vector4 fac2(coef08, coef08, coef10, coef11);
        const Vector4 fac3(coef12, coef12, coef14, coef15);
        const Vector4 fac4(coef16, coef16, coef18, coef19);
        const Vector4 fac5(coef20, coef20, coef22, coef23);

        const Vector4 v0(column[1][0], column[0][0], column[0][0], column[0][0]);
        const Vector4 v1(column[1][1], column[0][1], column[0][1], column[0][1]);
        const Vector4 v2(column[1][2], column[0][2], column[0][2], column[0][2]);
        const Vector4 v3(column[1][3], column[0][3], column[0][3], column[0][3]);

        const Vector4 inv0(v1 * fac0 - v2 * fac1 + v3 * fac2);
        const Vector4 inv1(v0 * fac0 - v2 * fac3 + v3 * fac4);
        const Vector4 inv2(v0 * fac1 - v1 * fac3 + v3 * fac5);
        const Vector4 inv3(v0 * fac2 - v1 * fac4 + v2 * fac5);

        const Vector4 sgn0(1.0f, -1.0f, 1.0f, -1.0f);
        const Vector4 sgn1(-1.0f, 1.0f, -1.0f, 1.0f);

        const Matrix4 result(inv0 * sgn0, inv1 * sgn1, inv2 * sgn0, inv3 * sgn1);
        const Vector4 r0(result[0][0], result[1][0], result[2][0], result[3][0]);
        const Vector4 dot0(column[0] * r0);

        const float dot1   = (dot0.x + dot0.y) + (dot0.z + dot0.w);
        const float invDet = 1.0f / dot1;

        return Matrix4(result[0] * invDet, result[1] * invDet, result[2] * invDet,
                       result[3] * invDet);
    }

    /// @brief
    ///   Apply a 3D translate transform to this matrix.
    /// @remark
    ///   To get translated matrix without modifying this matrix, use @p translated() instead.
    /// @note
    ///   It is assumed that this is already a valid 3D transform matrix.
    ///
    /// @param x
    ///   Offset along x axis.
    /// @param y
    ///   Offset along y axis.
    /// @param z
    ///   Offset along z axis.
    ///
    /// @return
    ///   Reference to this matrix.
    constexpr auto translate(float x, float y, float z) noexcept -> Matrix4 & {
        column[0][3] += x;
        column[1][3] += y;
        column[2][3] += z;
        return *this;
    }

    /// @brief
    ///   Get 3D translated matrix of this one.
    /// @remark
    ///   To apply translate transform to this matrix, use @p translate() instead.
    /// @note
    ///   It is assumed that this is already a valid 3D transform matrix.
    ///
    /// @param x
    ///   Offset along x axis.
    /// @param y
    ///   Offset along y axis.
    /// @param z
    ///   Offset along z axis.
    ///
    /// @return
    ///   The translated matrix of this one.
    [[nodiscard]]
    constexpr auto translated(float x, float y, float z) const noexcept -> Matrix4 {
        // clang-format off
        return Matrix4{
            column[0][0], column[0][1], column[0][2], column[0][3] + x,
            column[1][0], column[1][1], column[1][2], column[1][3] + y,
            column[2][0], column[2][1], column[2][2], column[2][3] + z,
            column[3][0], column[3][1], column[3][2], column[3][3],
        };
        // clang-format on
    }

    /// @brief
    ///   Apply a 3D translate transform to this matrix.
    /// @remark
    ///   To get translated matrix without modifying this matrix, use @p translated() instead.
    /// @note
    ///   It is assumed that this is already a valid 3D transform matrix.
    ///
    /// @param offset
    ///   Translate offset to be applied.
    ///
    /// @return
    ///   Reference to this matrix.
    constexpr auto translate(Vector3 offset) noexcept -> Matrix4 & {
        column[0][3] += offset.x;
        column[1][3] += offset.y;
        column[2][3] += offset.z;
        return *this;
    }

    /// @brief
    ///   Get 3D translated matrix of this one.
    /// @remark
    ///   To apply translate transform to this matrix, use @p translate() instead.
    /// @note
    ///   It is assumed that this is already a valid 3D transform matrix.
    ///
    /// @param offset
    ///   Translate offset to be applied.
    ///
    /// @return
    ///   The translated matrix of this one.
    [[nodiscard]]
    constexpr auto translated(Vector3 offset) const noexcept -> Matrix4 {
        // clang-format off
        return Matrix4{
            column[0][0], column[0][1], column[0][2], column[0][3] + offset.x,
            column[1][0], column[1][1], column[1][2], column[1][3] + offset.y,
            column[2][0], column[2][1], column[2][2], column[2][3] + offset.z,
            column[3][0], column[3][1], column[3][2], column[3][3],
        };
        // clang-format on
    }

    /// @brief
    ///   Apply a 3D translate transform to this matrix.
    /// @remark
    ///   To get translated matrix without modifying this matrix, use @p translated() instead.
    /// @note
    ///   It is assumed that this is already a valid 3D transform matrix.
    ///
    /// @param offset
    ///   Translate offset to be applied. w is ignored.
    ///
    /// @return
    ///   Reference to this matrix.
    constexpr auto translate(Vector4 offset) noexcept -> Matrix4 & {
        column[0][3] += offset.x;
        column[1][3] += offset.y;
        column[2][3] += offset.z;
        return *this;
    }

    /// @brief
    ///   Get 3D translated matrix of this one.
    /// @remark
    ///   To apply translate transform to this matrix, use @p translate() instead.
    /// @note
    ///   It is assumed that this is already a valid 3D transform matrix.
    ///
    /// @param offset
    ///   Translate offset to be applied. w is ignored.
    ///
    /// @return
    ///   The translated matrix of this one.
    [[nodiscard]]
    constexpr auto translated(Vector4 offset) const noexcept -> Matrix4 {
        // clang-format off
        return Matrix4{
            column[0][0], column[0][1], column[0][2], column[0][3] + offset.x,
            column[1][0], column[1][1], column[1][2], column[1][3] + offset.y,
            column[2][0], column[2][1], column[2][2], column[2][3] + offset.z,
            column[3][0], column[3][1], column[3][2], column[3][3],
        };
        // clang-format on
    }

    /// @brief
    ///   Apply 3D rotate transform to this matrix.
    /// @remark
    ///   To get rotated matrix without modifying this matrix, use @p rotated() instead.
    /// @note
    ///   It is assumed that this is already a valid 3D transform matrix.
    ///
    /// @param axis
    ///   The axis to be rotated along to.
    /// @param radian
    ///   Radian to rotate.
    ///
    /// @return
    ///   Reference to this matrix.
    auto rotate(Vector3 axis, float radian) noexcept -> Matrix4 & {
        const float s = std::sin(radian);
        const float c = std::cos(radian);

        axis.normalize();
        const Vector3 temp = (1.0f - c) * axis;

        const float r00 = temp.x * axis.x + c;
        const float r01 = temp.y * axis.x - s * axis.z;
        const float r02 = temp.z * axis.x + s * axis.y;
        const float r10 = temp.x * axis.y + s * axis.z;
        const float r11 = temp.y * axis.y + c;
        const float r12 = temp.z * axis.y - s * axis.x;
        const float r20 = temp.x * axis.z - s * axis.y;
        const float r21 = temp.y * axis.z + s * axis.x;
        const float r22 = temp.z * axis.z + c;

        const Vector4 c0 = column[0] * r00 + column[1] * r01 + column[2] * r02;
        const Vector4 c1 = column[0] * r10 + column[1] * r11 + column[2] * r12;
        const Vector4 c2 = column[0] * r20 + column[1] * r21 + column[2] * r22;

        column[0] = c0;
        column[1] = c1;
        column[2] = c2;

        return *this;
    }

    /// @brief
    ///   Get 3D rotated matrix of this one.
    /// @remark
    ///   To apply rotate transform to this matrix, use @p rotate() instead.
    /// @note
    ///   It is assumed that this is already a valid 2D transform matrix.
    ///
    /// @param axis
    ///   The axis to be rotated along to.
    /// @param radian
    ///   Radian to rotate.
    ///
    /// @return
    ///   Rotated matrix of this one.
    [[nodiscard]]
    auto rotated(Vector3 axis, float radian) const noexcept -> Matrix4 {
        const float s = std::sin(radian);
        const float c = std::cos(radian);

        axis.normalize();
        const Vector3 temp = (1.0f - c) * axis;

        const float r00 = temp.x * axis.x + c;
        const float r01 = temp.y * axis.x - s * axis.z;
        const float r02 = temp.z * axis.x + s * axis.y;
        const float r10 = temp.x * axis.y + s * axis.z;
        const float r11 = temp.y * axis.y + c;
        const float r12 = temp.z * axis.y - s * axis.x;
        const float r20 = temp.x * axis.z - s * axis.y;
        const float r21 = temp.y * axis.z + s * axis.x;
        const float r22 = temp.z * axis.z + c;

        return Matrix4{
            column[0] * r00 + column[1] * r01 + column[2] * r02,
            column[0] * r10 + column[1] * r11 + column[2] * r12,
            column[0] * r20 + column[1] * r21 + column[2] * r22,
            column[3],
        };
    }

    /// @brief
    ///   Apply 3D rotate transform to this matrix.
    /// @remark
    ///   To get rotated matrix without modifying this matrix, use @p rotated() instead.
    /// @note
    ///   It is assumed that this is already a valid 3D transform matrix.
    ///
    /// @param quat
    ///   The quaternion that represents the rotation to be applied.
    ///
    /// @return
    ///   Reference to this matrix.
    constexpr auto rotate(Quaternion quat) noexcept -> Matrix4 &;

    /// @brief
    ///   Get 3D rotated matrix from this one.
    /// @remark
    ///   To apply rotate transform to this matrix, use @p rotate() instead.
    /// @note
    ///   It is assumed that this is already a valid 2D transform matrix.
    ///
    /// @param quat
    ///   The quaternion that represents the rotation to be applied.
    ///
    /// @return
    ///   Rotated matrix from this one.
    [[nodiscard]]
    constexpr auto rotated(Quaternion quat) const noexcept -> Matrix4;

    /// @brief
    ///   Apply 3D scale transform to this matrix.
    /// @remark
    ///   To get scaled matrix without modifying this matrix, use @p scaled() instead.
    /// @note
    ///   It is assumed that this is already a valid 3D transform matrix.
    ///
    /// @param x
    ///   Scale factor along x axis.
    /// @param y
    ///   Scale factor along y axis.
    /// @param z
    ///   Scale factor along z axis.
    ///
    /// @return
    ///   Reference to this matrix.
    constexpr auto scale(float x, float y, float z) noexcept -> Matrix4 & {
        column[0] *= x;
        column[1] *= y;
        column[2] *= z;
        return *this;
    }

    /// @brief
    ///   Get 3D scaled matrix of this one.
    /// @remark
    ///   To apply scale transform to this matrix, use @p scale() instead.
    /// @note
    ///   It is assumed that this is already a valid 3D transform matrix.
    ///
    /// @param x
    ///   Scale factor along x axis.
    /// @param y
    ///   Scale factor along y axis.
    /// @param z
    ///   Scale factor along z axis.
    ///
    /// @return
    ///   The scaled matrix.
    [[nodiscard]]
    constexpr auto scaled(float x, float y, float z) const noexcept -> Matrix4 {
        return Matrix4(column[0] * x, column[1] * y, column[2] * z, column[3]);
    }

    /// @brief
    ///   Apply 3D scale transform to this matrix.
    /// @remark
    ///   To get scaled matrix without modifying this matrix, use @p scaled() instead.
    /// @note
    ///   It is assumed that this is already a valid 3D transform matrix.
    ///
    /// @param factor
    ///   Scale factors.
    ///
    /// @return
    ///   Reference to this matrix.
    constexpr auto scale(Vector3 factor) noexcept -> Matrix4 & {
        column[0] *= factor.x;
        column[1] *= factor.y;
        column[2] *= factor.z;
        return *this;
    }

    /// @brief
    ///   Get 3D scaled matrix of this one.
    /// @remark
    ///   To apply scale transform to this matrix, use @p scale() instead.
    /// @note
    ///   It is assumed that this is already a valid 3D transform matrix.
    ///
    /// @param factor
    ///   Scale factors.
    ///
    /// @return
    ///   The scaled matrix.
    [[nodiscard]]
    constexpr auto scaled(Vector3 factor) const noexcept -> Matrix4 {
        return Matrix4(column[0] * factor.x, column[1] * factor.y, column[2] * factor.z, column[3]);
    }

    constexpr auto operator+() const noexcept -> Matrix4 {
        return *this;
    }

    constexpr auto operator-() const noexcept -> Matrix4 {
        return Matrix4(-column[0], -column[1], -column[2], -column[3]);
    }

    constexpr auto operator+=(float rhs) noexcept -> Matrix4 & {
        column[0] += rhs;
        column[1] += rhs;
        column[2] += rhs;
        column[3] += rhs;
        return *this;
    }

    constexpr auto operator+=(const Matrix4 &rhs) noexcept -> Matrix4 & {
        column[0] += rhs[0];
        column[1] += rhs[1];
        column[2] += rhs[2];
        column[3] += rhs[3];
        return *this;
    }

    constexpr auto operator-=(float rhs) noexcept -> Matrix4 & {
        column[0] -= rhs;
        column[1] -= rhs;
        column[2] -= rhs;
        column[3] -= rhs;
        return *this;
    }

    constexpr auto operator-=(const Matrix4 &rhs) noexcept -> Matrix4 & {
        column[0] -= rhs[0];
        column[1] -= rhs[1];
        column[2] -= rhs[2];
        column[3] -= rhs[3];
        return *this;
    }

    constexpr auto operator*=(float rhs) noexcept -> Matrix4 & {
        column[0] *= rhs;
        column[1] *= rhs;
        column[2] *= rhs;
        column[3] *= rhs;
        return *this;
    }

    constexpr auto operator*=(const Matrix4 &rhs) noexcept -> Matrix4 & {
        // clang-format off
        const float v00 = column[0][0] * rhs[0][0] + column[1][0] * rhs[0][1] + column[2][0] * rhs[0][2] + column[3][0] * rhs[0][3];
        const float v01 = column[0][1] * rhs[0][0] + column[1][1] * rhs[0][1] + column[2][1] * rhs[0][2] + column[3][1] * rhs[0][3];
        const float v02 = column[0][2] * rhs[0][0] + column[1][2] * rhs[0][1] + column[2][2] * rhs[0][2] + column[3][2] * rhs[0][3];
        const float v03 = column[0][3] * rhs[0][0] + column[1][3] * rhs[0][1] + column[2][3] * rhs[0][2] + column[3][3] * rhs[0][3];
        const float v10 = column[0][0] * rhs[1][0] + column[1][0] * rhs[1][1] + column[2][0] * rhs[1][2] + column[3][0] * rhs[1][3];
        const float v11 = column[0][1] * rhs[1][0] + column[1][1] * rhs[1][1] + column[2][1] * rhs[1][2] + column[3][1] * rhs[1][3];
        const float v12 = column[0][2] * rhs[1][0] + column[1][2] * rhs[1][1] + column[2][2] * rhs[1][2] + column[3][2] * rhs[1][3];
        const float v13 = column[0][3] * rhs[1][0] + column[1][3] * rhs[1][1] + column[2][3] * rhs[1][2] + column[3][3] * rhs[1][3];
        const float v20 = column[0][0] * rhs[2][0] + column[1][0] * rhs[2][1] + column[2][0] * rhs[2][2] + column[3][0] * rhs[2][3];
        const float v21 = column[0][1] * rhs[2][0] + column[1][1] * rhs[2][1] + column[2][1] * rhs[2][2] + column[3][1] * rhs[2][3];
        const float v22 = column[0][2] * rhs[2][0] + column[1][2] * rhs[2][1] + column[2][2] * rhs[2][2] + column[3][2] * rhs[2][3];
        const float v23 = column[0][3] * rhs[2][0] + column[1][3] * rhs[2][1] + column[2][3] * rhs[2][2] + column[3][3] * rhs[2][3];
        const float v30 = column[0][0] * rhs[3][0] + column[1][0] * rhs[3][1] + column[2][0] * rhs[3][2] + column[3][0] * rhs[3][3];
        const float v31 = column[0][1] * rhs[3][0] + column[1][1] * rhs[3][1] + column[2][1] * rhs[3][2] + column[3][1] * rhs[3][3];
        const float v32 = column[0][2] * rhs[3][0] + column[1][2] * rhs[3][1] + column[2][2] * rhs[3][2] + column[3][2] * rhs[3][3];
        const float v33 = column[0][3] * rhs[3][0] + column[1][3] * rhs[3][1] + column[2][3] * rhs[3][2] + column[3][3] * rhs[3][3];
        // clang-format on

        column[0] = Vector4(v00, v01, v02, v03);
        column[1] = Vector4(v10, v11, v12, v13);
        column[2] = Vector4(v20, v21, v22, v23);
        column[3] = Vector4(v30, v31, v32, v33);

        return *this;
    }

    constexpr auto operator/=(float rhs) noexcept -> Matrix4 & {
        column[0] /= rhs;
        column[1] /= rhs;
        column[2] /= rhs;
        column[3] /= rhs;
        return *this;
    }

    constexpr auto operator/=(const Matrix4 &rhs) noexcept -> Matrix4 & {
        *this *= rhs.inversed();
        return *this;
    }
};

constexpr auto operator==(const Matrix4 &lhs, const Matrix4 &rhs) noexcept -> bool {
    return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2] && lhs[3] == rhs[3];
}

constexpr auto operator!=(const Matrix4 &lhs, const Matrix4 &rhs) noexcept -> bool {
    return lhs[0] != rhs[0] || lhs[1] != rhs[1] || lhs[2] != rhs[2] || lhs[3] != rhs[3];
}

constexpr auto operator+(const Matrix4 &lhs, const Matrix4 &rhs) noexcept -> Matrix4 {
    return Matrix4(lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2], lhs[3] + rhs[3]);
}

constexpr auto operator+(const Matrix4 &lhs, float rhs) noexcept -> Matrix4 {
    return Matrix4(lhs[0] + rhs, lhs[1] + rhs, lhs[2] + rhs, lhs[3] + rhs);
}

constexpr auto operator+(float lhs, const Matrix4 &rhs) noexcept -> Matrix4 {
    return Matrix4(lhs + rhs[0], lhs + rhs[1], lhs + rhs[2], lhs + rhs[3]);
}

constexpr auto operator-(const Matrix4 &lhs, const Matrix4 &rhs) noexcept -> Matrix4 {
    return Matrix4(lhs[0] - rhs[0], lhs[1] - rhs[1], lhs[2] - rhs[2], lhs[3] - rhs[3]);
}

constexpr auto operator-(const Matrix4 &lhs, float rhs) noexcept -> Matrix4 {
    return Matrix4(lhs[0] - rhs, lhs[1] - rhs, lhs[2] - rhs, lhs[3] - rhs);
}

constexpr auto operator-(float lhs, const Matrix4 &rhs) noexcept -> Matrix4 {
    return Matrix4(lhs - rhs[0], lhs - rhs[1], lhs - rhs[2], lhs - rhs[3]);
}

constexpr auto operator*(const Matrix4 &lhs, const Matrix4 &rhs) noexcept -> Matrix4 {
    // clang-format off
    return Matrix4{
        lhs[0][0] * rhs[0][0] + lhs[1][0] * rhs[0][1] + lhs[2][0] * rhs[0][2] + lhs[3][0] * rhs[0][3],
        lhs[0][1] * rhs[0][0] + lhs[1][1] * rhs[0][1] + lhs[2][1] * rhs[0][2] + lhs[3][1] * rhs[0][3],
        lhs[0][2] * rhs[0][0] + lhs[1][2] * rhs[0][1] + lhs[2][2] * rhs[0][2] + lhs[3][2] * rhs[0][3],
        lhs[0][3] * rhs[0][0] + lhs[1][3] * rhs[0][1] + lhs[2][3] * rhs[0][2] + lhs[3][3] * rhs[0][3],
        lhs[0][0] * rhs[1][0] + lhs[1][0] * rhs[1][1] + lhs[2][0] * rhs[1][2] + lhs[3][0] * rhs[1][3],
        lhs[0][1] * rhs[1][0] + lhs[1][1] * rhs[1][1] + lhs[2][1] * rhs[1][2] + lhs[3][1] * rhs[1][3],
        lhs[0][2] * rhs[1][0] + lhs[1][2] * rhs[1][1] + lhs[2][2] * rhs[1][2] + lhs[3][2] * rhs[1][3],
        lhs[0][3] * rhs[1][0] + lhs[1][3] * rhs[1][1] + lhs[2][3] * rhs[1][2] + lhs[3][3] * rhs[1][3],
        lhs[0][0] * rhs[2][0] + lhs[1][0] * rhs[2][1] + lhs[2][0] * rhs[2][2] + lhs[3][0] * rhs[2][3],
        lhs[0][1] * rhs[2][0] + lhs[1][1] * rhs[2][1] + lhs[2][1] * rhs[2][2] + lhs[3][1] * rhs[2][3],
        lhs[0][2] * rhs[2][0] + lhs[1][2] * rhs[2][1] + lhs[2][2] * rhs[2][2] + lhs[3][2] * rhs[2][3],
        lhs[0][3] * rhs[2][0] + lhs[1][3] * rhs[2][1] + lhs[2][3] * rhs[2][2] + lhs[3][3] * rhs[2][3],
        lhs[0][0] * rhs[3][0] + lhs[1][0] * rhs[3][1] + lhs[2][0] * rhs[3][2] + lhs[3][0] * rhs[3][3],
        lhs[0][1] * rhs[3][0] + lhs[1][1] * rhs[3][1] + lhs[2][1] * rhs[3][2] + lhs[3][1] * rhs[3][3],
        lhs[0][2] * rhs[3][0] + lhs[1][2] * rhs[3][1] + lhs[2][2] * rhs[3][2] + lhs[3][2] * rhs[3][3],
        lhs[0][3] * rhs[3][0] + lhs[1][3] * rhs[3][1] + lhs[2][3] * rhs[3][2] + lhs[3][3] * rhs[3][3],
    };
    // clang-format on
}

constexpr auto operator*(const Matrix4 &lhs, float rhs) noexcept -> Matrix4 {
    return Matrix4(lhs[0] * rhs, lhs[1] * rhs, lhs[2] * rhs, lhs[3] * rhs);
}

constexpr auto operator*(float lhs, const Matrix4 &rhs) noexcept -> Matrix4 {
    return Matrix4(lhs * rhs[0], lhs * rhs[1], lhs * rhs[2], lhs * rhs[3]);
}

constexpr auto operator*(const Matrix4 &lhs, Vector4 rhs) noexcept -> Vector4 {
    return Vector4{
        lhs[0][0] * rhs.x + lhs[1][0] * rhs.y + lhs[2][0] * rhs.z + lhs[3][0] * rhs.w,
        lhs[0][1] * rhs.x + lhs[1][1] * rhs.y + lhs[2][1] * rhs.z + lhs[3][1] * rhs.w,
        lhs[0][2] * rhs.x + lhs[1][2] * rhs.y + lhs[2][2] * rhs.z + lhs[3][2] * rhs.w,
        lhs[0][3] * rhs.x + lhs[1][3] * rhs.y + lhs[2][3] * rhs.z + lhs[3][3] * rhs.w,
    };
}

constexpr auto operator*(Vector4 lhs, const Matrix4 &rhs) noexcept -> Vector4 {
    return Vector4{
        lhs.x * rhs[0][0] + lhs.y * rhs[0][1] + lhs.z * rhs[0][2] + lhs.w * rhs[0][3],
        lhs.x * rhs[1][0] + lhs.y * rhs[1][1] + lhs.z * rhs[1][2] + lhs.w * rhs[1][3],
        lhs.x * rhs[2][0] + lhs.y * rhs[2][1] + lhs.z * rhs[2][2] + lhs.w * rhs[2][3],
        lhs.x * rhs[3][0] + lhs.y * rhs[3][1] + lhs.z * rhs[3][2] + lhs.w * rhs[3][3],
    };
}

constexpr auto operator*=(Vector4 &lhs, const Matrix4 &rhs) noexcept -> Vector4 & {
    lhs = (lhs * rhs);
    return lhs;
}

constexpr auto operator/(const Matrix4 &lhs, const Matrix4 &rhs) noexcept -> Matrix4 {
    return lhs * rhs.inversed();
}

constexpr auto operator/(const Matrix4 &lhs, float rhs) noexcept -> Matrix4 {
    return Matrix4(lhs[0] / rhs, lhs[1] / rhs, lhs[2] / rhs, lhs[3] / rhs);
}

constexpr auto operator/(float lhs, const Matrix4 &rhs) noexcept -> Matrix4 {
    return Matrix4(lhs / rhs[0], lhs / rhs[1], lhs / rhs[2], lhs / rhs[3]);
}

constexpr auto operator/(Vector4 lhs, const Matrix4 &rhs) noexcept -> Vector4 {
    return lhs * rhs.inversed();
}

constexpr auto operator/=(Vector4 &lhs, const Matrix4 &rhs) noexcept -> Vector4 & {
    lhs = (lhs / rhs);
    return lhs;
}

/// @brief
///   Create a look at transform matrix. This is equivalent to lookTo(eye, target - eye, up).
///
/// @param eye
///   Eye position.
/// @param target
///   The target to look at. Must differ from @p eye.
/// @param up
///   Camera up direction.
///
/// @return
///   A 4x4 matrix that represents the look at transform.
[[nodiscard]]
inline auto lookAt(Vector3 eye, Vector3 target, Vector3 up) noexcept -> Matrix4 {
    const Vector3 front = (target - eye).normalized();
    const Vector3 right = cross(up, front).normalized();
    const Vector3 upDir = cross(front, right);

    return Matrix4{
        Vector4{right.x, right.y, right.z, -dot(right, eye)},
        Vector4{upDir.x, upDir.y, upDir.z, -dot(upDir, eye)},
        Vector4{front.x, front.y, front.z, -dot(front, eye)},
        Vector4{0, 0, 0, 1.0f},
    };
}

/// @brief
///   Create a look at transform matrix. This is equivalent to lookAt(eye, eye + direction, up).
/// @note
///   Left-hand coordinate is used.
///
/// @param eye
///   Eye position.
/// @param direction
///   The direction to look to. Zero vector is not accepted.
/// @param up
///   Camera up direction.
///
/// @return
///   A 4x4 matrix that represents the look at transform.
[[nodiscard]]
inline auto lookTo(Vector3 eye, Vector3 direction, Vector3 up) noexcept -> Matrix4 {
    const Vector3 front = direction.normalized();
    const Vector3 right = cross(up, front).normalized();
    const Vector3 upDir = cross(front, right);

    return Matrix4{
        Vector4{right.x, right.y, right.z, -dot(right, eye)},
        Vector4{upDir.x, upDir.y, upDir.z, -dot(upDir, eye)},
        Vector4{front.x, front.y, front.z, -dot(front, eye)},
        Vector4{0, 0, 0, 1.0f},
    };
}

/// @brief
///   Create a perspective transform matrix.
/// @note
///   Left-hand coordinate is used.
///
/// @param fovY
///   Field of view in radian according Y axis.
/// @param aspect
///   Screen aspect ratio. Generally this value is equivalent to width / height.
/// @param zNear
///   Near camera plane distance. For left-hand coordinate, this should be positive and less than @p
///   zFar.
/// @param zFar
///   Far camera plane distance. For left-hand coordinate, this should be positive and greater than
///   @p zNear.
///
/// @return
///   A 4x4 matrix that represents the perspective transform.
[[nodiscard]]
inline auto perspective(float fovY, float aspect, float zNear, float zFar) noexcept -> Matrix4 {
    const float t = 1.0f / std::tan(fovY / 2.0f);
    const float z = zFar / (zFar - zNear);

    return Matrix4{
        Vector4{aspect * t, 0.0f, 0.0f, 0.0f},
        Vector4{0.0f, t, 0.0f, 0.0f},
        Vector4{0.0f, 0.0f, z, -zNear * z},
        Vector4{0.0f, 0.0f, 1.0f, 0.0f},
    };
}

/// @brief
///   Create a perspective transform matrix.
/// @note
///   Left-hand coordinate is used.
///
/// @param fovY
///   Field of view in radian according Y axis.
/// @param width
///   Screen width. This is used to calculate aspect ratio.
/// @param height
///   Screen height. This is used to calculate aspect ratio.
/// @param zNear
///   Near camera plane distance. For left-hand coordinate, this should be positive and less than @p
///   zFar.
/// @param zFar
///   Far camera plane distance. For left-hand coordinate, this should be positive and greater than
///   @p zNear.
///
/// @return
///   A 4x4 matrix that represents the perspective transform.
[[nodiscard]]
inline auto perspective(float fovY, float width, float height, float zNear, float zFar) noexcept
    -> Matrix4 {
    const float a = width / height;
    const float t = 1.0f / std::tan(fovY / 2.0f);
    const float z = zFar / (zFar - zNear);

    return Matrix4{
        Vector4{a * t, 0.0f, 0.0f, 0.0f},
        Vector4{0.0f, t, 0.0f, 0.0f},
        Vector4{0.0f, 0.0f, z, -zNear * z},
        Vector4{0.0f, 0.0f, 1.0f, 0.0f},
    };
}

/// @brief
///   Create an orthographic transform matrix.
/// @note
///   Left-hand coordinate is used.
///
/// @param left
///   Left side of the orthographic cube. Should be smaller than @p right.
/// @param right
///   Right side of the orthographic cube. Should be greater than @p left.
/// @param bottom
///   Bottom side of the orthographic cube. Should be smaller than @p top.
/// @param top
///   Top side of the orthographic cube. Should be greater than @p bottom.
/// @param zNear
///   Near plane of the orthographic cube. Should be smaller than @p zFar.
/// @param zFar
///   Far plane of the orthographic cube. Should be greater than @p zNear.
///
/// @return
///   A 4x4 matrix that represents the orthographic transform.
[[nodiscard]]
constexpr auto
orthographic(float left, float right, float bottom, float top, float zNear, float zFar) noexcept
    -> Matrix4 {
    const float lr = 1.0f / (right - left);
    const float bt = 1.0f / (top - bottom);
    const float nf = 1.0f / (zFar - zNear);

    return Matrix4{
        Vector4{2.0f * lr, 0.0f, 0.0f, -(left + right) * lr},
        Vector4{0.0f, 2.0f * bt, 0.0f, -(top + bottom) * bt},
        Vector4{0.0f, 0.0f, 1.0f * nf, -zNear * nf},
        Vector4{0.0f, 0.0f, 0.0f, 1.0f},
    };
}

} // namespace ink
