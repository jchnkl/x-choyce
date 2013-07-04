#include "x_client_thumbnail.hpp"

x_client_thumbnail::x_client_thumbnail(x_connection & c,
                                       const rectangle_t & rectangle,
                                       const xcb_window_t & window,
                                       std::shared_ptr<x_client> xclient)
      : _c(c), _rectangle(rectangle)
{
  if (window == XCB_NONE && xclient == NULL) {
    throw std::invalid_argument(
        "x_client_thumbnail requires either window or xclient parameter");
  } else if (xclient == NULL) {
    _x_client = x_client_ptr(new x_client(_c, window));
  } else {
    _x_client = xclient;
  }

  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT;
  uint32_t values[] = { 0, true };
  _preview = xcb_generate_id(_c());
  xcb_create_window(_c(), XCB_COPY_FROM_PARENT, _preview,
                    _c.default_screen()->root,
                    0, 0, 1, 1, 0,
                    // _position.x, _position.y, _size.width, _size.height, 0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    _c.default_screen()->root_visual, mask, values);

  _damage = xcb_generate_id(_c());

  xcb_damage_create(_c(), _damage, _x_client->window(),
                    XCB_DAMAGE_REPORT_LEVEL_NON_EMPTY);

  _window_picture = make_picture(_c, _x_client->window());
  _preview_picture = make_picture(_c, _preview);

  _scale = std::min((double)rectangle.width() / _x_client->rectangle().width(),
                    (double)rectangle.height() / _x_client->rectangle().height());

  _rectangle.width() = _x_client->rectangle().width() * _scale;
  _rectangle.height() = _x_client->rectangle().height() * _scale;
}

x_client_thumbnail::~x_client_thumbnail(void)
{
  xcb_destroy_window(_c(), _preview);
  xcb_damage_destroy(_c(), _damage);
  xcb_render_free_picture(_c(), _alpha_picture);
  xcb_render_free_picture(_c(), _window_picture);
  xcb_render_free_picture(_c(), _preview_picture);
}
