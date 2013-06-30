#ifndef _DATA_TYPES
#define _DATA_TYPES

#include <iostream>

struct dimension_t {
  dimension_t(void);
  dimension_t(unsigned int w, unsigned int h);
  unsigned int width, height;
};

struct position_t {
  position_t(void);
  position_t(int x, int y);
  int x, y;
};

struct rectangle_t {
  rectangle_t(void);
  rectangle_t(position_t position, dimension_t dimension);
  rectangle_t(int x, int y, unsigned int width, unsigned int height);

  int & x(void);
  int const & x(void) const;
  int & y(void);
  int const & y(void) const;
  unsigned int & width(void);
  unsigned int const & width(void) const;
  unsigned int & height(void);
  unsigned int const & height(void) const;

  position_t position;
  dimension_t dimension;
};

std::ostream & operator<<(std::ostream & os, const rectangle_t & rectangle);

#endif
