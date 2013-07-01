#include "data_types.hpp"

dimension_t::dimension_t(void) {}
dimension_t::dimension_t(unsigned int w, unsigned int h)
  : width(w), height(h) {}

position_t::position_t(void) {}
position_t::position_t(int x, int y) : x(x), y(y) {}

rectangle_t::rectangle_t(void) {}

rectangle_t::rectangle_t(position_t position, dimension_t dimension)
  : position(position), dimension(dimension) {}

rectangle_t::rectangle_t(int x, int y, unsigned int width, unsigned int height)
  : position(x, y), dimension(width, height) {}

int &
rectangle_t::x(void) { return position.x; }

int const &
rectangle_t::x(void) const { return position.x; }

int &
rectangle_t::y(void) { return position.y; }

int const &
rectangle_t::y(void) const { return position.y; }

unsigned int &
rectangle_t::width(void) { return dimension.width; }

unsigned int const &
rectangle_t::width(void) const { return dimension.width; }

unsigned int &
rectangle_t::height(void) { return dimension.height; }

unsigned int const &
rectangle_t::height(void) const { return dimension.height; }

std::ostream & operator<<(std::ostream & os, const rectangle_t & rectangle)
{
  return os << rectangle.x() << "x" << rectangle.y() << "+"
            << rectangle.width() << "+" << rectangle.height();
}
