#pragma once

#include <cmath>

namespace ink {

struct Vector2 {
#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wpedantic"
#elif defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4201)
#endif
    union {
        float m_arr[2];
        struct {
            float u;
            float v;
        };
        struct {
            float x;
            float y;
        };
    };
#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#    pragma warning(pop)
#endif

    /// @brief
    ///   Create a 2D vector and initialize all elements to 0.
    constexpr Vector2() noexcept : m_arr{0, 0} {}

    /// @brief
    ///   Create a 2D vector and initialize all elements to the specified value.
    ///
    /// @param v
    ///   Value to be set to elements of this vector.
    explicit constexpr Vector2(float v) noexcept : m_arr{v, v} {}

    /// @brief
    ///   Create a 2D vector and initialize elements.
    ///
    /// @param x
    ///   Value of the first element of this vector.
    /// @param y
    ///   Value of the second element of this vector.
    constexpr Vector2(float x, float y) noexcept : m_arr{x, y} {}

    /// @brief
    ///   Create a 2D vector and initialize elements.
    ///
    /// @param arr
    ///   A float array that contains at least 2 elements.
    explicit constexpr Vector2(const float arr[2]) noexcept : m_arr{arr[0], arr[1]} {}

    /// @brief
    ///   Random access elements in this vector by index.
    /// @note
    ///   No boundary check performed.
    ///
    /// @param i
    ///   Index of the element to be accessed.
    ///
    /// @return
    ///   Reference to the specified element.
    constexpr auto operator[](std::size_t i) noexcept -> float & {
        return m_arr[i];
    }

    /// @brief
    ///   Random access elements in this vector by index.
    /// @note
    ///   No boundary check performed.
    ///
    /// @param i
    ///   Index of the element to be accessed.
    ///
    /// @return
    ///   Reference to the specified element.
    constexpr auto operator[](std::size_t i) const noexcept -> const float & {
        return m_arr[i];
    }

    /// @brief
    ///   Calculate of this 2D vector.
    /// @note
    ///   This method requires a square root operation which is time-consuming.
    ///
    /// @return
    ///   Length of this 2D vector.
    [[nodiscard]]
    auto length() const noexcept -> float {
        return std::sqrt(x * x + y * y);
    }

    /// @brief
    ///   Normalize this vector.
    /// @remark
    ///   To get normalized vector of this one without modifying this vector, use @p
    ///   Vector2::normalized() instead.
    ///
    /// @return
    ///   Reference to this vector.
    auto normalize() noexcept -> Vector2 & {
        const float len    = length();
        const float invLen = 1.0f / len;

        x *= invLen;
        y *= invLen;

        return *this;
    }

    /// @brief
    ///   Get normalized vector of this one.
    /// @note
    ///   This vector itself is not modified. To normalize this vector itself, use @p
    ///   Vector2::normalize() instead.
    ///
    /// @return
    ///   Normalized version of this vector.
    [[nodiscard]]
    auto normalized() const noexcept -> Vector2 {
        const float len    = length();
        const float invLen = 1.0f / len;

        return Vector2(x * invLen, y * invLen);
    }

    constexpr auto operator+=(float rhs) noexcept -> Vector2 & {
        x += rhs;
        y += rhs;
        return *this;
    }

    constexpr auto operator+=(Vector2 rhs) noexcept -> Vector2 & {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    constexpr auto operator-=(float rhs) noexcept -> Vector2 & {
        x -= rhs;
        y -= rhs;
        return *this;
    }

    constexpr auto operator-=(Vector2 rhs) noexcept -> Vector2 & {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    constexpr auto operator*=(float rhs) noexcept -> Vector2 & {
        x *= rhs;
        y *= rhs;
        return *this;
    }

    constexpr auto operator*=(Vector2 rhs) noexcept -> Vector2 & {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }

    constexpr auto operator/=(float rhs) noexcept -> Vector2 & {
        x /= rhs;
        y /= rhs;
        return *this;
    }

    constexpr auto operator/=(Vector2 rhs) noexcept -> Vector2 & {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }
};

constexpr auto operator+(Vector2 vec) noexcept -> Vector2 {
    return vec;
}

constexpr auto operator-(Vector2 vec) noexcept -> Vector2 {
    return Vector2(-vec.x, -vec.y);
}

constexpr auto operator==(Vector2 lhs, Vector2 rhs) noexcept -> bool {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

constexpr auto operator!=(Vector2 lhs, Vector2 rhs) noexcept -> bool {
    return lhs.x != rhs.x || lhs.y != rhs.y;
}

constexpr auto operator+(Vector2 lhs, Vector2 rhs) noexcept -> Vector2 {
    return Vector2(lhs.x + rhs.x, lhs.y + rhs.y);
}

constexpr auto operator+(Vector2 lhs, float rhs) noexcept -> Vector2 {
    return Vector2(lhs.x + rhs, lhs.y + rhs);
}

constexpr auto operator+(float lhs, Vector2 rhs) noexcept -> Vector2 {
    return Vector2(lhs + rhs.x, lhs + rhs.y);
}

constexpr auto operator-(Vector2 lhs, Vector2 rhs) noexcept -> Vector2 {
    return Vector2(lhs.x - rhs.x, lhs.y - rhs.y);
}

constexpr auto operator-(Vector2 lhs, float rhs) noexcept -> Vector2 {
    return Vector2(lhs.x - rhs, lhs.y - rhs);
}

constexpr auto operator-(float lhs, Vector2 rhs) noexcept -> Vector2 {
    return Vector2(lhs - rhs.x, lhs - rhs.y);
}

constexpr auto operator*(Vector2 lhs, Vector2 rhs) noexcept -> Vector2 {
    return Vector2(lhs.x * rhs.x, lhs.y * rhs.y);
}

constexpr auto operator*(Vector2 lhs, float rhs) noexcept -> Vector2 {
    return Vector2(lhs.x * rhs, lhs.y * rhs);
}

constexpr auto operator*(float lhs, Vector2 rhs) noexcept -> Vector2 {
    return Vector2(lhs * rhs.x, lhs * rhs.y);
}

constexpr auto operator/(Vector2 lhs, Vector2 rhs) noexcept -> Vector2 {
    return Vector2(lhs.x / rhs.x, lhs.y / rhs.y);
}

constexpr auto operator/(Vector2 lhs, float rhs) noexcept -> Vector2 {
    return Vector2(lhs.x / rhs, lhs.y / rhs);
}

constexpr auto operator/(float lhs, Vector2 rhs) noexcept -> Vector2 {
    return Vector2(lhs / rhs.x, lhs / rhs.y);
}

/// @brief
///   Calculate dot production of the specified vectors.
///
/// @param lhs
///   The first vector to calculate the dot production.
/// @param rhs
///   The second vector to calculate the dot production.
///
/// @return
///   A floating value that represents the dot production.
[[nodiscard]]
constexpr auto dot(Vector2 lhs, Vector2 rhs) noexcept -> float {
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

/// @brief
///   Perform linear interpolation between the 2 vectors.
///
/// @param start
///   The first vector to be interpolated.
/// @param end
///   The second vector to be interpolated.
/// @param t
///   Interpolation factor, must between 0 and 1. Passing 0 will return @p start and passing 1 will
///   return @p end.
///
/// @return
///   The interpolation result vector.
[[nodiscard]]
constexpr auto lerp(Vector2 start, Vector2 end, float t) noexcept -> Vector2 {
    return start + (end - start) * t;
}

/// @brief
///   Get element-wise absolute values of the specified vector.
///
/// @param vec
///   The vector to get absolute values.
///
/// @return
///   A new vector that contains element-wise absolute values of the original vector.
[[nodiscard]]
constexpr auto abs(Vector2 vec) noexcept -> Vector2 {
    return Vector2{
        vec.x < 0 ? -vec.x : vec.x,
        vec.y < 0 ? -vec.y : vec.y,
    };
}

/// @brief
///   Get element-wise minimum element of the specified vectors.
///
/// @param lhs
///   The first vector to get minimum elements.
/// @param rhs
///   The second vector to get minimum elements.
///
/// @return
///   A new vector that contains element-wise minimum values of the 2 vectors.
[[nodiscard]]
constexpr auto min(Vector2 lhs, Vector2 rhs) noexcept -> Vector2 {
    return Vector2{
        lhs.x < rhs.x ? lhs.x : rhs.x,
        lhs.y < rhs.y ? lhs.y : rhs.y,
    };
}

/// @brief
///   Get element-wise maximum element of the specified vectors.
///
/// @param lhs
///   The first vector to get maximum elements.
/// @param rhs
///   The second vector to get maximum elements.
///
/// @return
///   A new vector that contains element-wise maximum values of the 2 vectors.
[[nodiscard]]
constexpr auto max(Vector2 lhs, Vector2 rhs) noexcept -> Vector2 {
    return Vector2{
        lhs.x < rhs.x ? rhs.x : lhs.x,
        lhs.y < rhs.y ? rhs.y : lhs.y,
    };
}

/// @brief
///   Clamp each element of the specified vector into the specified range.
///
/// @param vec
///   The vector to be clamped.
/// @param floor
///   A vector that contains minimum available values of each element.
/// @param ceil
///   A vector that contains maximum available values of each element.
///
/// @return
///   A new vector that contains clamped elements.
[[nodiscard]]
constexpr auto clamp(Vector2 vec, Vector2 floor, Vector2 ceil) noexcept -> Vector2 {
    return max(floor, min(vec, ceil));
}

struct Vector3 {
#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wpedantic"
#elif defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4201)
#endif
    union {
        float m_arr[3];
        struct {
            float x;
            float y;
            float z;
        };
    };
#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#    pragma warning(pop)
#endif

    /// @brief
    ///   Create a 3D vector and initialize all elements to 0.
    constexpr Vector3() noexcept : m_arr{0, 0, 0} {}

    /// @brief
    ///   Create a 3D vector and initialize all elements to the specified value.
    ///
    /// @param v
    ///   Value to be set to elements of this vector.
    explicit constexpr Vector3(float v) noexcept : m_arr{v, v, v} {}

    /// @brief
    ///   Create a 3D vector and initialize elements.
    ///
    /// @param x
    ///   Value of the first element of this vector.
    /// @param yz
    ///   A 2D vector that contains values of the second and third element of this vector.
    constexpr Vector3(float x, Vector2 yz) noexcept : m_arr{x, yz.x, yz.y} {}

    /// @brief
    ///   Create a 3D vector and initialize elements.
    ///
    /// @param xy
    ///   A 2D vector that contains values of the first and second element of this vector.
    /// @param z
    ///   Value of the third element of this vector.
    constexpr Vector3(Vector2 xy, float z) noexcept : m_arr{xy.x, xy.y, z} {}

    /// @brief
    ///   Create a 3D vector and initialize elements.
    ///
    /// @param x
    ///   Value of the first element of this vector.
    /// @param y
    ///   Value of the second element of this vector.
    /// @param z
    ///   Value of the third element of this vector.
    constexpr Vector3(float x, float y, float z) noexcept : m_arr{x, y, z} {}

    /// @brief
    ///   Create a 3D vector and initialize elements.
    ///
    /// @param arr
    ///   A float array that contains at least 3 elements.
    explicit constexpr Vector3(const float arr[3]) noexcept : m_arr{arr[0], arr[1], arr[2]} {}

    /// @brief
    ///   Random access elements in this vector by index.
    /// @note
    ///   No boundary check performed.
    ///
    /// @param i
    ///   Index of the element to be accessed.
    ///
    /// @return
    ///   Reference to the specified element.
    constexpr auto operator[](std::size_t i) noexcept -> float & {
        return m_arr[i];
    }

    /// @brief
    ///   Random access elements in this vector by index.
    /// @note
    ///   No boundary check performed.
    ///
    /// @param i
    ///   Index of the element to be accessed.
    ///
    /// @return
    ///   Reference to the specified element.
    constexpr auto operator[](std::size_t i) const noexcept -> const float & {
        return m_arr[i];
    }

    /// @brief
    ///   Calculate of this 3D vector.
    /// @note
    ///   This method requires a square root operation which is time-consuming.
    ///
    /// @return
    ///   Length of this 3D vector.
    [[nodiscard]]
    auto length() const noexcept -> float {
        return std::sqrt(x * x + y * y + z * z);
    }

    /// @brief
    ///   Normalize this vector.
    /// @remark
    ///   To get normalized vector of this one without modifying this vector, use @p
    ///   Vector3::normalized() instead.
    ///
    /// @return
    ///   Reference to this vector.
    auto normalize() noexcept -> Vector3 & {
        const float len    = length();
        const float invLen = 1.0f / len;

        x *= invLen;
        y *= invLen;
        z *= invLen;

        return *this;
    }

    /// @brief
    ///   Get normalized vector of this one.
    /// @note
    ///   This vector itself is not modified. To normalize this vector itself, use @p
    ///   Vector3::normalize() instead.
    ///
    /// @return
    ///   Normalized version of this vector.
    [[nodiscard]]
    auto normalized() const noexcept -> Vector3 {
        const float len    = length();
        const float invLen = 1.0f / len;

        return Vector3(x * invLen, y * invLen, z * invLen);
    }

    constexpr auto operator+=(float rhs) noexcept -> Vector3 & {
        x += rhs;
        y += rhs;
        z += rhs;
        return *this;
    }

    constexpr auto operator+=(Vector3 rhs) noexcept -> Vector3 & {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    constexpr auto operator-=(float rhs) noexcept -> Vector3 & {
        x -= rhs;
        y -= rhs;
        z -= rhs;
        return *this;
    }

    constexpr auto operator-=(Vector3 rhs) noexcept -> Vector3 & {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    constexpr auto operator*=(float rhs) noexcept -> Vector3 & {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }

    constexpr auto operator*=(Vector3 rhs) noexcept -> Vector3 & {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }

    constexpr auto operator/=(float rhs) noexcept -> Vector3 & {
        x /= rhs;
        y /= rhs;
        z /= rhs;
        return *this;
    }

    constexpr auto operator/=(Vector3 rhs) noexcept -> Vector3 & {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        return *this;
    }
};

constexpr auto operator+(Vector3 vec) noexcept -> Vector3 {
    return vec;
}

constexpr auto operator-(Vector3 vec) noexcept -> Vector3 {
    return Vector3(-vec.x, -vec.y, -vec.z);
}

constexpr auto operator==(Vector3 lhs, Vector3 rhs) noexcept -> bool {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

constexpr auto operator!=(Vector3 lhs, Vector3 rhs) noexcept -> bool {
    return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z;
}

constexpr auto operator+(Vector3 lhs, Vector3 rhs) noexcept -> Vector3 {
    return Vector3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

constexpr auto operator+(Vector3 lhs, float rhs) noexcept -> Vector3 {
    return Vector3(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs);
}

constexpr auto operator+(float lhs, Vector3 rhs) noexcept -> Vector3 {
    return Vector3(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z);
}

constexpr auto operator-(Vector3 lhs, Vector3 rhs) noexcept -> Vector3 {
    return Vector3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

constexpr auto operator-(Vector3 lhs, float rhs) noexcept -> Vector3 {
    return Vector3(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs);
}

constexpr auto operator-(float lhs, Vector3 rhs) noexcept -> Vector3 {
    return Vector3(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z);
}

constexpr auto operator*(Vector3 lhs, Vector3 rhs) noexcept -> Vector3 {
    return Vector3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
}

constexpr auto operator*(Vector3 lhs, float rhs) noexcept -> Vector3 {
    return Vector3(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);
}

constexpr auto operator*(float lhs, Vector3 rhs) noexcept -> Vector3 {
    return Vector3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}

constexpr auto operator/(Vector3 lhs, Vector3 rhs) noexcept -> Vector3 {
    return Vector3(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z);
}

constexpr auto operator/(Vector3 lhs, float rhs) noexcept -> Vector3 {
    return Vector3(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);
}

constexpr auto operator/(float lhs, Vector3 rhs) noexcept -> Vector3 {
    return Vector3(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z);
}

/// @brief
///   Calculate dot production of the specified vectors.
///
/// @param lhs
///   The first vector to calculate the dot production.
/// @param rhs
///   The second vector to calculate the dot production.
///
/// @return
///   A floating value that represents the dot production.
[[nodiscard]]
constexpr auto dot(Vector3 lhs, Vector3 rhs) noexcept -> float {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

/// @brief
///   Calculate cross production of the specified vectors.
///
/// @param lhs
///   The first 3D vector parameter to calculate the cross production.
/// @param rhs
///   The second 3D vector parameter to calculate the cross production.
///
/// @return
///   A 3D vector that represents result of the cross production.
[[nodiscard]]
constexpr auto cross(Vector3 lhs, Vector3 rhs) noexcept -> Vector3 {
    return Vector3{
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x,
    };
}

/// @brief
///   Perform linear interpolation between the 2 vectors.
///
/// @param start
///   The first vector to be interpolated.
/// @param end
///   The second vector to be interpolated.
/// @param t
///   Interpolation factor, must between 0 and 1. Passing 0 will return @p start and passing 1 will
///   return @p end.
///
/// @return
///   The interpolation result vector.
[[nodiscard]]
constexpr auto lerp(Vector3 start, Vector3 end, float t) noexcept -> Vector3 {
    return start + (end - start) * t;
}

/// @brief
///   Get element-wise absolute values of the specified vector.
///
/// @param vec
///   The vector to get absolute values.
///
/// @return
///   A new vector that contains element-wise absolute values of the original vector.
[[nodiscard]]
constexpr auto abs(Vector3 vec) noexcept -> Vector3 {
    return Vector3{
        vec.x < 0 ? -vec.x : vec.x,
        vec.y < 0 ? -vec.y : vec.y,
        vec.z < 0 ? -vec.z : vec.z,
    };
}

/// @brief
///   Get element-wise minimum element of the specified vectors.
///
/// @param lhs
///   The first vector to get minimum elements.
/// @param rhs
///   The second vector to get minimum elements.
///
/// @return
///   A new vector that contains element-wise minimum values of the 2 vectors.
[[nodiscard]]
constexpr auto min(Vector3 lhs, Vector3 rhs) noexcept -> Vector3 {
    return Vector3{
        lhs.x < rhs.x ? lhs.x : rhs.x,
        lhs.y < rhs.y ? lhs.y : rhs.y,
        lhs.z < rhs.z ? lhs.z : rhs.z,
    };
}

/// @brief
///   Get element-wise maximum element of the specified vectors.
///
/// @param lhs
///   The first vector to get maximum elements.
/// @param rhs
///   The second vector to get maximum elements.
///
/// @return
///   A new vector that contains element-wise maximum values of the 2 vectors.
[[nodiscard]]
constexpr auto max(Vector3 lhs, Vector3 rhs) noexcept -> Vector3 {
    return Vector3{
        lhs.x < rhs.x ? rhs.x : lhs.x,
        lhs.y < rhs.y ? rhs.y : lhs.y,
        lhs.z < rhs.z ? rhs.z : lhs.z,
    };
}

/// @brief
///   Clamp each element of the specified vector into the specified range.
///
/// @param vec
///   The vector to be clamped.
/// @param floor
///   A vector that contains minimum available values of each element.
/// @param ceil
///   A vector that contains maximum available values of each element.
///
/// @return
///   A new vector that contains clamped elements.
[[nodiscard]]
constexpr auto clamp(Vector3 vec, Vector3 floor, Vector3 ceil) noexcept -> Vector3 {
    return max(floor, min(vec, ceil));
}

struct alignas(16) Vector4 {
#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wpedantic"
#elif defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4201)
#endif
    union {
        float m_arr[4];
        struct {
            float x;
            float y;
            float z;
            float w;
        };
    };
#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#    pragma warning(pop)
#endif

    /// @brief
    ///   Create a 4D vector and initialize all elements to 0.
    constexpr Vector4() noexcept : m_arr{0, 0, 0, 0} {}

    /// @brief
    ///   Create a 4D vector and initialize all elements to the specified value.
    ///
    /// @param v
    ///   Value to be set to elements of this vector.
    explicit constexpr Vector4(float v) noexcept : m_arr{v, v, v, v} {}

    /// @brief
    ///   Create a 4D vector and initialize elements.
    ///
    /// @param x
    ///   Value of the first element of this vector.
    /// @param yzw
    ///   A 3D vector that contains values of the second, third and forth element of this vector.
    constexpr Vector4(float x, Vector3 yzw) noexcept : m_arr{x, yzw.x, yzw.y, yzw.z} {}

    /// @brief
    ///   Create a 4D vector and initialize elements.
    ///
    /// @param xyz
    ///   A 3D vector that contains values of the first, second and third element of this vector.
    /// @param w
    ///   Value of the forth element of this vector.
    constexpr Vector4(Vector3 xyz, float w) noexcept : m_arr{xyz.x, xyz.y, xyz.z, w} {}

    /// @brief
    ///   Create a 4D vector and initialize elements.
    ///
    /// @param x
    ///   Value of the first element of this vector.
    /// @param y
    ///   Value of the second element of this vector.
    /// @param zw
    ///   A 2D vector that contains values of the third and forth element of this vector.
    constexpr Vector4(float x, float y, Vector2 zw) noexcept : m_arr{x, y, zw.x, zw.y} {}

    /// @brief
    ///   Create a 4D vector and initialize elements.
    ///
    /// @param x
    ///   Value of the first element of this vector.
    /// @param yz
    ///   A 2D vector that contains values of the second and third element of this vector.
    /// @param w
    ///   Value of the forth element of this vector.
    constexpr Vector4(float x, Vector2 yz, float w) noexcept : m_arr{x, yz.x, yz.y, w} {}

    /// @brief
    ///   Create a 4D vector and initialize elements.
    ///
    /// @param xy
    ///   A 2D vector that contains values of the first and second element of this vector.
    /// @param z
    ///   Value of the third element of this vector.
    /// @param w
    ///   Value of the forth element of this vector.
    constexpr Vector4(Vector2 xy, float z, float w) noexcept : m_arr{xy.x, xy.y, z, w} {}

    /// @brief
    ///   Create a 4D vector and initialize elements.
    ///
    /// @param xy
    ///   A 2D vector that contains values of the first and second element of this vector.
    /// @param zw
    ///   A 2D vector that contains values of the third and forth element of this vector.
    constexpr Vector4(Vector2 xy, Vector2 zw) noexcept : m_arr{xy.x, xy.y, zw.x, zw.y} {}

    /// @brief
    ///   Create a 4D vector and initialize elements.
    ///
    /// @param x
    ///   Value of the first element of this vector.
    /// @param y
    ///   Value of the second element of this vector.
    /// @param z
    ///   Value of the third element of this vector.
    /// @param w
    ///   Value of the forth element of this vector.
    constexpr Vector4(float x, float y, float z, float w) noexcept : m_arr{x, y, z, w} {}

    /// @brief
    ///   Create a 4D vector and initialize elements.
    ///
    /// @param arr
    ///   A float array that contains at least 4 elements.
    explicit constexpr Vector4(const float arr[4]) noexcept
        : m_arr{arr[0], arr[1], arr[2], arr[3]} {}

    /// @brief
    ///   Random access elements in this vector by index.
    /// @note
    ///   No boundary check performed.
    ///
    /// @param i
    ///   Index of the element to be accessed.
    ///
    /// @return
    ///   Reference to the specified element.
    constexpr auto operator[](std::size_t i) noexcept -> float & {
        return m_arr[i];
    }

    /// @brief
    ///   Random access elements in this vector by index.
    /// @note
    ///   No boundary check performed.
    ///
    /// @param i
    ///   Index of the element to be accessed.
    ///
    /// @return
    ///   Reference to the specified element.
    constexpr auto operator[](std::size_t i) const noexcept -> const float & {
        return m_arr[i];
    }

    /// @brief
    ///   Calculate of this 4D vector.
    /// @note
    ///   This method requires a square root operation which is time-consuming.
    ///
    /// @return
    ///   Length of this 4D vector.
    [[nodiscard]]
    auto length() const noexcept -> float {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    /// @brief
    ///   Normalize this vector.
    /// @remark
    ///   To get normalized vector of this one without modifying this vector, use @p
    ///   Vector4::normalized() instead.
    ///
    /// @return
    ///   Reference to this vector.
    auto normalize() noexcept -> Vector4 & {
        const float len    = length();
        const float invLen = 1.0f / len;

        x *= invLen;
        y *= invLen;
        z *= invLen;
        w *= invLen;

        return *this;
    }

    /// @brief
    ///   Get normalized vector of this one.
    /// @note
    ///   This vector itself is not modified. To normalize this vector itself, use @p
    ///   Vector4::normalize() instead.
    ///
    /// @return
    ///   Normalized version of this vector.
    [[nodiscard]]
    auto normalized() const noexcept -> Vector4 {
        const float len    = length();
        const float invLen = 1.0f / len;

        return Vector4(x * invLen, y * invLen, z * invLen, w * invLen);
    }

    constexpr auto operator+=(float rhs) noexcept -> Vector4 & {
        x += rhs;
        y += rhs;
        z += rhs;
        w += rhs;
        return *this;
    }

    constexpr auto operator+=(Vector4 rhs) noexcept -> Vector4 & {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }

    constexpr auto operator-=(float rhs) noexcept -> Vector4 & {
        x -= rhs;
        y -= rhs;
        z -= rhs;
        w -= rhs;
        return *this;
    }

    constexpr auto operator-=(Vector4 rhs) noexcept -> Vector4 & {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }

    constexpr auto operator*=(float rhs) noexcept -> Vector4 & {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        w *= rhs;
        return *this;
    }

    constexpr auto operator*=(Vector4 rhs) noexcept -> Vector4 & {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        w *= rhs.w;
        return *this;
    }

    constexpr auto operator/=(float rhs) noexcept -> Vector4 & {
        x /= rhs;
        y /= rhs;
        z /= rhs;
        w /= rhs;
        return *this;
    }

    constexpr auto operator/=(Vector4 rhs) noexcept -> Vector4 & {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        w /= rhs.w;
        return *this;
    }
};

constexpr auto operator+(Vector4 vec) noexcept -> Vector4 {
    return vec;
}

constexpr auto operator-(Vector4 vec) noexcept -> Vector4 {
    return Vector4(-vec.x, -vec.y, -vec.z, -vec.w);
}

constexpr auto operator==(Vector4 lhs, Vector4 rhs) noexcept -> bool {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
}

constexpr auto operator!=(Vector4 lhs, Vector4 rhs) noexcept -> bool {
    return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z || lhs.w != rhs.w;
}

constexpr auto operator+(Vector4 lhs, Vector4 rhs) noexcept -> Vector4 {
    return Vector4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
}

constexpr auto operator+(Vector4 lhs, float rhs) noexcept -> Vector4 {
    return Vector4(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs);
}

constexpr auto operator+(float lhs, Vector4 rhs) noexcept -> Vector4 {
    return Vector4(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z, lhs + rhs.w);
}

constexpr auto operator-(Vector4 lhs, Vector4 rhs) noexcept -> Vector4 {
    return Vector4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
}

constexpr auto operator-(Vector4 lhs, float rhs) noexcept -> Vector4 {
    return Vector4(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs, lhs.w - rhs);
}

constexpr auto operator-(float lhs, Vector4 rhs) noexcept -> Vector4 {
    return Vector4(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z, lhs - rhs.w);
}

constexpr auto operator*(Vector4 lhs, Vector4 rhs) noexcept -> Vector4 {
    return Vector4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w);
}

constexpr auto operator*(Vector4 lhs, float rhs) noexcept -> Vector4 {
    return Vector4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs);
}

constexpr auto operator*(float lhs, Vector4 rhs) noexcept -> Vector4 {
    return Vector4(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
}

constexpr auto operator/(Vector4 lhs, Vector4 rhs) noexcept -> Vector4 {
    return Vector4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w);
}

constexpr auto operator/(Vector4 lhs, float rhs) noexcept -> Vector4 {
    return Vector4(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs);
}

constexpr auto operator/(float lhs, Vector4 rhs) noexcept -> Vector4 {
    return Vector4(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z, lhs / rhs.w);
}

/// @brief
///   Calculate dot production of the specified vectors.
///
/// @param lhs
///   The first vector to calculate the dot production.
/// @param rhs
///   The second vector to calculate the dot production.
///
/// @return
///   A floating value that represents the dot production.
[[nodiscard]]
constexpr auto dot(Vector4 lhs, Vector4 rhs) noexcept -> float {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

/// @brief
///   Calculate cross production of the specified vectors.
///
/// @param lhs
///   The first 3D homogeneous coordinate vector parameter to calculate the cross production.
/// @param rhs
///   The second 3D homogeneous coordinate vector parameter to calculate the cross production.
///
/// @return
///   A 3D homogeneous coordinate vector that represents result of the cross production.
[[nodiscard]]
constexpr auto cross(Vector4 lhs, Vector4 rhs) noexcept -> Vector4 {
    return Vector4{
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x,
        0,
    };
}

/// @brief
///   Perform linear interpolation between the 2 vectors.
///
/// @param start
///   The first vector to be interpolated.
/// @param end
///   The second vector to be interpolated.
/// @param t
///   Interpolation factor, must between 0 and 1. Passing 0 will return @p start and passing 1 will
///   return @p end.
///
/// @return
///   The interpolation result vector.
[[nodiscard]]
constexpr auto lerp(Vector4 start, Vector4 end, float t) noexcept -> Vector4 {
    return start + (end - start) * t;
}

/// @brief
///   Get element-wise absolute values of the specified vector.
///
/// @param vec
///   The vector to get absolute values.
///
/// @return
///   A new vector that contains element-wise absolute values of the original vector.
[[nodiscard]]
constexpr auto abs(Vector4 vec) noexcept -> Vector4 {
    return Vector4{
        vec.x < 0 ? -vec.x : vec.x,
        vec.y < 0 ? -vec.y : vec.y,
        vec.z < 0 ? -vec.z : vec.z,
        vec.w < 0 ? -vec.w : vec.w,
    };
}

/// @brief
///   Get element-wise minimum element of the specified vectors.
///
/// @param lhs
///   The first vector to get minimum elements.
/// @param rhs
///   The second vector to get minimum elements.
///
/// @return
///   A new vector that contains element-wise minimum values of the 2 vectors.
[[nodiscard]]
constexpr auto min(Vector4 lhs, Vector4 rhs) noexcept -> Vector4 {
    return Vector4{
        lhs.x < rhs.x ? lhs.x : rhs.x,
        lhs.y < rhs.y ? lhs.y : rhs.y,
        lhs.z < rhs.z ? lhs.z : rhs.z,
        lhs.w < rhs.w ? lhs.w : rhs.w,
    };
}

/// @brief
///   Get element-wise maximum element of the specified vectors.
///
/// @param lhs
///   The first vector to get maximum elements.
/// @param rhs
///   The second vector to get maximum elements.
///
/// @return
///   A new vector that contains element-wise maximum values of the 2 vectors.
[[nodiscard]]
constexpr auto max(Vector4 lhs, Vector4 rhs) noexcept -> Vector4 {
    return Vector4{
        lhs.x < rhs.x ? rhs.x : lhs.x,
        lhs.y < rhs.y ? rhs.y : lhs.y,
        lhs.z < rhs.z ? rhs.z : lhs.z,
        lhs.w < rhs.w ? rhs.w : lhs.w,
    };
}

/// @brief
///   Clamp each element of the specified vector into the specified range.
///
/// @param vec
///   The vector to be clamped.
/// @param floor
///   A vector that contains minimum available values of each element.
/// @param ceil
///   A vector that contains maximum available values of each element.
///
/// @return
///   A new vector that contains clamped elements.
[[nodiscard]]
constexpr auto clamp(Vector4 vec, Vector4 floor, Vector4 ceil) noexcept -> Vector4 {
    return max(floor, min(vec, ceil));
}

} // namespace ink
