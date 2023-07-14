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
    ///   A new matrix that represents the transposed matrix.
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

} // namespace ink
