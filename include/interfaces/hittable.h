#ifndef HITTABLE_H_
#define HITTABLE_H_

#include <glm/glm.hpp>
#include "ray/ray.h"

class hit_record {
public:
    glm::vec3 p;
    glm::vec3 n;
    float t;
};

class hittable {
public:
    virtual ~hittable() = default;

    virtual auto hit(const ray& r, float ray_t_min, float ray_t_max,
        hit_record& hit_record) const -> bool = 0;
};

#endif  // HITTABLE_H_
