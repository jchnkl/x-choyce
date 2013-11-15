#include "x_client_name.hpp"

#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>

x_client_name::x_client_name(x_connection & c, x_client * const x_client)
  : _c(c), _x_client(x_client)
{
  _c.register_handler(XCB_PROPERTY_NOTIFY, this);
  _c.update_input(_x_client->window(), XCB_EVENT_MASK_PROPERTY_CHANGE);

  update_wm_name();
  update_wm_class();
  update_net_wm_name();
}

x_client_name::~x_client_name(void)
{
  _c.deregister_handler(XCB_PROPERTY_NOTIFY, this);
}

const std::string & x_client_name::net_wm_name(void) const
{
  return _net_wm_name;
}

const std::string & x_client_name::wm_name(void) const
{
  return _wm_name;
}

const std::string & x_client_name::wm_class_name(void) const
{
  return _class_name;
}

const std::string & x_client_name::wm_instance_name(void) const
{
  return _instance_name;
}

bool
x_client_name::handle(xcb_generic_event_t * ge)
{
  if (XCB_PROPERTY_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_property_notify_event_t * e = (xcb_property_notify_event_t *)ge;

    if (e->window != _x_client->window()) return true;

    if (e->atom == _a_wm_name) {
      update_wm_name();

    } else if (e->atom == _a_wm_class) {
      update_wm_class();

    } else if (e->atom == _a_net_wm_name) {
      update_net_wm_name();

    }

    return true;
  }

  return false;
}
// private

void
x_client_name::update_net_wm_name(void)
{
  _net_wm_name.clear();

  xcb_generic_error_t * error;
  xcb_get_property_cookie_t c =
    xcb_ewmh_get_wm_name(_c.ewmh(), _x_client->window());
  xcb_get_property_reply_t * r = xcb_get_property_reply(_c(), c, &error);

  if (error) {
    delete error;

  } else {
    xcb_ewmh_get_utf8_strings_reply_t net_wm_name;
    if (xcb_ewmh_get_wm_name_from_reply(_c.ewmh(), &net_wm_name, r)) {
      _net_wm_name = std::string(net_wm_name.strings, net_wm_name.strings_len);
      xcb_ewmh_get_utf8_strings_reply_wipe(&net_wm_name);
    }
  }
}

void
x_client_name::update_wm_name(void)
{
  _wm_name.clear();

  xcb_generic_error_t * error;
  xcb_icccm_get_text_property_reply_t wm_name;
  xcb_get_property_cookie_t c =
    xcb_icccm_get_wm_name(_c(), _x_client->window());
  xcb_icccm_get_wm_name_reply(_c(), c, &wm_name, &error);

  if (error) {
    delete error;

  } else {
    _wm_name = std::string(wm_name.name, wm_name.name_len);
    xcb_icccm_get_text_property_reply_wipe(&wm_name);
  }
}

void
x_client_name::update_wm_class(void)
{
  _class_name.clear();
  _instance_name.clear();

  xcb_generic_error_t * error;
  xcb_get_property_cookie_t c =
    xcb_icccm_get_wm_class(_c(), _x_client->window());
  xcb_get_property_reply_t * r = xcb_get_property_reply(_c(), c, &error);

  if (error) {
    delete error;

  } else {
    xcb_icccm_get_wm_class_reply_t wm_class;
    if (xcb_icccm_get_wm_class_from_reply(&wm_class, r)) {
      _class_name = wm_class.class_name;
      _instance_name = wm_class.instance_name;
      xcb_icccm_get_wm_class_reply_wipe(&wm_class);
    }
  }
}
