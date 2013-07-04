#include "grid.hpp"

#include <cmath>
#include <vector>

std::deque<rectangle>
grid_t::arrange(const rectangle & screen, unsigned int nrects) const
{
  int gap = 5;

  int radix = std::round(std::sqrt(nrects));
  int rest = nrects - (radix * radix);

  std::vector<int> cells(radix, radix);

  if (rest < 0 || (rest > 0 && rest < radix / 2.0)) {
    cells.back() += rest;
  } else if (rest > 0) {
    cells.push_back(rest);
  }

  int ncol = cells.size();
  int colw = screen.width() / ncol;

  std::deque<rectangle> rects;
  for (int c = 0; c < ncol; ++c) {
    int nrow = cells[c];
    int rowh = screen.height() / nrow;
    for (int r = 0; r < nrow; ++r) {
      rects.push_back(rectangle(c * colw + screen.x() + gap,
                                  r * rowh + screen.y() + gap,
                                  colw - 2 * gap, rowh - 2 * gap));
    }
  }

  return rects;
}
