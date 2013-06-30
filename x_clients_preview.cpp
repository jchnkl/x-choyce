#include "x_clients_preview.hpp"

#include <algorithm>
#include <X11/keysym.h>

x_clients_preview::x_clients_preview(const x_connection & c,
                                     const layout_t * layout,
                                     x_client_container & x_clients)
: _c(c), _layout(layout), _x_clients(x_clients) {}


void x_clients_preview::handle(xcb_generic_event_t * ge)
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

void x_clients_preview::move_client(bool next)
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
