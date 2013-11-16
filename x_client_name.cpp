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
  xcb_free_pixmap(_c(), _title);
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

const xcb_pixmap_t &
x_client_name::title(void) const
{
  return _title;
}

bool
x_client_name::handle(xcb_generic_event_t * ge)
{
  if (XCB_PROPERTY_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_property_notify_event_t * e = (xcb_property_notify_event_t *)ge;

    if (e->window != _x_client->window()) return true;

    bool update_title = false;
    if (e->atom == _a_wm_name) {
      update_title = true;
      update_wm_name();

    } else if (e->atom == _a_wm_class) {
      update_title = true;
      update_wm_class();

    } else if (e->atom == _a_net_wm_name) {
      update_title = true;
      update_net_wm_name();

    }

    if (update_title) {
      make_title();
      observable::notify();
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

void
x_client_name::make_title(void)
{
  xcb_free_pixmap(_c(), _title);

  _title = xcb_generate_id(_c());
  xcb_create_pixmap(_c(), 32, _title, _c.root_window(),
                    _title_width, _title_height);

  xcb_gcontext_t gc = xcb_generate_id(_c());
  xcb_create_gc(_c(), gc, _title, XCB_GC_FOREGROUND, &_title_bg_color );
  xcb_rectangle_t r = { 0, 0, (uint16_t)_title_width, (uint16_t)_title_height };
  xcb_poly_fill_rectangle(_c(), _title, gc, 1, &r);
  xcb_free_gc(_c(), gc);

  _x_xft = std::shared_ptr<x::xft>(new x::xft(_c.dpy(), _title, 32));

  std::string pname = _class_name;
  std::string title = _net_wm_name.empty() ? _wm_name : _net_wm_name;

  std::string lower = "abcdefghijklmnopqrstuvwxy";
  std::string upper = "ABCDEFGHIJKLMNOPQRSTUVWXY";

  XGlyphInfo text_extents = _x_xft->text_extents_utf8(_pnamefont, lower + upper);

  int x_off = _icon_size + 3 * _border_width;
  int y_off = _border_width;

  y_off += text_extents.height;
  _x_xft->draw_string_utf8(pname, x_off, y_off, _pnamefont, _colorname);

  y_off += text_extents.height;
  _x_xft->draw_string_utf8(title, x_off, y_off, _titlefont, _colorname);
}
