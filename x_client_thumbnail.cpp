#include "x_client_thumbnail.hpp"

#include <xcb/xcb_renderutil.h>

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

  _c.register_handler(this);
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

  _alpha_picture = xcb_generate_id(_c());
  configure_alpha_picture(_alpha_value);

  _window_picture = make_picture(_c, _x_client->window());
  _preview_picture = make_picture(_c, _preview);

  _scale = std::min((double)rectangle.width() / _x_client->rectangle().width(),
                    (double)rectangle.height() / _x_client->rectangle().height());

  _rectangle.width() = _x_client->rectangle().width() * _scale;
  _rectangle.height() = _x_client->rectangle().height() * _scale;
}

x_client_thumbnail::~x_client_thumbnail(void)
{
  _c.unregister_handler(this);
  xcb_destroy_window(_c(), _preview);
  xcb_damage_destroy(_c(), _damage);
  xcb_render_free_picture(_c(), _alpha_picture);
  xcb_render_free_picture(_c(), _window_picture);
  xcb_render_free_picture(_c(), _preview_picture);
}

void
x_client_thumbnail::show(void) const
{
  configure_preview_window();
  configure_preview_picture();
  update();
}

void
x_client_thumbnail::hide(void) const
{
  xcb_unmap_window(_c(), _preview);
}

void
x_client_thumbnail::select(void) const
{
  _c.request_change_current_desktop(_x_client->net_wm_desktop());
  _c.request_change_active_window(_x_client->window());
}

void
x_client_thumbnail::update(void) const
{
  update(0, 0, _rectangle.width(), _rectangle.height());
}

void
x_client_thumbnail::update(int x, int y, unsigned int width, unsigned int height) const
{
  xcb_render_composite(_c(), XCB_RENDER_PICT_OP_SRC,
                       _window_picture, _alpha_picture, _preview_picture,
                       // int16_t src_x, int16_t src_y,
                       x, y,
                       // int16_t mask_x, int16_t mask_y,
                       0, 0,
                       // int16_t dst_x, int16_t dst_y,
                       x, y,
                       // uint16_t width, uint16_t height
                       width, height);
}

void
x_client_thumbnail::highlight(bool want_highlight) const
{
  xcb_rectangle_t r = { 0, 0, 1, 1 };
  xcb_render_color_t color = { 0, 0, 0, 0xffff };

  if (! want_highlight) { color.alpha = _alpha_value; }

  xcb_render_fill_rectangles(_c(), XCB_RENDER_PICT_OP_SRC,
                             _alpha_picture, color, 1, &r);

  update();
}

void
x_client_thumbnail::handle(xcb_generic_event_t * ge)
{
  if (_c.damage_event_id() == (ge->response_type & ~0x80)) {
    xcb_damage_notify_event_t * e = (xcb_damage_notify_event_t *)ge;
    xcb_damage_subtract(_c(), e->damage, XCB_NONE, XCB_NONE);
    update(e->area.x * _scale, e->area.y * _scale,
           e->area.width * _scale, e->area.height * _scale);
  }
}

void
x_client_thumbnail::configure_preview_window(void) const
{
  uint32_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
                | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT
                | XCB_CONFIG_WINDOW_STACK_MODE;

  uint32_t values[] = { (uint32_t)_rectangle.x(),
                        (uint32_t)_rectangle.y(),
                        (uint32_t)_rectangle.width(),
                        (uint32_t)_rectangle.height(),
                        XCB_STACK_MODE_ABOVE };

  xcb_configure_window(_c(), _preview, mask, values);
  xcb_map_window(_c(), _preview);
}

void
x_client_thumbnail::configure_preview_picture(void) const
{
  xcb_render_transform_t transform_matrix = {
      DOUBLE_TO_FIXED(1), DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(    0)
    , DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(1), DOUBLE_TO_FIXED(    0)
    , DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(_scale)
    };

  xcb_render_set_picture_transform(_c(), _window_picture, transform_matrix);
}

void
x_client_thumbnail::configure_alpha_picture(uint16_t alpha_value) const
{
  xcb_pixmap_t alpha_pixmap = xcb_generate_id(_c());
  xcb_create_pixmap(_c(), 8, alpha_pixmap, _c.root_window(), 1, 1);

  const xcb_render_query_pict_formats_reply_t * formats_reply =
    xcb_render_util_query_formats(_c());
  xcb_render_pictforminfo_t * format =
    xcb_render_util_find_standard_format(formats_reply, XCB_PICT_STANDARD_A_8);

  uint32_t mask = XCB_RENDER_CP_REPEAT | XCB_RENDER_CP_COMPONENT_ALPHA;
  uint32_t values[] = { XCB_RENDER_REPEAT_NORMAL, true };

  xcb_render_create_picture(_c(), _alpha_picture, alpha_pixmap,
                            format->id, mask, values);

  xcb_free_pixmap(_c(), alpha_pixmap);

  xcb_render_color_t color = { 0, 0, 0, alpha_value };

  xcb_rectangle_t r = { 0, 0, 1, 1 };
  xcb_render_fill_rectangles(_c(), XCB_RENDER_PICT_OP_SRC,
                             _alpha_picture, color, 1, &r);
}
