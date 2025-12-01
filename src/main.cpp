#include <iostream>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

auto main() -> int {
  // Image
  int image_width = 256;
  int image_height = 256;
  int channels = 3;

  // Create image buffer
  std::vector<unsigned char> image(image_width * image_height * channels);

  // Render
  for (auto row = 0; row < image_height; ++row) {
    std::clog << "\rScanlines remaining: " << (image_height - row)
              << ' ' << std::flush;
    for (auto col = 0; col < image_width; ++col) {
      auto r = double(col) / (image_width - 1);
      auto g = double(row) / (image_height - 1);
      auto b = 0.0;

      int ir = int(255.999 * r);
      int ig = int(255.999 * g);
      int ib = int(255.999 * b);

      int idx = (row * image_width + col) * channels;
      image[idx++] = ir;
      image[idx++] = ig;
      image[idx] = ib;
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
