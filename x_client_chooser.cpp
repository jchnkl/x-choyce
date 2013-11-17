#include "x_client_chooser.hpp"

#include <algorithm>
#include <X11/keysym.h>

x_client_chooser::x_client_chooser(x_connection & c,
                                   chooser_t * chooser,
                                   xcb_keysym_t east_keysym,
                                   xcb_keysym_t west_keysym,
                                   xcb_keysym_t north_keysym,
                                   xcb_keysym_t south_keysym,
                                   xcb_keysym_t quit_keysym,
                                   xcb_keysym_t action_keysym,
                                   xcb_mod_mask_t action_modmask)
  : _c(c), _chooser(chooser)
  , _action_modmask(action_modmask)
{
  _c.attach(XCB_KEY_PRESS, this);
  _c.attach(XCB_KEY_RELEASE, this);
  _c.attach(XCB_BUTTON_PRESS, this);
  _c.attach(XCB_MOTION_NOTIFY, this);
  _c.grab_key(_action_modmask, action_keysym);
  _east_keycode = _c.keysym_to_keycode(east_keysym);
  _west_keycode = _c.keysym_to_keycode(west_keysym);
  _north_keycode = _c.keysym_to_keycode(north_keysym);
  _south_keycode = _c.keysym_to_keycode(south_keysym);
  _quit_keycode = _c.keysym_to_keycode(quit_keysym);
  _action_keycode = _c.keysym_to_keycode(action_keysym);
  _modifier_map = _c.modifier_mapping();
}

x_client_chooser::~x_client_chooser(void)
{
  _c.detach(XCB_KEY_PRESS, this);
  _c.detach(XCB_KEY_RELEASE, this);
  _c.detach(XCB_BUTTON_PRESS, this);
  _c.detach(XCB_MOTION_NOTIFY, this);
}

bool
x_client_chooser::handle(xcb_generic_event_t * ge)
{
  bool result = false;

  if (XCB_KEY_PRESS == (ge->response_type & ~0x80)) {
    result = true;
    _ignore_release = false;
    xcb_key_press_event_t * e = (xcb_key_press_event_t *)ge;

    if (e->detail == _action_keycode
        && (e->state == _action_modmask
          || e->state == (_action_modmask | XCB_MOD_MASK_SHIFT))) {
      if (_active) {
        if (e->state == _action_modmask) {
          _chooser->next();
        } else {
          _chooser->prev();
        }

      } else {
        _active = true;
        _last_motion = XCB_NONE;
        _c.grab_keyboard();
        _c.grab_pointer(_c.root_window(),
                        XCB_EVENT_MASK_POINTER_MOTION
                        | XCB_EVENT_MASK_BUTTON_PRESS);
        _chooser->show();
        _chooser->next();
      }

    } else if (e->detail == _east_keycode) {
      _chooser->east();

    } else if (e->detail == _west_keycode) {
      _chooser->west();

    } else if (e->detail == _north_keycode) {
      _chooser->north();

    } else if (e->detail == _south_keycode) {
      _chooser->south();

    } else if (e->detail == _quit_keycode) {
      quit();
    }

  } else if (XCB_KEY_RELEASE == (ge->response_type & ~0x80)) {
    result = true;
    if (_ignore_release) return result;

    xcb_key_release_event_t * e = (xcb_key_release_event_t *)ge;
    for (auto & keycode : _modifier_map[_action_modmask]) {
      if (e->detail == keycode) {
        quit();
        _chooser->select();
      }
      break;
    }

  } else if (XCB_BUTTON_PRESS == (ge->response_type & ~0x80)) {
    result = true;
    xcb_button_press_event_t * e = (xcb_button_press_event_t *)ge;
    quit();
    _chooser->select(e->child);

  } else if (XCB_MOTION_NOTIFY == (ge->response_type & ~0x80)) {
    result = true;
    xcb_motion_notify_event_t *e = (xcb_motion_notify_event_t *)ge;

    if (e->event == _c.root_window()) {
      _ignore_release = true;
    }

    if (_last_motion != e->child) {
      _last_motion = e->child;
      _ignore_release = true;
      _chooser->highlight(e->child);
    }

  }

  return result;
}

void
x_client_chooser::quit(void)
{
  _active = false;
  _c.ungrab_pointer();
  _c.ungrab_keyboard();
  _chooser->hide();
}
