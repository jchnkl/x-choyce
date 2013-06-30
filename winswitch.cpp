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

xcb_render_pictformat_t
render_find_visual_format(const x_connection & c, xcb_visualid_t visual);

xcb_render_picture_t
make_picture(const x_connection & c, xcb_window_t window);

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

xcb_render_pictformat_t
render_find_visual_format(const x_connection & c, xcb_visualid_t visual)
{
  // http://lists.freedesktop.org/archives/xcb/2004-December/000236.html

  xcb_render_query_pict_formats_reply_t * query_pict_formats_reply =
    xcb_render_query_pict_formats_reply(
        c(), xcb_render_query_pict_formats(c()), NULL);

  xcb_render_pictscreen_iterator_t pictscreen_iterator =
    xcb_render_query_pict_formats_screens_iterator(query_pict_formats_reply);

  while (pictscreen_iterator.rem) {
    xcb_render_pictdepth_iterator_t pictdepth_iterator =
      xcb_render_pictscreen_depths_iterator(pictscreen_iterator.data);

    while (pictdepth_iterator.rem) {
      xcb_render_pictvisual_iterator_t pictvisual_iterator =
        xcb_render_pictdepth_visuals_iterator(pictdepth_iterator.data);

      while (pictvisual_iterator.rem) {
        if (pictvisual_iterator.data->visual == visual) {
          delete query_pict_formats_reply;
          return pictvisual_iterator.data->format;
        }
        xcb_render_pictvisual_next(&pictvisual_iterator);
      }
      xcb_render_pictdepth_next(&pictdepth_iterator);
    }
    xcb_render_pictscreen_next(&pictscreen_iterator);
  }

  delete query_pict_formats_reply;
  return 0;
}

xcb_render_picture_t
make_picture(const x_connection & c, xcb_window_t window)
{
  xcb_get_window_attributes_reply_t * window_attributes_reply =
    xcb_get_window_attributes_reply(c(),
                                    xcb_get_window_attributes(c(), window),
                                    NULL);

  if (window_attributes_reply) {
    xcb_render_pictformat_t format =
      render_find_visual_format(c, window_attributes_reply->visual);

    delete window_attributes_reply;

    uint32_t mask = XCB_RENDER_CP_REPEAT | XCB_RENDER_CP_SUBWINDOW_MODE;
    uint32_t list[] = { XCB_RENDER_REPEAT_NONE,
                        XCB_SUBWINDOW_MODE_INCLUDE_INFERIORS };

    xcb_render_picture_t picture = xcb_generate_id(c());
    xcb_render_create_picture(c(), picture, window, format, mask, list);

    return picture;
  }

  return XCB_NONE;
}

std::list<x_client>
make_x_clients(const x_connection & c, const std::vector<xcb_window_t> & windows)
{
  std::list<x_client> x_clients;
  for (auto & window : windows) { x_clients.emplace_back(c, window); }
  return x_clients;
}

xcb_visualtype_t *
argb_visual(const x_connection & c)
{
  xcb_depth_iterator_t depth_iter =
    xcb_screen_allowed_depths_iterator(c.default_screen());

  if (depth_iter.data) {
    for (; depth_iter.rem; xcb_depth_next (&depth_iter)) {
      if (depth_iter.data->depth == 32) {
        xcb_visualtype_iterator_t visual_iter = xcb_depth_visuals_iterator(depth_iter.data);
        for (; visual_iter.rem; xcb_visualtype_next(&visual_iter)) {
          return visual_iter.data;
        }
      }
    }
  }

  return NULL;
}

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
