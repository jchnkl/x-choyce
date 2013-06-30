#ifndef _LAYOUT_T
#define _LAYOUT_T

#include "data_types.hpp"
#include "x_client_container.hpp"

class layout_t {
  public:
    virtual void arrange(const rectangle_t &, x_client_container &) const = 0;
};

#endif
