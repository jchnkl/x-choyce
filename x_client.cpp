#include "x_client.hpp"

x_client::x_client(x_connection & c, const xcb_window_t & window)
  : _c(c), _window(window)
{
  _c.attach(0, XCB_MAP_NOTIFY, this);
  _c.attach(0, XCB_UNMAP_NOTIFY, this);
  _c.attach(0, XCB_CONFIGURE_NOTIFY, this);
  _c.attach(0, XCB_PROPERTY_NOTIFY, this);
  _c.update_input(_window, XCB_EVENT_MASK_STRUCTURE_NOTIFY
                           | XCB_EVENT_MASK_PROPERTY_CHANGE);
  update_geometry();
  update_net_wm_desktop();
  update_parent_window();
  update_name_window_pixmap();
}

x_client::~x_client(void)
{
  _c.detach(XCB_MAP_NOTIFY, this);
  _c.detach(XCB_UNMAP_NOTIFY, this);
  _c.detach(XCB_CONFIGURE_NOTIFY, this);
  _c.detach(XCB_PROPERTY_NOTIFY, this);

  xcb_free_pixmap(_c(), _name_window_pixmap);
  xcb_free_pixmap(_c(), _name_window_dummy);
}

const    rectangle & x_client::rect(void)   const { return _rectangle; }
const xcb_window_t & x_client::window(void) const { return _window; }
const xcb_window_t & x_client::parent(void) const { return _parent; }

const xcb_window_t &
x_client::name_window_pixmap(void) const
{
  return _name_window_pixmap == XCB_NONE
         ? _name_window_dummy
         : _name_window_pixmap;
}

unsigned int x_client::net_wm_desktop(void) const { return _net_wm_desktop; }

void x_client::update_geometry(void)
{
  xcb_get_geometry_reply_t * geometry_reply =
    xcb_get_geometry_reply(_c(), xcb_get_geometry(_c(), _window), NULL);

  _rectangle.x()      = geometry_reply->x;
  _rectangle.y()      = geometry_reply->y;
  _rectangle.width()  = geometry_reply->width;
  _rectangle.height() = geometry_reply->height;

  delete geometry_reply;
}

bool
x_client::handle(xcb_generic_event_t * ge)
{
  auto update = [this](void)
  {
    update_parent_window();
    update_name_window_pixmap();
    observable::notify();
  };

  if (XCB_CONFIGURE_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_configure_notify_event_t * e = (xcb_configure_notify_event_t *)ge;
    if (e->window == _window) {
      _rectangle.x() = e->x; _rectangle.y() = e->y;
      _rectangle.width() = e->width; _rectangle.height() = e->height;
    }

    if (e->window == _c.root_window() || e->window == _window) {
      update();
    }

    return true;

  } else if (XCB_PROPERTY_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_property_notify_event_t * e = (xcb_property_notify_event_t *)ge;

    if (e->window != _window) return true;

    if (e->atom == a_net_wm_desktop) {
      update_net_wm_desktop();
    }

    return true;

  } else if (XCB_MAP_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_map_notify_event_t * e = (xcb_map_notify_event_t *)ge;

    if (e->window == _window) {
      update();
    }

    return true;

  } else if (XCB_UNMAP_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_unmap_notify_event_t * e = (xcb_unmap_notify_event_t *)ge;

    if (e->window == _window) {
      update();
    }

    return true;
  }

  return false;
}

// private

void
x_client::update_net_wm_desktop(void)
{
  xcb_generic_error_t * error = NULL;
  xcb_get_property_cookie_t c = xcb_get_property(
      _c(), false, _window, a_net_wm_desktop, XCB_ATOM_CARDINAL, 0, 32);
  xcb_get_property_reply_t * r = xcb_get_property_reply(_c(), c, &error);

  if (error || r->value_len == 0) {
    delete error;
    _net_wm_desktop = 0;
  } else {
    _net_wm_desktop = *(unsigned int *)xcb_get_property_value(r);
  }

  if (r) delete r;
}

void
x_client::update_parent_window(void)
{
  xcb_window_t next_parent = _window;

  while (next_parent != _c.root_window() && next_parent != XCB_NONE) {
    _parent = next_parent;
    next_parent = std::get<0>(_c.query_tree(next_parent));
  }
}

void
x_client::update_name_window_pixmap(void)
{
  xcb_free_pixmap(_c(), _name_window_pixmap);
  _name_window_pixmap = xcb_generate_id(_c());
  xcb_void_cookie_t c = xcb_composite_name_window_pixmap_checked(
      _c(), _parent, _name_window_pixmap);

  xcb_generic_error_t * error = xcb_request_check(_c(), c);

  if (error) {
    delete error;
    _name_window_pixmap = XCB_NONE;
    make_dummy();
  }
}

void
x_client::make_dummy(void)
{
  xcb_free_pixmap(_c(), _name_window_dummy);
  _name_window_dummy = xcb_generate_id(_c());
  xcb_create_pixmap(_c(), 24, _name_window_dummy, _c.root_window(),
      _rectangle.width(), _rectangle.height());

  xcb_gcontext_t gc = xcb_generate_id(_c());
  uint32_t fg = 0xff808080;

  xcb_create_gc(_c(), gc, _name_window_dummy, XCB_GC_FOREGROUND, &fg);
  xcb_rectangle_t r = {
    0, 0, (uint16_t)_rectangle.width(), (uint16_t)_rectangle.height() };
  xcb_poly_fill_rectangle(_c(), _name_window_dummy, gc, 1, &r);
  xcb_free_gc(_c(), gc);
}

// free functions

std::list<x_client>
make_x_clients(x_connection & c, const std::vector<xcb_window_t> & windows)
{
  std::list<x_client> x_clients;
  for (auto & window : windows) { x_clients.emplace_back(c, window); }
  return x_clients;
}

bool operator==(const x_client & x_client, const xcb_window_t & window)
{
  return x_client._window == window;
}

bool operator==(const xcb_window_t & window, const x_client & x_client)
{
  return x_client == window;
}
