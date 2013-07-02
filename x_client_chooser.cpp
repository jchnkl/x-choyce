#include "x_client_chooser.hpp"

#include <algorithm>
#include <X11/keysym.h>

x_client_chooser::x_client_chooser(const x_connection & c,
                                   const layout_t * layout,
                                   x_client_container & x_clients,
                                   xcb_keysym_t action_keysym,
                                   xcb_mod_mask_t action_modmask)
  : _c(c), _layout(layout), _x_clients(x_clients)
  , _action_modmask(action_modmask)
{
  _c.grab_key(_action_modmask, action_keysym);
  _action_keycode = _c.keysym_to_keycode(action_keysym);
  _modifier_map = _c.modifier_mapping();
}


void x_client_chooser::handle(xcb_generic_event_t * ge)
{
  if (XCB_KEY_PRESS == (ge->response_type & ~0x80)) {
    xcb_key_press_event_t * e = (xcb_key_press_event_t *)ge;
    xcb_keysym_t keysym = _c.keycode_to_keysym(e->detail);
    if (keysym == XK_Escape && e->state == 0) {
      _active = false;

    } else if (keysym == _action_keycode
        && (e->state == _action_modmask
          || e->state == (_action_modmask | XCB_MOD_MASK_SHIFT))) {
      if (_active) {
        _current_client->update_preview(false);
        move_client(e->state == _action_modmask);
        _current_client->update_preview(true);

      } else {
        _x_clients.update();
        if (_x_clients.size() == 0) { return; }

        _active = true;
        _c.grab_keyboard();
        _active_window = _c.net_active_window();

        configure_clients_preview();

        _current_client =
          std::find(_x_clients.begin(), _x_clients.end(), _active_window);
        move_client(e->state == _action_modmask);

        for (auto & client : _x_clients) {
          client.show_preview(client == _current_client->window());
        }
      }
    }

  } else if (XCB_KEY_RELEASE == (ge->response_type & ~0x80)) {
    xcb_key_release_event_t * e = (xcb_key_release_event_t *)ge;
    for (auto & keycode : _modifier_map[_action_modmask]) {
      if (e->detail == keycode) {
        for (auto & client : _x_clients) {
          client.hide_preview();
        }
        _c.request_change_active_window(_current_client->window());
        _active = false;
        _c.ungrab_keyboard();
      }
      break;
    }
  }
}

void x_client_chooser::move_client(bool next)
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

void
x_client_chooser::configure_clients_preview(void)
{
  int i = 0;
  auto rects = _layout->arrange(_c.current_screen(), _x_clients.size());

  for (auto & client : _x_clients) {
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
