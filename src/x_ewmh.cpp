#include "x_ewmh.hpp"

x_ewmh::x_ewmh(x_connection & c) : _c(c)
{
  _c.attach(10, XCB_PROPERTY_NOTIFY, this);
  update_net_active_window();

  xcb_intern_atom_cookie_t * cookie = xcb_ewmh_init_atoms(_c(), &_ewmh);
  xcb_ewmh_init_atoms_replies(&_ewmh, cookie, NULL);
}

x_ewmh::~x_ewmh(void)
{
  _c.detach(XCB_PROPERTY_NOTIFY, this);
  xcb_ewmh_connection_wipe(&_ewmh);
}

xcb_window_t x_ewmh::net_active_window(void) const
{
  return _net_active_window;
}

xcb_ewmh_connection_t *
x_ewmh::connection(void) const
{
  return const_cast<xcb_ewmh_connection_t *>(&_ewmh);
}

bool
x_ewmh::handle(xcb_generic_event_t * ge)
{
  if (XCB_PROPERTY_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_property_notify_event_t * e = (xcb_property_notify_event_t *)ge;
    if (e->window == _c.root_window()
        && e->atom == _c.intern_atom("_NET_ACTIVE_WINDOW")) {
      update_net_active_window();
    }
    return true;
  }

  return false;
}

void x_ewmh::update_net_active_window(void)
{
  xcb_atom_t atom = _c.intern_atom("_NET_ACTIVE_WINDOW");
  if (atom == XCB_ATOM_NONE) {
    _net_active_window = XCB_NONE;
    return;
  }

  xcb_get_property_cookie_t property_cookie =
    xcb_get_property(_c(), false, _c.root_window(), atom, XCB_ATOM_WINDOW, 0, 32);

  xcb_generic_error_t * error = NULL;
  xcb_get_property_reply_t * property_reply =
    xcb_get_property_reply(_c(), property_cookie, &error);

  if (error || property_reply->value_len == 0) {
    delete error;
    _net_active_window = XCB_NONE;
  } else {
    _net_active_window = *(xcb_window_t *)xcb_get_property_value(property_reply);
  }

  delete property_reply;
}
