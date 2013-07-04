#ifndef _DATA_TYPES
#define _DATA_TYPES

#include <iostream>

struct dimension {
  dimension(void);
  dimension(unsigned int w, unsigned int h);
  unsigned int width, height;
};

struct position_t {
  position_t(void);
  position_t(int x, int y);
  int x, y;
};

struct rectangle {
  rectangle(void);
  rectangle(position_t position, dimension dim);
  rectangle(int x, int y, unsigned int width, unsigned int height);

  int & x(void);
  int const & x(void) const;
  int & y(void);
  int const & y(void) const;
  unsigned int & width(void);
  unsigned int const & width(void) const;
  unsigned int & height(void);
  unsigned int const & height(void) const;

  position_t position;
  dimension _dimension;
};

std::ostream & operator<<(std::ostream & os, const rectangle & rect);

#endif
