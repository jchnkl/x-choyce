#ifndef _GRID
#define _GRID

#include <deque>

#include "layout_t.hpp"

class grid_t : public layout_t {
  public:
    void arrange(const rectangle_t & screen, x_client_container & clients) const;

  private:
    std::deque<int> decompose(int f, int n) const;
};

#endif
