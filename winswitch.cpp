#include <climits>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <list>
#include <deque>
#include <vector>
#include <X11/keysym.h>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/damage.h>
#include <xcb/xinerama.h>
#include <xcb/composite.h>

#include "data_types.hpp"
#include "x_event_handler.hpp"
#include "x_connection.hpp"
#include "x_client.hpp"
#include "x_event_source.hpp"
#include "x_client_container.hpp"

// http://svn.enlightenment.org/svn/e/tags/evas-1.0.2/src/modules/engines/xrender_x11/evas_engine_xcb_render.c
#define DOUBLE_TO_FIXED(d) ((xcb_render_fixed_t) ((d) * 65536))

class layout_t {
  public:
    virtual void arrange(const rectangle_t &, x_client_container &) const = 0;
};

class grid_t : public layout_t {
  public:
    void arrange(const rectangle_t & screen, x_client_container & clients) const
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

  private:
    std::deque<int> decompose(int f, int n) const
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
};

class x_clients_preview : public x_event_handler {
  public:
    x_clients_preview(const x_connection & c,
                      const layout_t * layout,
                      x_client_container & x_clients)
      : _c(c), _layout(layout), _x_clients(x_clients) {}

    void handle(xcb_generic_event_t * ge)
    {
      if (XCB_KEY_PRESS == (ge->response_type & ~0x80)) {
        xcb_key_press_event_t * e = (xcb_key_press_event_t *)ge;
        xcb_keysym_t keysym = _c.keycode_to_keysym(e->detail);
        if (keysym == XK_Escape && e->state == 0) {
          _active = false;

        } else if (keysym == XK_Tab
            && (e->state == XCB_MOD_MASK_4
              || e->state == (XCB_MOD_MASK_4 | XCB_MOD_MASK_SHIFT))) {
          if (_active) {
            _current_client->update_preview(false);
            move_client(e->state == XCB_MOD_MASK_4);
            _current_client->update_preview(true);
          } else {
            _active = true;
            _c.grab_keyboard();
            _active_window = _c.net_active_window();
            _x_clients.update();

            _layout->arrange(_c.current_screen(), _x_clients);

            _current_client =
              std::find(_x_clients.begin(), _x_clients.end(), _active_window);
            move_client(e->state == XCB_MOD_MASK_4);

            for (auto & client : _x_clients) {
              client.show_preview(client == _current_client->window());
            }
          }
        }

      } else if (XCB_KEY_RELEASE == (ge->response_type & ~0x80)) {
        xcb_key_release_event_t * e = (xcb_key_release_event_t *)ge;
        xcb_keysym_t keysym = _c.keycode_to_keysym(e->detail);
        if (keysym == XK_Super_L) {
          for (auto & client : _x_clients) {
            client.hide_preview();
          }
          _c.request_change_active_window(_current_client->window());
          _active = false;
          _c.ungrab_keyboard();
        }
      }
    }

  private:
    bool _active = false;
    xcb_window_t _active_window;

    const x_connection & _c;
    const layout_t * _layout;
    x_client_container & _x_clients;
    x_client_container::iterator _current_client;

    void move_client(bool next)
    {
      if (next) {
        if (++_current_client == _x_clients.end()) {
          _current_client = _x_clients.begin();
        }
      } else {
        if (_current_client == _x_clients.begin()) {
          _current_client = _x_clients.end();
        }
        --_current_client;
      }
    }
};

int main(int argc, char ** argv)
{
  x_connection c;
  c.grab_key(XCB_MOD_MASK_4, XK_Tab);

  x_event_source es(c);
  x_client_container cc(c, es);

  grid_t grid;
  x_clients_preview cp(c, &grid, cc);

  es.register_handler(&c);
  es.register_handler(&cp);

  es.run_event_loop();

  return 0;
}
