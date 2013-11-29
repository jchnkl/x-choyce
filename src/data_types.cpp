#include "data_types.hpp"

dimension::dimension(void) {}
dimension::dimension(unsigned int w, unsigned int h)
  : width(w), height(h) {}

position::position(void) {}
position::position(int x, int y) : x(x), y(y) {}

rectangle::rectangle(void) {}

rectangle::rectangle(position pos, dimension dim)
  : m_position(pos), _dimension(dim) {}

rectangle::rectangle(int x, int y, unsigned int width, unsigned int height)
  : m_position(x, y), _dimension(width, height) {}

int &
rectangle::x(void) { return m_position.x; }

int const &
rectangle::x(void) const { return m_position.x; }

int &
rectangle::y(void) { return m_position.y; }

int const &
rectangle::y(void) const { return m_position.y; }

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
