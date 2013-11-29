#ifndef _GRID
#define _GRID

#include <deque>

#include "layout_t.hpp"

class grid_t : public layout_t {
  public:
    std::deque<rectangle>
      arrange(const rectangle & screen, unsigned int nrects) const;
};

#endif
