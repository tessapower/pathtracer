#include <cstdint>
#include <cstdlib>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "ray/ray.h"
#include "utils/color.h"

static bool hit_sphere(const glm::vec3& center, const double radius,
                       const ray& r) {
  // Origin to center vector
  const auto oc = center - r.origin();
  // Quadratic coefficients
  const auto a = glm::dot(r.direction(), r.direction());
  const auto b = -2.0f * glm::dot(r.direction(), oc);
  const auto c = glm::dot(oc, oc) - radius * radius;
  auto discriminant = b * b - 4 * a * c;

  // If discriminant is positive, the ray hit the sphere
  return discriminant > 0;
}

static auto ray_color(const ray& r) -> color {
  if (hit_sphere(glm::vec3(0.0, 0.0, -1.0), 0.5, r))
    return color(1.0f, 0.0f, 0.0f);

  glm::vec3 unit_dir = glm::normalize(r.direction());
  auto a = 0.5f * (unit_dir.y + 1.0f);
  return glm::mix(color(1.0f, 1.0f, 1.0f), color(0.5f, 0.7f, 1.0f), a);
}

auto main() -> int {
  // Image
  auto aspect_ratio = 16.0 / 9.0;
  int image_width = 400;
  int image_height = static_cast<int>(image_width / aspect_ratio);
  image_height = (image_height < 1) ? 1 : image_height;
  const uint8_t channels = 3;

  // Create image buffer
  std::vector<unsigned char> image(image_width * image_height * channels);

  // Camera
  const auto focal_length = 1.0;
  const auto viewport_height = 2.0;
  const auto viewport_width =
      viewport_height * (static_cast<double>(image_width) / image_height);
  auto camera_origin = glm::vec3(0.0);

  // Calclulate horizontal and vertical vectors for viewport edges
  auto viewport_u = glm::vec3(viewport_width, 0.0, 0.0);
  auto viewport_v = glm::vec3(0, -viewport_height, 0.0);

  // Calculate horizontal and verta delta vectors between pixels
  auto pixel_delta_u = viewport_u / static_cast<float>(image_width);
  auto pixel_delta_v = viewport_v / static_cast<float>(image_height);

  // Calculate location of upper left pixel
  auto viewport_upper_left = camera_origin - glm::vec3(0.0, 0.0, focal_length) -
                             viewport_u / 2.0f - viewport_v / 2.0f;
  auto pixel00_loc =
      viewport_upper_left + 0.5f * (pixel_delta_u + pixel_delta_v);

  // Render
  for (auto row = 0; row < image_height; ++row) {
    std::clog << "\rScanlines remaining: " << (image_height - row) << ' '
              << std::flush;
    for (auto col = 0; col < image_width; ++col) {
      const auto pixel_center = pixel00_loc +
                                (static_cast<float>(col) * pixel_delta_u) +
                                (static_cast<float>(row) * (pixel_delta_v));
      auto ray_dir = pixel_center - camera_origin;
      ray r(camera_origin, ray_dir);

      const auto pixel_color = ray_color(r);
      const auto idx = (row * image_width + col) * channels;

      write_color(image, idx, pixel_color);
    }
  }

  // Write to PNG
  if (stbi_write_png("image.png", image_width, image_height, channels,
                     image.data(), image_width * channels)) {
    std::clog << "\rImage written to image.png\n";
  } else {
    std::cerr << "Error writing image\n";

    return EXIT_FAILURE;
  }

  std::clog << "\rDone!\n";

  return EXIT_SUCCESS;
}
