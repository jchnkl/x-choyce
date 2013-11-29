#include "x_client_thumbnail.hpp"

#include <xcb/xcb_renderutil.h>

x_client_thumbnail::x_client_thumbnail(x_connection & c,
                                       const rectangle & rect,
                                       const xcb_window_t & window,
                                       std::shared_ptr<x_client> xclient)
      : m_c(c)
{
  if (window == XCB_NONE && xclient == NULL) {
    throw std::invalid_argument(
        "x_client_thumbnail requires either window or xclient parameter");
  } else if (xclient == NULL) {
    _x_client = x_client_ptr(new x_client(m_c, window));
  } else {
    _x_client = xclient;
  }

  update_rectangle(rect);

  m_c.attach(0, m_c.damage_event_id(), this);
  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT;
  uint32_t values[] = { 0, true };
  _thumbnail_window = xcb_generate_id(m_c());
  xcb_create_window(m_c(), XCB_COPY_FROM_PARENT, _thumbnail_window,
                    m_c.default_screen()->root,
                    0, 0, 1, 1, 0,
                    // m_position.x, m_position.y, _size.width, _size.height, 0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    m_c.default_screen()->root_visual, mask, values);

  _damage = xcb_generate_id(m_c());

  _alpha_picture = xcb_generate_id(m_c());
  configure_alpha_picture(_alpha_value);

  m_window_picture = make_picture(m_c, _x_client->window());
  _thumbnail_picture = make_picture(m_c, _thumbnail_window);
}

x_client_thumbnail::~x_client_thumbnail(void)
{
  m_c.detach(m_c.damage_event_id(), this);
  xcb_destroy_window(m_c(), _thumbnail_window);
  xcb_render_free_picture(m_c(), _alpha_picture);
  xcb_render_free_picture(m_c(), m_window_picture);
  xcb_render_free_picture(m_c(), _thumbnail_picture);
}

void
x_client_thumbnail::show(void)
{
  xcb_damage_create(m_c(), _damage, _x_client->window(),
                    XCB_DAMAGE_REPORT_LEVEL_NON_EMPTY);
  configure_thumbnail_window();
  configure_thumbnail_picture();
  update();
}

void
x_client_thumbnail::hide(void)
{
  xcb_damage_destroy(m_c(), _damage);
  xcb_unmap_window(m_c(), _thumbnail_window);
}

void
x_client_thumbnail::select(void)
{
  m_c.request_change_current_desktop(_x_client->net_wm_desktop());
  m_c.request_change_active_window(_x_client->window());
}

void
x_client_thumbnail::update(void)
{
  update(0, 0, m_rectangle.width(), m_rectangle.height());
}

void
x_client_thumbnail::update(int x, int y, unsigned int width, unsigned int height)
{
  xcb_render_composite(m_c(), XCB_RENDER_PICT_OP_SRC,
                       m_window_picture, _alpha_picture, _thumbnail_picture,
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
x_client_thumbnail::highlight(bool want_highlight)
{
  xcb_rectangle_t r = { 0, 0, 1, 1 };
  xcb_render_color_t color = { 0, 0, 0, 0xffff };

  if (! want_highlight) { color.alpha = _alpha_value; }

  xcb_render_fill_rectangles(m_c(), XCB_RENDER_PICT_OP_SRC,
                             _alpha_picture, color, 1, &r);

  update();
}

bool
x_client_thumbnail::handle(xcb_generic_event_t * ge)
{
  if (m_c.damage_event_id() == (ge->response_type & ~0x80)) {
    xcb_damage_notify_event_t * e = (xcb_damage_notify_event_t *)ge;
    xcb_damage_subtract(m_c(), e->damage, XCB_NONE, XCB_NONE);
    update(e->area.x * _scale, e->area.y * _scale,
           e->area.width * _scale, e->area.height * _scale);
    return true;
  }

  return false;
}

void
x_client_thumbnail::update_rectangle(const rectangle & rect)
{
  _scale = std::min((double)rect.width() / _x_client->rect().width(),
                    (double)rect.height() / _x_client->rect().height());

  m_rectangle.x() = rect.x();
  m_rectangle.y() = rect.y();
  m_rectangle.width() = _x_client->rect().width() * _scale;
  m_rectangle.height() = _x_client->rect().height() * _scale;
}

void
x_client_thumbnail::configure_thumbnail_window(void)
{
  uint32_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
                | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT
                | XCB_CONFIG_WINDOW_STACK_MODE;

  uint32_t values[] = { (uint32_t)m_rectangle.x(),
                        (uint32_t)m_rectangle.y(),
                        (uint32_t)m_rectangle.width(),
                        (uint32_t)m_rectangle.height(),
                        XCB_STACK_MODE_ABOVE };

  xcb_configure_window(m_c(), _thumbnail_window, mask, values);
  xcb_map_window(m_c(), _thumbnail_window);
}

void
x_client_thumbnail::configure_thumbnail_picture(void)
{
  xcb_render_transform_t transform_matrix = {
      DOUBLE_TO_FIXED(1), DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(    0)
    , DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(1), DOUBLE_TO_FIXED(    0)
    , DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(_scale)
    };

  xcb_render_set_picture_transform(m_c(), m_window_picture, transform_matrix);
}

void
x_client_thumbnail::configure_alpha_picture(uint16_t alpha_value)
{
  xcb_pixmap_t alpha_pixmap = xcb_generate_id(m_c());
  xcb_create_pixmap(m_c(), 8, alpha_pixmap, m_c.root_window(), 1, 1);

  const xcb_render_query_pict_formats_reply_t * formats_reply =
    xcb_render_util_query_formats(m_c());
  xcb_render_pictforminfo_t * format =
    xcb_render_util_find_standard_format(formats_reply, XCB_PICT_STANDARD_A_8);

  uint32_t mask = XCB_RENDER_CP_REPEAT | XCB_RENDER_CP_COMPONENT_ALPHA;
  uint32_t values[] = { true, true };

  xcb_render_create_picture(m_c(), _alpha_picture, alpha_pixmap,
                            format->id, mask, values);

  xcb_free_pixmap(m_c(), alpha_pixmap);

  xcb_render_color_t color = { 0, 0, 0, alpha_value };

  xcb_rectangle_t r = { 0, 0, 1, 1 };
  xcb_render_fill_rectangles(m_c(), XCB_RENDER_PICT_OP_SRC,
                             _alpha_picture, color, 1, &r);
}

bool operator==(const x_client_thumbnail & thumbnail, const xcb_window_t & window)
{
  return *(thumbnail._x_client) == window;
}

bool operator==(const xcb_window_t & window, const x_client_thumbnail & thumbnail)
{
  return thumbnail == window;
}
