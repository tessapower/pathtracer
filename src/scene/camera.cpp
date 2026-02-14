#include "stdafx.h"

#include "scene/camera.h"

#include <glm/gtc/constants.hpp>

namespace pathtracer
{

static constexpr float TWO_PI = glm::two_pi<float>();

auto Camera::Rotate(float deltaAzimuth, float deltaElevation) -> void
{
    // Update the spherical coordinates based on the input deltas
    m_azimuth += deltaAzimuth;
    // Wrap the azimuth angle to keep it within [0, 2π)
    m_azimuth = glm::mod(m_azimuth, TWO_PI);

    // Don't worry about clamping elevation here, it will be clamped in
    // UpdateCameraVectors() to avoid gimbal lock
    m_elevation += deltaElevation;

    UpdateCameraVectors();
}

auto Camera::Zoom(float deltaRadius) -> void
{
    // const static float SPEED = 0.5f;
    static constexpr float MAX = 100.0f;
    static constexpr float MIN = 0.1f;

    m_radius += deltaRadius;

    // Prevent radius from too close or too far
    m_radius = glm::clamp(m_radius, MIN, MAX);

    UpdateCameraVectors();
}

auto Camera::UpdateCameraVectors() -> void
{
    /// NOTE TO SELF:
    /// To convert spherical coordinates (rho, theta, phi) to Cartesian
    /// coordinates (x, y, z), use the radius rho (distance from origin),
    /// polar/inclination angle theta (angle from positive z-axis), and
    /// azimuthal angle phi (angle from positive x-axis on xy-plane):
    ///
    /// x = rho * sin(theta) * cos(phi)
    /// y = rho * sin(theta) * sin(phi)
    /// z = rho * cos(theta)
    ///
    /// Where theta = m_elevation, phi = m_azimuth, rho = m_radius

    // Clamp elevation FIRST to avoid gimbal lock (caused by looking straight up
    // or down, which makes the right and up vectors degenerate!)
    static constexpr float epsilon = 0.001f;
    m_elevation = glm::clamp(m_elevation, -glm::half_pi<float>() + epsilon,
                             glm::half_pi<float>() - epsilon);

    // Convert spherical coordinates to Cartesian coordinates
    m_position.x =
        m_target.x + m_radius * glm::cos(m_elevation) * glm::sin(m_azimuth);
    m_position.y = m_target.y + m_radius * glm::sin(m_elevation);
    m_position.z =
        m_target.z + m_radius * glm::cos(m_elevation) * glm::cos(m_azimuth);

    // Compute basis vectors
    m_forward = glm::normalize(m_target - m_position);
    m_right = glm::normalize(glm::cross(m_forward, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_forward));

    m_isDirty = true;
}
} // namespace pathtracer
