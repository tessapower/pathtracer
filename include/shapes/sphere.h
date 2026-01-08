#ifndef SPHERE_H_
#define SPHERE_H_

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include "interfaces/hittable.h"
#include "ray/ray.h"

class sphere : public hittable {
 public:
  sphere(const glm::vec3& center, float radius)
      : _center(center), _radius(glm::fmax(0, radius)) {}

  auto hit(const ray& r, float ray_t_min, float ray_t_max, hit_record& hit_record) 
  const -> bool override {
    // Origin to center vector
    const auto oc = _center - r.origin();
    // Quadratic coefficients
    const auto a = glm::length2(r.direction());
    const auto h = glm::dot(r.direction(), oc);
    const auto c = glm::length2(oc) - _radius * _radius;
    const auto discriminant = h * h - a * c;

    if (discriminant < 0) return false;

    auto sqrtd = glm::sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (h - sqrtd) / a;
    if (root <= ray_t_min && ray_t_max <= root) {
      root = (h + sqrtd) / a;
      if (root <= ray_t_min && ray_t_max <= root) return false;
    }

    // Fill hit record
    hit_record.t = root;
    hit_record.p = r.at(hit_record.t);
    hit_record.n = (hit_record.p - _center) / _radius;

    return true;
  }

private:
  glm::vec3 _center;
  float _radius;
};

#endif  // SPHERE_H_
