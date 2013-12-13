#include <iostream>
#include <cstdlib> // EXIT_FAILURE

#define STRINGIFY_1(V) #V
#define STRINGIFY(V) STRINGIFY_1(V)

#ifdef SOURCE
#include STRINGIFY(SOURCE)
#endif

#define LINE_WIDTH 80

int main(int argc, char ** argv)
{
#ifndef SOURCE
  std::cerr << "SOURCE undefined. Compile with -DSOURCE=/path/to/image.c" << std::endl;
  return EXIT_FAILURE;
#else

  const uint32_t width = default_application_icon.width;
  const uint32_t height = default_application_icon.width;

  const size_t bytes_per_pixel = default_application_icon.bytes_per_pixel;
  const size_t size = width * height * bytes_per_pixel;

  uint8_t * image_data = new uint8_t[size];
  uint32_t * icon_data = (uint32_t *)default_application_icon.pixel_data;

  // swap red & blue: abgr -> argb
  for (int i = 0; i < width * height; ++i) {
    uint8_t b = 0xff & (icon_data[i] >> 16);
    uint8_t r = 0xff & (icon_data[i] >>  0);

    *(uint32_t *)&image_data[i * bytes_per_pixel] =
      (0xff00ff00 & icon_data[i]) | (r << 16) |  (b << 0);
  }

  std::printf("static const struct {\n"
              "  unsigned int 	 width;\n"
              "  unsigned int 	 height;\n"
              "  unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */\n"
              "  unsigned char	 pixel_data[%u * %u * %u + 1];\n"
              "} default_application_icon = {\n"
              "  %u, %u, %u,\n",
              width, height, bytes_per_pixel,
              width, height, bytes_per_pixel);


  size_t n = 2;
  n += std::printf("  { 0x%02x", image_data[0]);

  for (size_t i = 1; i < size; ++i) {
    // ", 0x00" ^= 6 chars
    if (n + 6 > LINE_WIDTH) {
      n = 4;
      std::printf("\n  ");
    }
    n += std::printf(", 0x%02x", image_data[i]);
  }

  std::cout << "\n  }\n};" << std::endl;

  return EXIT_SUCCESS;

#endif
}
