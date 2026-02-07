#ifndef RAY_H_
#define RAY_H_

#include <glm/glm.hpp>

class ray
{
  public:
    ray() = default;

    ray(const glm::vec3& origin, const glm::vec3& direction)
        : _origin(origin), _direction(direction)
    {
    }

    auto origin() const -> const glm::vec3&
    {
        return _origin;
    }
    auto direction() const -> const glm::vec3&
    {
        return _direction;
    }

    auto at(float t) const -> glm::vec3
    {
        return _origin + t * _direction;
    }

  private:
    glm::vec3 _origin{};
    glm::vec3 _direction{};
};

#endif
