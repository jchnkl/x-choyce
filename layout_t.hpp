#ifndef _LAYOUT_T
#define _LAYOUT_T

#include <deque>

#include "data_types.hpp"

class layout_t {
  public:
    virtual std::deque<rectangle>
      arrange(const rectangle &, unsigned int) const = 0;
};

#endif
