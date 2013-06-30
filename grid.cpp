#include "grid.hpp"

#include <cmath>

void
grid_t::arrange(const rectangle_t & screen, x_client_container & clients) const
{
  int gap = 5;

  int factor = std::round(std::sqrt(clients.size()));
  int rest = (factor * factor) - clients.size();

  auto cells = decompose(factor, factor * factor);

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

  std::vector<rectangle_t> rects;
  for (int c = 0; c < ncol; ++c) {
    int nrow = cells[c];
    int rowh = screen.height() / nrow;
    for (int r = 0; r < nrow; ++r) {
      rects.push_back(rectangle_t(c * colw + screen.x() + gap,
                                  r * rowh + screen.y() + gap,
                                  colw - 2 * gap, rowh - 2 * gap));
    }
  }

  int i = 0;
  for (auto & client : clients) {
    double scale_x = (double)rects[i].width()
                   / (double)client.rectangle().width();
    double scale_y = (double)rects[i].height()
                   / (double)client.rectangle().height();
    client.preview_scale() = std::min(scale_x, scale_y);
    client.preview_position().x = rects[i].x();
    client.preview_position().y = rects[i].y();

    unsigned int realwidth  = client.rectangle().width()
                            * client.preview_scale();
    unsigned int realheight = client.rectangle().height()
                            * client.preview_scale();

    if (realwidth < rects[i].width()) {
      client.preview_position().x += (rects[i].width() - realwidth) / 2;
    }
    if (realheight < rects[i].height()) {
      client.preview_position().y += (rects[i].height() - realheight) / 2;
    }
    i++;
  }
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
