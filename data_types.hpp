#ifndef _DATA_TYPES
#define _DATA_TYPES

#include <iostream>

struct dimension_t {
  dimension_t(void) {}
  dimension_t(unsigned int w, unsigned int h) : width(w), height(h) {}
  unsigned int width, height;
};

struct position_t {
  position_t(void) {}
  position_t(int x, int y) : x(x), y(y) {}
  int x, y;
};

struct rectangle_t {
  rectangle_t(void) {}
  rectangle_t(position_t position, dimension_t dimension)
    : position(position), dimension(dimension) {}
  rectangle_t(int x, int y, unsigned int width, unsigned int height)
    : position(x, y), dimension(width, height) {}

  int & x(void) { return position.x; }
  int const & x(void) const { return position.x; }
  int & y(void) { return position.y; }
  int const & y(void) const { return position.y; }
  unsigned int & width(void) { return dimension.width; }
  unsigned int const & width(void) const { return dimension.width; }
  unsigned int & height(void) { return dimension.height; }
  unsigned int const & height(void) const { return dimension.height; }

  position_t position;
  dimension_t dimension;
};

std::ostream & operator<<(std::ostream & os, const rectangle_t & rectangle)
{
  return os << rectangle.x() << "x" << rectangle.y() << "+"
            << rectangle.width() << "+" << rectangle.height();
}

#endif
