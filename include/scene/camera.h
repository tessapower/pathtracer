#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

namespace pathtracer
{
/// <summary>
/// A struct representing the camera data that will be sent to the GPU for
/// the path tracing shader. This struct is designed to be tightly packed
/// aligned to 16 bytes for efficient GPU access.
/// </summary>
///
/// <param name="position">The position of the camera in world space.</param>
/// <param name="fovTanHalf">The precomputed tangent of half the field of view
/// angle, used for ray direction calculations in the shader.</param>
///
/// <param name="right">The right vector of the camera, used for horizontal ray
/// offsets.</param>
///
/// <param name="aspectRatio">The aspect ratio of the camera, used to correct
/// for non-square pixels.</param>
///
/// <param name="up">The up vector of the camera, used for vertical ray
/// offsets.</param>
///
/// <param name="forward">The forward vector of the camera, representing the
/// direction the camera is looking at, used as the base direction for rays.
/// This vector should be normalized.</param>
///
/// <param name="_pad0">Explicit padding to ensure proper alignment of the
/// struct. Ignored in the shader.</param>
///
/// <param name="_pad1">Explicit padding to ensure proper alignment of the
/// struct. Ignored in the shader.</param>
struct alignas(16) CameraGPUData
{
    // Camera position in world space. Used as ray origin in the shader.
    glm::vec3 position;

    // Scales the UV coords to match fov. Pre-computerd tan(FOV/2) for ray
    // scaling, cheaper CPU-side than per-pixel on the GPU.
    float fovTanHalf;

    // Camera right vector. Used to offset rays horizontally based on px pos.
    glm::vec3 right;

    // Width/height ratio. Corrects for non-square pixels so image isn't squish
    float aspectRatio;

    // Camera up vector, used to offset rays vertically based on px pos.
    glm::vec3 up;

    // Explicit padding
    float _pad0;

    // Camera forward vector, normalized, pointing at target.
    // Base direction for all rays.
    glm::vec3 forward;

    // Explicit padding
    float _pad1;
};

class Camera
{
  public:
    /// <summary>
    /// Constructs a camera with the given parameters. The camera starts at a
    /// default position and orientation, looking at the origin.
    /// </summary>
    ///
    /// <param name="fov">Field of view in degrees.</param>
    /// <param name="aspectRatio">Aspect ratio of the camera.</param>
    /// <param name="nearPlane">Near clipping plane distance.</param>
    /// <param name="farPlane">Far clipping plane distance.</param>
    Camera(float fov, float aspectRatio, float nearPlane, float farPlane)
        : m_fov(fov), m_aspectRatio(aspectRatio), m_nearPlane(nearPlane),
          m_farPlane(farPlane)
    {
        UpdateCameraVectors();
    };

    ~Camera() = default;

    /// <summary>
    /// Rotates the camera around the target point by the specified azimuth and
    /// elevation angles. The camera's position and orientation will be updated
    /// accordingly, and the camera will continue to look at the target point.
    /// </summary>
    ///
    /// <param name="deltaAzimuth">The change in azimuth angle in radians.
    /// The azimuth angle rotates the camera horizontally around the target.
    /// Positive azimuth values rotate the camera to the right.</param>
    ///
    /// <param name="deltaElevation">The change in elevation angle in radians.
    /// The elevation angle rotates the camera vertically. Positive elevation
    /// values rotate the camera upwards.</param>
    auto Rotate(float deltaAzimuth, float deltaElevation) -> void;

    /// <summary>
    /// Zooms the camera in or out by changing the radius of the orbit around
    /// the target point. A positive deltaRadius will zoom the camera out (move
    /// it away from the target), while a negative deltaRadius will zoom the
    /// camera in (move it closer to the target). The camera's position will be
    /// updated accordingly, and the camera will continue to look at the target
    /// point.
    /// </summary>
    auto Zoom(float deltaRadius) -> void;

    /// <summary>
    /// Sets the target point that the camera is looking at. The camera will
    /// orbit around this point when rotated.
    /// </summary>
    ///
    /// <param name="target">The new target point in world space.</param>
    auto SetTarget(const glm::vec3& target) -> void
    {
        m_target = target;
        m_isDirty = true;
        UpdateCameraVectors();
    }

    /// <summary>
    /// Gets the current position of the camera in world space.
    /// </summary>
    auto GetPosition() const noexcept -> const glm::vec3&
    {
        return m_position;
    }

    /// <summary>
    /// Gets the current up vector of the camera, representing the vertical
    /// direction in world space.
    /// </summary>
    auto GetUp() const noexcept -> const glm::vec3&
    {
        return m_up;
    }

    /// <summary>
    /// Gets the current right vector of the camera, representing the
    /// horizontal direction in world space.
    /// </summary>
    auto GetRight() const noexcept -> const glm::vec3&
    {
        return m_right;
    }

    /// <summary>
    /// Gets the current forward vector of the camera, representing the
    /// direction the camera is looking at in world space.
    /// </summary>
    auto GetForward() const noexcept -> const glm::vec3&
    {
        return m_forward;
    }

    /// <summary>
    /// Gets the camera data formatted for GPU consumption.
    /// </summary>
    auto GetGPUData() const -> CameraGPUData
    {
        return CameraGPUData{m_position, glm::tan(m_fov * 0.5f),
                             m_right,    m_aspectRatio,
                             m_up,       0.0f,
                             m_forward,  0.0f};
    }

    /// <summary>
    /// Sets the aspect ratio of the camera. This should be called whenever the
    /// viewport size changes to ensure the camera's projection matrix is
    /// updated correctly.
    /// </summary>
    auto SetAspectRatio(float aspectRatio) -> void
    {
        m_aspectRatio = aspectRatio;
        m_isDirty = true;
    }

    auto IsDirty() const -> bool
    {
        return m_isDirty;
    }

    auto ClearDirty() -> void
    {
        m_isDirty = false;
    }

  private:
    auto UpdateCameraVectors() -> void;

    // Orbit params
    float m_radius = 5.0f;
    float m_azimuth = 0.0f;
    float m_elevation = 0.0f;
    glm::vec3 m_target{0.0f, 0.0f, 0.0f};
    glm::vec3 m_worldUp{0.0f, 1.0f, 0.0f};

    // Camera intrinsics
    float m_fov;
    float m_aspectRatio;
    float m_nearPlane;
    float m_farPlane;

    // Computed values
    glm::vec3 m_position;
    glm::vec3 m_forward;
    glm::vec3 m_right;
    glm::vec3 m_up;
    bool m_isDirty = true;
};
} // namespace pathtracer

#endif // CAMERA_H
