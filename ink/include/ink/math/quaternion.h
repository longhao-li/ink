#pragma once

#include "ink/math/matrix.h"

#include <cfloat>

namespace ink {

struct alignas(16) Quaternion {
    float w; // Real part of this quaternion.
    float x; // Imaginary X
    float y; // Imaginary Y
    float z; // Imaginary Z

    /// @brief
    ///   Create a zero quaternion.
    constexpr Quaternion() noexcept : w(), x(), y(), z() {}

    /// @brief
    ///   Create a quaternion from a real number.
    ///
    /// @param real
    ///   Real part of this quaternion.
    constexpr Quaternion(float real) noexcept : w(real), x(), y(), z() {}

    /// @brief
    ///   Create a quaternion with the given values.
    ///
    /// @param real
    ///   Real part of this quaternion.
    /// @param imgX
    ///   Imaginary X of this quaternion.
    /// @param imgY
    ///   Imaginary Y of this quaternion.
    /// @param imgZ
    ///   Imaginary Z of this quaternion.
    constexpr Quaternion(float real, float imgX, float imgY, float imgZ) noexcept
        : w(real), x(imgX), y(imgY), z(imgZ) {}

    /// @brief
    ///   Create a quaternion from Euler angle.
    ///
    /// @param pitch
    ///   Pitch of the Euler angle in radian.
    /// @param yaw
    ///   Yaw of the Euler angle in radian.
    /// @param roll
    ///   Roll of the Euler angle in radian.
    Quaternion(float pitch, float yaw, float roll) noexcept {
        const float sinPitch = std::sin(pitch * 0.5f);
        const float cosPitch = std::cos(pitch * 0.5f);
        const float sinYaw   = std::sin(yaw * 0.5f);
        const float cosYaw   = std::cos(yaw * 0.5f);
        const float sinRoll  = std::sin(roll * 0.5f);
        const float cosRoll  = std::cos(roll * 0.5f);

        w = cosPitch * cosYaw * cosRoll - sinPitch * sinYaw * sinRoll;
        x = sinPitch * cosYaw * cosRoll + cosPitch * sinYaw * sinRoll;
        y = cosPitch * sinYaw * cosRoll - sinPitch * cosYaw * sinRoll;
        z = cosPitch * cosYaw * sinRoll + sinPitch * sinYaw * cosRoll;
    }

    /// @brief
    ///   Create a quaternion for rotation.
    ///
    /// @param axis
    ///   The axis to be rotated around.
    /// @param radian
    ///   Radian to rotate.
    Quaternion(Vector3 axis, float radian) noexcept {
        radian *= 0.5f;
        const float s = std::sin(radian);
        const float c = std::cos(radian);

        axis.normalize();

        w = c;
        x = s * axis.x;
        y = s * axis.y;
        z = s * axis.z;
    }

    /// @brief
    ///   Calculate length of this quaternion.
    /// @note
    ///   This method requires a square root operation which is time-consuming.
    ///
    /// @return
    ///   Length of this quaternion.
    [[nodiscard]]
    auto length() const noexcept -> float {
        return std::sqrt(w * w + x * x + y * y + z * z);
    }

    /// @brief
    ///   Normalize this quaternion.
    /// @remark
    ///   To get normalized quaternion of this one without modifying this quaternion, use @p
    ///   normalized() instead.
    ///
    /// @return
    ///   Reference to this quaternion.
    auto normalize() noexcept -> Quaternion & {
        const float len    = length();
        const float invLen = 1.0f / len;

        w *= invLen;
        x *= invLen;
        y *= invLen;
        z *= invLen;

        return *this;
    }

    /// @brief
    ///   Get normalized quaternion of this one.
    /// @note
    ///   This quaternion itself is not modified. To normalize this quaternion itself, use @p
    ///   normalize() instead.
    ///
    /// @return
    ///   Normalized version of this quaternion.
    [[nodiscard]]
    auto normalized() const noexcept -> Quaternion {
        const float len    = length();
        const float invLen = 1.0f / len;

        return Quaternion(w * invLen, x * invLen, y * invLen, z * invLen);
    }

    /// @brief
    ///   Convert this quaternion to its conjugate quaternion.
    /// @note
    ///   To get conjugate quaternion without modifying this quaternion, use @p conjugated()
    ///   instead.
    ///
    /// @return
    ///   Reference to this quaternion.
    constexpr auto conjugate() noexcept -> Quaternion & {
        x = -x;
        y = -y;
        z = -z;
        return *this;
    }

    /// @brief
    ///   Get conjugate quaternion of this one.
    /// @note
    ///   To conjugate this quaternion, use @p conjugate() instead.
    ///
    /// @return
    ///   Conjugate quaternion of this quaternion.
    [[nodiscard]]
    constexpr auto conjugated() const noexcept -> Quaternion {
        return Quaternion(w, -x, -y, -z);
    }

    /// @brief
    ///   Inverse this quaternion.
    /// @remark
    ///   To get inversed quaternion without modifying this quaternion, use @p inversed() instead.
    ///
    /// @return
    ///   Reference to this quaternion.
    constexpr auto inverse() noexcept -> Quaternion & {
        const float len2    = w * w + x * x + y * y + z * z;
        const float invLen2 = 1.0f / len2;

        w *= invLen2;
        x *= -invLen2;
        y *= -invLen2;
        z *= -invLen2;

        return *this;
    }

    /// @brief
    ///   Get inversed quaternion of this one.
    /// @remark
    ///   To inverse this quaternion, use @p inverse() instead.
    ///
    /// @return
    ///   A new quaternion that represents the inversed quaternion.
    [[nodiscard]]
    constexpr auto inversed() const noexcept -> Quaternion {
        const float len2    = w * w + x * x + y * y + z * z;
        const float invLen2 = 1.0f / len2;
        return Quaternion(w * invLen2, -x * invLen2, -y * invLen2, -z * invLen2);
    }

    /// @brief
    ///   Convert this quaternion to 4D rotation matrix.
    ///
    /// @return
    ///   A 4D rotation matrix that is equivalent to this quaternion.
    [[nodiscard]]
    constexpr auto toMatrix() const noexcept -> Matrix4 {
        const float x2 = 2 * x * x;
        const float y2 = 2 * y * y;
        const float z2 = 2 * z * z;
        const float xw = 2 * w * x;
        const float yw = 2 * w * y;
        const float zw = 2 * w * z;
        const float xy = 2 * x * y;
        const float xz = 2 * x * z;
        const float yz = 2 * y * z;

        return Matrix4{
            Vector4{1 - y2 - z2, xy - zw, yw + xz, 0.0f},
            Vector4{xy + zw, 1 - x2 - z2, yz - xw, 0.0f},
            Vector4{xz - yw, xw + yz, 1 - x2 - y2, 0.0f},
            Vector4{0.0f, 0.0f, 0.0f, 1.0f},
        };
    }

    constexpr auto operator+() const noexcept -> Quaternion {
        return *this;
    }

    constexpr auto operator-() const noexcept -> Quaternion {
        return Quaternion(-w, -x, -y, -z);
    }

    constexpr auto operator+=(float rhs) noexcept -> Quaternion & {
        w += rhs;
        return *this;
    }

    constexpr auto operator+=(Quaternion rhs) noexcept -> Quaternion & {
        w += rhs.w;
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    constexpr auto operator-=(float rhs) noexcept -> Quaternion & {
        w -= rhs;
        return *this;
    }

    constexpr auto operator-=(Quaternion rhs) noexcept -> Quaternion & {
        w -= rhs.w;
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    constexpr auto operator*=(float rhs) noexcept -> Quaternion & {
        w *= rhs;
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }

    constexpr auto operator*=(Quaternion rhs) noexcept -> Quaternion & {
        const float a = w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z;
        const float b = x * rhs.w + w * rhs.x - z * rhs.y + y * rhs.z;
        const float c = y * rhs.w + z * rhs.x + w * rhs.y - x * rhs.z;
        const float d = z * rhs.w - y * rhs.x + x * rhs.y + w * rhs.z;

        w = a;
        x = b;
        y = c;
        z = d;
        return *this;
    }

    constexpr auto operator/=(float rhs) noexcept -> Quaternion & {
        w /= rhs;
        x /= rhs;
        y /= rhs;
        z /= rhs;
        return *this;
    }

    constexpr auto operator/=(Quaternion rhs) noexcept -> Quaternion & {
        *this *= rhs.inversed();
        return *this;
    }
};

constexpr auto operator==(Quaternion lhs, Quaternion rhs) noexcept -> bool {
    return lhs.w == rhs.w && lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

constexpr auto operator!=(Quaternion lhs, Quaternion rhs) noexcept -> bool {
    return lhs.w != rhs.w || lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z;
}

constexpr auto operator+(Quaternion lhs, Quaternion rhs) noexcept -> Quaternion {
    return Quaternion(lhs.w + rhs.w, lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

constexpr auto operator+(Quaternion lhs, float rhs) noexcept -> Quaternion {
    return Quaternion(lhs.w + rhs, lhs.x, lhs.y, lhs.z);
}

constexpr auto operator+(float lhs, Quaternion rhs) noexcept -> Quaternion {
    return Quaternion(lhs + rhs.w, rhs.x, rhs.y, rhs.z);
}

constexpr auto operator-(Quaternion lhs, Quaternion rhs) noexcept -> Quaternion {
    return Quaternion(lhs.w - rhs.w, lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

constexpr auto operator-(Quaternion lhs, float rhs) noexcept -> Quaternion {
    return Quaternion(lhs.w - rhs, lhs.x, lhs.y, lhs.z);
}

constexpr auto operator-(float lhs, Quaternion rhs) noexcept -> Quaternion {
    return Quaternion(lhs - rhs.w, -rhs.x, -rhs.y, -rhs.z);
}

constexpr auto operator*(Quaternion lhs, Quaternion rhs) noexcept -> Quaternion {
    return Quaternion{
        lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z,
        lhs.x * rhs.w + lhs.w * rhs.x - lhs.z * rhs.y + lhs.y * rhs.z,
        lhs.y * rhs.w + lhs.z * rhs.x + lhs.w * rhs.y - lhs.x * rhs.z,
        lhs.z * rhs.w - lhs.y * rhs.x + lhs.x * rhs.y + lhs.w * rhs.z,
    };
}

constexpr auto operator*(Quaternion lhs, float rhs) noexcept -> Quaternion {
    return Quaternion(lhs.w * rhs, lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);
}

constexpr auto operator*(float lhs, Quaternion rhs) noexcept -> Quaternion {
    return Quaternion(lhs * rhs.w, lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}

constexpr auto operator/(Quaternion lhs, Quaternion rhs) noexcept -> Quaternion {
    return lhs * rhs.inversed();
}

constexpr auto operator/(Quaternion lhs, float rhs) noexcept -> Quaternion {
    return Quaternion(lhs.w / rhs, lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);
}

constexpr auto operator/(float lhs, Quaternion rhs) noexcept -> Quaternion {
    return lhs * rhs.inversed();
}

/// @brief
///   Calculate dot production of the specified quaternions.
///
/// @param lhs
///   The quaternions to calculate the dot production.
/// @param rhs
///   The second quaternions to calculate the dot production.
///
/// @return
///   A floating value that represents the dot production.
[[nodiscard]]
constexpr auto dot(Quaternion lhs, Quaternion rhs) noexcept -> float {
    return lhs.w * rhs.w + lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

/// @brief
///   Perform normalized linear interpolation between the 2 quaternions.
///
/// @param start
///   The first quaternions to be interpolated.
/// @param end
///   The second quaternions to be interpolated.
/// @param t
///   Interpolation factor, must between 0 and 1. Passing 0 will return @p start and passing 1 will
///   return @p end.
///
/// @return
///   The interpolation result quaternions.
[[nodiscard]]
inline auto nlerp(Quaternion start, Quaternion end, float t) noexcept -> Quaternion {
    return (start + (end - start) * t).normalized();
}

/// @brief
///   Perform spherical linear interpolation between the 2 quaternions.
///
/// @param start
///   The first quaternions to be interpolated.
/// @param end
///   The second quaternions to be interpolated.
/// @param t
///   Interpolation factor, must between 0 and 1. Passing 0 will return @p start and passing 1 will
///   return @p end.
///
/// @return
///   The interpolation result quaternions.
[[nodiscard]]
inline auto slerp(Quaternion start, Quaternion end, float t) noexcept -> Quaternion {
    float c = dot(start, end);

    // Interpolate the shortest path.
    if (c < 0) {
        c   = -c;
        end = -end;
    }

    const float s = std::sqrt(1 - c * c);

    if (s < FLT_EPSILON)
        return nlerp(start, end, t);

    const float invSin = 1.0f / s;
    const float theta  = std::atan2(s, c);
    const float t0     = std::sin((1.0f - t) * theta) * invSin;
    const float t1     = std::sin(t * theta) * invSin;

    return t0 * start + t1 * end;
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
constexpr auto Matrix4::rotate(Quaternion quat) noexcept -> Matrix4 & {
    *this *= quat.toMatrix();
    return *this;
}

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
constexpr auto Matrix4::rotated(Quaternion quat) const noexcept -> Matrix4 {
    return *this * quat.toMatrix();
}

} // namespace ink
