#ifndef COLOR_H
#define COLOR_H

#include <glm/glm.hpp>
#include <iostream>

using color = glm::vec3;

/// <summary>
/// Write a color to an output stream in PPM format.
/// </summary>
/// <param name="image">The image vector to write to.</param>
/// <param name="idx">The starting index in the image vector to write the color.</param>
/// <param name="pixel_color">The color to write (expects RGB components in [0,1] range using color.r, color.g, and color.b members).</param>
auto write_color(std::vector<unsigned char>& image, int idx,
                 const color& pixel_color) -> void
{
    auto r = pixel_color.r;
    auto g = pixel_color.g;
    auto b = pixel_color.b;

    // Translate the [0,1] component values to the byte range [0,255].
    int rbyte = int(255.999 * r);
    int gbyte = int(255.999 * g);
    int bbyte = int(255.999 * b);

    image[idx++] = static_cast<unsigned char>(rbyte);
    image[idx++] = static_cast<unsigned char>(gbyte);
    image[idx] = static_cast<unsigned char>(bbyte);
}

#endif
