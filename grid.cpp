#include "grid.hpp"

#include <cmath>

std::deque<rectangle_t>
grid_t::arrange(const rectangle_t & screen, unsigned int nrects) const
{
  int gap = 5;

  int radix = std::round(std::sqrt(nrects));
  int rest = (radix * radix) - nrects;

  auto cells = decompose(radix, radix * radix);

  if (rest >= 0) {
    for (auto rit = cells.rbegin(); rit != cells.rend(); ) {
      if (rest == 0) {
        break;
      } else if (rest < *rit) {
        *rit -= rest;
        break;
      } else if (rest >= *rit) {
        rest -= *rit;
        ++rit;
        cells.pop_back();
      }
    }
  } else {
    cells.push_back((-1) * rest);
  }

  int ncol = cells.size();
  int colw = screen.width() / ncol;

  std::deque<rectangle_t> rects;
  for (int c = 0; c < ncol; ++c) {
    int nrow = cells[c];
    int rowh = screen.height() / nrow;
    for (int r = 0; r < nrow; ++r) {
      rects.push_back(rectangle_t(c * colw + screen.x() + gap,
                                  r * rowh + screen.y() + gap,
                                  colw - 2 * gap, rowh - 2 * gap));
    }
  }

  return rects;
}

std::deque<int>
grid_t::decompose(int f, int n) const
{
  std::deque<int> result;
  while (true) {
    n -= f;
    if (n > 0) {
      result.push_back(f);
    } else {
      result.push_back(n + f);
      break;
    }
  }
  return result;
}
