#include "data_types.hpp"

dimension_t::dimension_t(void) {}
dimension_t::dimension_t(unsigned int w, unsigned int h)
  : width(w), height(h) {}

position_t::position_t(void) {}
position_t::position_t(int x, int y) : x(x), y(y) {}

rectangle::rectangle(void) {}

rectangle::rectangle(position_t position, dimension_t dimension)
  : position(position), dimension(dimension) {}

rectangle::rectangle(int x, int y, unsigned int width, unsigned int height)
  : position(x, y), dimension(width, height) {}

int &
rectangle::x(void) { return position.x; }

int const &
rectangle::x(void) const { return position.x; }

int &
rectangle::y(void) { return position.y; }

int const &
rectangle::y(void) const { return position.y; }

unsigned int &
rectangle::width(void) { return dimension.width; }

unsigned int const &
rectangle::width(void) const { return dimension.width; }

unsigned int &
rectangle::height(void) { return dimension.height; }

unsigned int const &
rectangle::height(void) const { return dimension.height; }

std::ostream & operator<<(std::ostream & os, const rectangle & rect)
{
  return os << rect.x() << "x" << rect.y() << "+"
            << rect.width() << "+" << rect.height();
}
