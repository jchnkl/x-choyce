#ifndef _LAYOUT_T
#define _LAYOUT_T

#include <deque>

#include "data_types.hpp"
#include "x_client_container.hpp"

class layout_t {
  public:
    virtual std::deque<rectangle_t>
    arrange(const rectangle_t &, x_client_container &) const = 0;
};

#endif
