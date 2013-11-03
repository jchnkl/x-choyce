#include "x_client_chooser.hpp"

#include <algorithm>
#include <X11/keysym.h>

x_client_chooser::x_client_chooser(x_connection & c,
                                   chooser_t * chooser,
                                   xcb_keysym_t action_keysym,
                                   xcb_mod_mask_t action_modmask)
  : _c(c), _chooser(chooser)
  , _action_modmask(action_modmask)
{
  _c.register_handler(XCB_KEY_PRESS, this);
  _c.register_handler(XCB_KEY_RELEASE, this);
  _c.register_handler(XCB_BUTTON_PRESS, this);
  _c.register_handler(XCB_MOTION_NOTIFY, this);
  _c.grab_key(_action_modmask, action_keysym);
  _action_keycode = _c.keysym_to_keycode(action_keysym);
  _modifier_map = _c.modifier_mapping();
}

x_client_chooser::~x_client_chooser(void)
{
  _c.deregister_handler(XCB_KEY_PRESS, this);
  _c.deregister_handler(XCB_KEY_RELEASE, this);
  _c.deregister_handler(XCB_BUTTON_PRESS, this);
  _c.deregister_handler(XCB_MOTION_NOTIFY, this);
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
