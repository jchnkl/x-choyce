#ifndef _LAYOUT_T
#define _LAYOUT_T

#include <deque>

#include "data_types.hpp"
#include "x_client_container.hpp"

class layout_t {
  public:
    virtual std::deque<rectangle_t>
    arrange(const rectangle_t &, unsigned int) const = 0;
};

#endif
