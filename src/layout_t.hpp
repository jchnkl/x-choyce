#ifndef LAYOUT_T
#define LAYOUT_T

#include <deque>

#include "data_types.hpp"

class layout_t {
  public:
    virtual std::deque<rectangle>
      arrange(const rectangle &, unsigned int) const = 0;
};

#endif
