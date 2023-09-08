#pragma once

#include "ink/math/quaternion.hpp"

namespace ink {

class Camera {
public:
    /// @brief
    ///   Create a new camera with the specified parameters.
    Camera(Vector3 pos, float fov, float aspect, float zNear = 1.0f, float zFar = 1000.0f) noexcept
        : m_position(pos),
          m_rotation(1.0f),
          m_fovY(fov),
          m_aspectRatio(aspect),
          m_zNear(zNear),
          m_zFar(zFar),
          m_isViewMatrixDirty(true),
          m_isProjectionMatrixDirty(true) {}

    [[nodiscard]] auto position() const noexcept -> Vector3 { return m_position; }
    [[nodiscard]] auto fieldOfView() const noexcept -> float { return m_fovY; }
    [[nodiscard]] auto nearClip() const noexcept -> float { return m_zNear; }
    [[nodiscard]] auto farClip() const noexcept -> float { return m_zFar; }

    [[nodiscard]] auto forward() const noexcept -> Vector3 {
        Quaternion dir{0.0f, 0.0f, 1.0f, 0.0f};
        dir = m_rotation * dir * m_rotation.conjugated();
        return {dir.x, dir.y, dir.z};
    }

    [[nodiscard]] auto viewMatrix() const noexcept -> const Matrix4 & {
        if (m_isViewMatrixDirty) {
            m_viewMatrix        = lookTo(m_position, forward(), {0.0f, 1.0f, 0.0f});
            m_isViewMatrixDirty = false;
        }
        return m_viewMatrix;
    }

    [[nodiscard]] auto projectionMatrix() const noexcept -> const Matrix4 & {
        if (m_isProjectionMatrixDirty) {
            m_projectionMatrix        = perspective(m_fovY, m_aspectRatio, m_zNear, m_zFar);
            m_isProjectionMatrixDirty = false;
        }
        return m_projectionMatrix;
    }

    auto rotate(float pitch, float yaw, float roll) noexcept -> void {
        m_rotation          = Quaternion(pitch, yaw, roll) * m_rotation;
        m_isViewMatrixDirty = true;
    }

    auto translate(Vector3 offset) noexcept -> void {
        m_position += offset;
        m_isViewMatrixDirty = true;
    }

private:
    Vector3    m_position;
    Quaternion m_rotation;
    float      m_fovY;
    float      m_aspectRatio;
    float      m_zNear;
    float      m_zFar;

    mutable bool    m_isViewMatrixDirty;
    mutable bool    m_isProjectionMatrixDirty;
    mutable Matrix4 m_viewMatrix;
    mutable Matrix4 m_projectionMatrix;
};

} // namespace ink
