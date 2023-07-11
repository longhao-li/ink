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

        column[0][0] *= invDet;
        column[0][1] *= -invDet;
        column[1][0] *= -invDet;
        column[1][1] *= invDet;

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

        return Matrix2(column[0][0] * invDet, column[0][1] * -invDet, column[1][0] * -invDet,
                       column[1][1] * invDet);
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

} // namespace ink
