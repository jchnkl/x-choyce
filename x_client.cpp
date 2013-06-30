#include "x_client.hpp"

x_client::x_client(const x_connection & c, xcb_window_t window)
  : _c(c), _window(window)
{
  update_geometry();
  get_net_wm_desktop();

  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT;
  uint32_t values[] = { 0, true };
  _preview = xcb_generate_id(c());
  xcb_create_window(_c(), XCB_COPY_FROM_PARENT, _preview,
                    _c.default_screen()->root,
                    0, 0, 1, 1, 0,
                    // _position.x, _position.y, _size.width, _size.height, 0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    _c.default_screen()->root_visual, mask, values);

  _damage = xcb_generate_id(_c());
  xcb_damage_create(_c(), _damage, _window,
                    XCB_DAMAGE_REPORT_LEVEL_NON_EMPTY);
}

x_client::~x_client(void)
{
  xcb_damage_destroy(_c(), _damage);
  xcb_render_free_picture(_c(), _window_picture);
  xcb_render_free_picture(_c(), _preview_picture);
  xcb_destroy_window(_c(), _preview);
}

bool x_client::operator==(const xcb_window_t & window) { return _window == window; }

double &       x_client::preview_scale(void)        { return _preview_scale; }
rectangle_t &  x_client::rectangle(void)            { return _rectangle; }
position_t &   x_client::preview_position(void)     { return _preview_position; }
rectangle_t &  x_client::preview_rectangle(void)    { return _preview_rectangle; }
unsigned int   x_client::net_wm_desktop(void) const { return _net_wm_desktop; }
xcb_window_t & x_client::window(void)               { return _window; }

void x_client::handle(xcb_generic_event_t * ge)
{
  if (_c.damage_event_id() == (ge->response_type & ~0x80)) {
    xcb_damage_notify_event_t * e = (xcb_damage_notify_event_t *)ge;
    xcb_damage_subtract(_c(), e->damage, XCB_NONE, XCB_NONE);
    compose(rectangle_t(e->area.x, e->area.y, e->area.width, e->area.height));
  }
}

void x_client::hide_preview(void) const
{
  xcb_unmap_window(_c(), _preview);
  xcb_render_free_picture(_c(), _window_picture);
  xcb_render_free_picture(_c(), _preview_picture);
}

void x_client::update_preview(bool is_active)
{
  _preview_is_active = is_active;
  compose(rectangle_t(0, 0, _rectangle.width(), _rectangle.height()));
}

void x_client::show_preview(bool is_active)
{
  _preview_is_active = is_active;

  xcb_get_geometry_reply_t * geometry_reply =
    xcb_get_geometry_reply(_c(), xcb_get_geometry(_c(), _window), NULL);

  delete geometry_reply;

  uint32_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
                | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT
                | XCB_CONFIG_WINDOW_STACK_MODE;
  uint32_t values[] = { (uint32_t)_preview_position.x,
                        (uint32_t)_preview_position.y,
                        (uint32_t)(_rectangle.width() * _preview_scale),
                        (uint32_t)(_rectangle.height() * _preview_scale),
                        XCB_STACK_MODE_ABOVE };

  xcb_configure_window(_c(), _preview, mask, values);
  xcb_map_window(_c(), _preview);

  _window_picture = make_picture(_c, _window);
  _preview_picture = make_picture(_c, _preview);

  xcb_render_transform_t transform_matrix =
    { DOUBLE_TO_FIXED(1), DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(             0)
    , DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(1), DOUBLE_TO_FIXED(             0)
    , DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(_preview_scale)
    };

  xcb_render_set_picture_transform(_c(), _window_picture, transform_matrix);

  compose(rectangle_t(0, 0, _rectangle.width(), _rectangle.height()));
}

void x_client::compose(const rectangle_t & rectangle)
{
  int16_t x = rectangle.x() * _preview_scale;
  int16_t y = rectangle.y() * _preview_scale;
  uint16_t width = rectangle.width() * _preview_scale;
  uint16_t height = rectangle.height() * _preview_scale;

  uint8_t op = XCB_RENDER_PICT_OP_SRC;
  xcb_render_composite(_c(), op,
                       _window_picture, XCB_NONE, _preview_picture,
                       // int16_t src_x, int16_t src_y,
                       x, y,
                       // int16_t mask_x, int16_t mask_y,
                       0, 0,
                       // int16_t dst_x, int16_t dst_y,
                       x, y,
                       // uint16_t width, uint16_t height
                       width, height);

  if (_preview_is_active) { return; }

  op = XCB_RENDER_PICT_OP_OVER;
  xcb_render_color_t color = { (uint16_t)(0.0f * 0xffff),
                               (uint16_t)(0.0f * 0xffff),
                               (uint16_t)(0.0f * 0xffff),
                               (uint16_t)(0.25f * 0xffff) };
  xcb_rectangle_t r = { x, y, width, height };
  xcb_render_fill_rectangles(_c(), op, _preview_picture, color, 1, &r);
}

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

std::ostream & operator<<(std::ostream & os, const x_client & xc)
{
  return os << "0x" << std::hex << xc._window << std::dec << " @ "
            << xc._rectangle.x()     << "x"
            << xc._rectangle.y()     << "+"
            << xc._rectangle.width() << "+"
            << xc._rectangle.height()
            << " on desktop " << xc._net_wm_desktop
            << " preview with scale "
            << xc._preview_scale << " @ "
            << xc._preview_position.x << "x"
            << xc._preview_position.y << "+"
            << (uint)(xc._rectangle.width() * xc._preview_scale) << "+"
            << (uint)(xc._rectangle.height() * xc._preview_scale)
            ;
}

std::list<x_client>
make_x_clients(const x_connection & c, const std::vector<xcb_window_t> & windows)
{
  std::list<x_client> x_clients;
  for (auto & window : windows) { x_clients.emplace_back(c, window); }
  return x_clients;
}
