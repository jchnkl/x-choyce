#include "x_client_chooser.hpp"

#include <algorithm>
#include <sstream>
#include <X11/keysym.h>

x_client_chooser::x_client_chooser(x_connection & c,
                                   x::xrm & xrm,
                                   chooser_t * chooser)
  : m_c(c), m_xrm(xrm), m_chooser(chooser)
{
  m_c.attach(0, XCB_KEY_PRESS, this);
  m_c.attach(0, XCB_KEY_RELEASE, this);
  m_c.attach(0, XCB_BUTTON_PRESS, this);
  m_c.attach(0, XCB_MOTION_NOTIFY, this);
  m_xrm.attach(this);
  load_config();
  m_modifier_map = m_c.modifier_mapping();
}

x_client_chooser::~x_client_chooser(void)
{
  m_c.detach(XCB_KEY_PRESS, this);
  m_c.detach(XCB_KEY_RELEASE, this);
  m_c.detach(XCB_BUTTON_PRESS, this);
  m_c.detach(XCB_MOTION_NOTIFY, this);
  m_xrm.detach(this);
}

bool
x_client_chooser::handle(xcb_generic_event_t * ge)
{
  bool result = false;

  if (XCB_KEY_PRESS == (ge->response_type & ~0x80)) {
    result = true;
    m_ignore_release = false;
    xcb_key_press_event_t * e = (xcb_key_press_event_t *)ge;

    if (e->detail == m_action_keycode
        && (e->state == m_action_modmask
          || e->state == (m_action_modmask | XCB_MOD_MASK_SHIFT))) {
      if (m_active) {
        if (e->state == m_action_modmask) {
          m_chooser->next();
        } else {
          m_chooser->prev();
        }

      } else {
        m_active = true;
        m_last_motion = XCB_NONE;
        m_c.grab_keyboard();
        m_c.grab_pointer(m_c.root_window(),
                        XCB_EVENT_MASK_POINTER_MOTION
                        | XCB_EVENT_MASK_BUTTON_PRESS);
        m_chooser->show();
        m_chooser->next();
      }

    } else if (e->detail == m_east_keycode) {
      m_chooser->east();

    } else if (e->detail == m_west_keycode) {
      m_chooser->west();

    } else if (e->detail == m_north_keycode) {
      m_chooser->north();

    } else if (e->detail == m_south_keycode) {
      m_chooser->south();

    } else if (e->detail == m_quit_keycode) {
      quit();

    } else if (e->detail == m_raise_keycode) {
      quit();
      m_chooser->raise();
    }

  } else if (XCB_KEY_RELEASE == (ge->response_type & ~0x80)) {
    result = true;
    if (m_ignore_release) return result;

    xcb_key_release_event_t * e = (xcb_key_release_event_t *)ge;
    for (auto & item : m_modifier_map) {
      if (item.first == XCB_MOD_MASK_SHIFT) continue;

      for (auto & keycode : item.second) {
        if (e->detail == keycode) {
          quit();
          m_chooser->select();
          break;
        }
      }
    }

  } else if (XCB_BUTTON_PRESS == (ge->response_type & ~0x80)) {
    result = true;
    xcb_button_press_event_t * e = (xcb_button_press_event_t *)ge;
    quit();
    m_chooser->select(e->child);

  } else if (XCB_MOTION_NOTIFY == (ge->response_type & ~0x80)) {
    result = true;
    xcb_motion_notify_event_t *e = (xcb_motion_notify_event_t *)ge;

    if (e->event == m_c.root_window()) {
      m_ignore_release = true;
    }

    if (m_last_motion != e->child) {
      m_last_motion = e->child;
      m_ignore_release = true;
      m_chooser->highlight(e->child);
    }

  }

  return result;
}

void
x_client_chooser::notify(x::xrm *)
{
  load_config();
}

void
x_client_chooser::quit(void)
{
  m_active = false;
  m_c.ungrab_pointer();
  m_c.ungrab_keyboard();
  m_chooser->hide();
}

void
x_client_chooser::load_config(void)
{
  m_c.ungrab_key(m_action_modmask, m_action_keysym);

  m_north_keycode =
    m_c.keysym_to_keycode(XStringToKeysym(m_xrm["north"].v.str->c_str()));
  m_south_keycode =
    m_c.keysym_to_keycode(XStringToKeysym(m_xrm["south"].v.str->c_str()));
  m_east_keycode =
    m_c.keysym_to_keycode(XStringToKeysym(m_xrm["east"].v.str->c_str()));
  m_west_keycode =
    m_c.keysym_to_keycode(XStringToKeysym(m_xrm["west"].v.str->c_str()));
  m_quit_keycode =
    m_c.keysym_to_keycode(XStringToKeysym(m_xrm["escape"].v.str->c_str()));
  m_raise_keycode =
    m_c.keysym_to_keycode(XStringToKeysym(m_xrm["raise"].v.str->c_str()));

  m_action_keysym = XStringToKeysym(m_xrm["action"].v.str->c_str());
  m_action_keycode = m_c.keysym_to_keycode(m_action_keysym);

  int mask = 0;
  std::stringstream ss(*m_xrm["mod"].v.str);
  std::string m;
  while (std::getline(ss, m, '+')) {

    m.erase(std::find_if_not(m.begin(), m.end(), ::isalnum), m.end());

         if ("mod1"    == m) mask |= XCB_MOD_MASK_1;
    else if ("mod2"    == m) mask |= XCB_MOD_MASK_2;
    else if ("mod3"    == m) mask |= XCB_MOD_MASK_3;
    else if ("mod4"    == m) mask |= XCB_MOD_MASK_4;
    else if ("mod5"    == m) mask |= XCB_MOD_MASK_5;
    else if ("control" == m) mask |= XCB_MOD_MASK_CONTROL;
  }

  m_action_modmask = (xcb_mod_mask_t)mask;

  m_c.grab_key(m_action_modmask, m_action_keysym);
}
