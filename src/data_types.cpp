#include "data_types.hpp"

dimension::dimension(void) {}
dimension::dimension(unsigned int w, unsigned int h)
  : width(w), height(h) {}

position::position(void) {}
position::position(int x, int y) : x(x), y(y) {}

rectangle::rectangle(void) {}

rectangle::rectangle(position pos, dimension dim)
  : m_position(pos), m_dimension(dim) {}

rectangle::rectangle(int x, int y, unsigned int width, unsigned int height)
  : m_position(x, y), m_dimension(width, height) {}

int &
rectangle::x(void) { return m_position.x; }

int const &
rectangle::x(void) const { return m_position.x; }

int &
rectangle::y(void) { return m_position.y; }

int const &
rectangle::y(void) const { return m_position.y; }

unsigned int &
rectangle::width(void) { return m_dimension.width; }

unsigned int const &
rectangle::width(void) const { return m_dimension.width; }

unsigned int &
rectangle::height(void) { return m_dimension.height; }

unsigned int const &
rectangle::height(void) const { return m_dimension.height; }

bool
operator==(const rectangle & r1, const rectangle & r2)
{
  return r1.m_position.x == r2.m_position.x
    && r1.m_position.y == r2.m_position.y
    && r1.m_dimension.width == r2.m_dimension.width
    && r1.m_dimension.height == r2.m_dimension.height;
}

std::ostream & operator<<(std::ostream & os, const rectangle & rect)
{
  return os << rect.x() << "x" << rect.y() << "+"
            << rect.width() << "+" << rect.height();
}
