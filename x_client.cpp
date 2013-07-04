#include "x_client.hpp"

x_client::x_client(x_connection & c, const xcb_window_t & window)
  : _c(c), _window(window)
{
  update_geometry();
  get_net_wm_desktop();
}

x_client::~x_client(void) {}

      rectangle_t &  x_client::rectangle(void)       { return _rectangle; }
const rectangle_t &  x_client::rectangle(void) const { return _rectangle; }
      xcb_window_t & x_client::window(void)          { return _window; }
const xcb_window_t & x_client::window(void) const    { return _window; }

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

void x_client::get_net_wm_desktop(void)
{
  std::string atom_name = "_NET_WM_DESKTOP";
  xcb_intern_atom_cookie_t atom_cookie =
    xcb_intern_atom(_c(), false, atom_name.length(), atom_name.c_str());
  xcb_intern_atom_reply_t * atom_reply =
    xcb_intern_atom_reply(_c(), atom_cookie, NULL);

  xcb_get_property_cookie_t property_cookie =
    xcb_get_property(_c(), false, _window,
                     atom_reply->atom, XCB_ATOM_CARDINAL, 0, 32);

  delete atom_reply;

  xcb_generic_error_t * error = NULL;
  xcb_get_property_reply_t * property_reply =
    xcb_get_property_reply(_c(), property_cookie, &error);

  if (error || property_reply->value_len == 0) {
    delete error;
    _net_wm_desktop = 0;
  } else {
    _net_wm_desktop =
      *(unsigned int *)xcb_get_property_value(property_reply);
  }

  delete property_reply;
}


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
