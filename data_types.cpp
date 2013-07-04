#include "data_types.hpp"

dimension::dimension(void) {}
dimension::dimension(unsigned int w, unsigned int h)
  : width(w), height(h) {}

position::position(void) {}
position::position(int x, int y) : x(x), y(y) {}

rectangle::rectangle(void) {}

rectangle::rectangle(position pos, dimension dim)
  : _position(pos), _dimension(dim) {}

rectangle::rectangle(int x, int y, unsigned int width, unsigned int height)
  : _position(x, y), _dimension(width, height) {}

int &
rectangle::x(void) { return _position.x; }

int const &
rectangle::x(void) const { return _position.x; }

int &
rectangle::y(void) { return _position.y; }

int const &
rectangle::y(void) const { return _position.y; }

unsigned int &
rectangle::width(void) { return _dimension.width; }

unsigned int const &
rectangle::width(void) const { return _dimension.width; }

unsigned int &
rectangle::height(void) { return _dimension.height; }

unsigned int const &
rectangle::height(void) const { return _dimension.height; }

std::ostream & operator<<(std::ostream & os, const rectangle & rect)
{
  return os << rect.x() << "x" << rect.y() << "+"
            << rect.width() << "+" << rect.height();
}
