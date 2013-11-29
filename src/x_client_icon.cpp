#include "x_client_icon.hpp"

#include <cstring> // memset
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_image.h>

x_client_icon::x_client_icon(x_connection & c, x_client & x_client)
  : m_c(c), m_x_client(x_client)
{
  m_c.attach(10, XCB_PROPERTY_NOTIFY, this);
  m_c.update_input(m_x_client.window(), XCB_EVENT_MASK_PROPERTY_CHANGE);

  update_net_wm_icon();
  if (m_net_wm_icon == XCB_NONE) {
    update_wm_hints_icon();
  }
}

x_client_icon::~x_client_icon(void)
{
  m_c.detach(XCB_PROPERTY_NOTIFY, this);
  xcb_free_pixmap(m_c(), m_net_wm_icon);
  xcb_free_pixmap(m_c(), m_wm_hints_icon);
}

bool
x_client_icon::handle(xcb_generic_event_t * ge)
{
  if (XCB_PROPERTY_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_property_notify_event_t * e = (xcb_property_notify_event_t *)ge;

    if (e->window != m_x_client.window()) {
      return true;

    } else if (e->atom == m_a_wm_hints) {
      update_wm_hints_icon();

    } else if (e->atom == m_a_net_wm_icon) {
      update_net_wm_icon();

    }

    return true;
  }

  return false;
}

// private

void
x_client_icon::update_net_wm_icon(void)
{
  xcb_free_pixmap(m_c(), m_net_wm_icon);
  m_net_wm_icon = XCB_NONE;

  xcb_generic_error_t * error;
  xcb_get_property_cookie_t c =
    xcb_ewmh_get_wm_icon(m_c.ewmh(), m_x_client.window());

  xcb_ewmh_get_wm_icon_reply_t wm_icon;
  std::memset(&wm_icon, 0, sizeof(xcb_ewmh_get_wm_icon_reply_t));
  xcb_ewmh_get_wm_icon_reply(m_c.ewmh(), c, &wm_icon, &error);

  if (error) {
    delete error;

  } else if (0 < xcb_ewmh_get_wm_icon_length(&wm_icon)) {

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t * data = NULL;

    xcb_ewmh_wm_icon_iterator_t iter = xcb_ewmh_get_wm_icon_iterator(&wm_icon);
    for (; iter.rem; xcb_ewmh_get_wm_icon_next(&iter)) {
      if (iter.width > width) {
        width = iter.width;
        height = iter.height;
        data = iter.data;
      }
    }

    _icon_geometry.first = width;
    _icon_geometry.second = height;

    m_net_wm_icon = xcb_generate_id(m_c());
    xcb_create_pixmap(
        m_c(), 32, m_net_wm_icon, m_c.root_window(), width, height);

    xcb_image_t * image = xcb_image_create_native(
        m_c(), width, height, XCB_IMAGE_FORMAT_Z_PIXMAP, 32, NULL, 0, NULL);

    image->data = (uint8_t *)data;

    alpha_transform(image->data, width, height);

    xcb_gcontext_t gc = xcb_generate_id(m_c());
    xcb_create_gc(m_c(), gc, m_net_wm_icon, 0, NULL);

    xcb_image_put(m_c(), m_net_wm_icon, gc, image, 0, 0, 0);

    xcb_image_destroy(image);
    xcb_free_gc(m_c(), gc);

    xcb_ewmh_get_wm_icon_reply_wipe(&wm_icon);
  }
}

void
x_client_icon::update_wm_hints_icon(void)
{
  xcb_free_pixmap(m_c(), m_wm_hints_icon);
  m_wm_hints_icon = XCB_NONE;

  xcb_generic_error_t * error;

  xcb_get_property_cookie_t c =
    xcb_icccm_get_wm_hints(m_c(), m_x_client.window());
  xcb_get_property_reply_t * r = xcb_get_property_reply(m_c(), c, &error);

  if (error) {
    delete error;

  } else {
    xcb_icccm_wm_hints_t wm_hints;
    xcb_icccm_get_wm_hints_from_reply(&wm_hints, r);

    if (wm_hints.flags & XCB_ICCCM_WM_HINT_ICON_PIXMAP) {
      unsigned int width, height;

      {
        Window root;
        int x, y;
        unsigned int border_width, depth;
        XGetGeometry(m_c.dpy(), wm_hints.icon_pixmap, &root,
            &x, &y, &width, &height, &border_width, &depth);
        _icon_geometry.first = width;
        _icon_geometry.second = height;
      }

      xcb_image_t * icon_rgb = xcb_image_get(m_c(), wm_hints.icon_pixmap,
          0, 0, width, height, 0xffffffff, XCB_IMAGE_FORMAT_XY_PIXMAP);

      xcb_image_t * icon_mask;
      if (wm_hints.flags & XCB_ICCCM_WM_HINT_ICON_MASK) {
        icon_mask = xcb_image_get(m_c(), wm_hints.icon_mask,
            0, 0, width, height, 0xffffffff, XCB_IMAGE_FORMAT_XY_PIXMAP);

      } else {
        icon_mask = xcb_image_create_native(
            m_c(), width, height, XCB_IMAGE_FORMAT_Z_PIXMAP, 32, NULL, 0, NULL);
        std::memset(icon_mask->data, 0xff,
                    width * height * (icon_mask->bpp / icon_mask->stride));
      }

      xcb_image_t * icon_rgba = xcb_image_create_native(
          m_c(), width, height, XCB_IMAGE_FORMAT_Z_PIXMAP, 32, NULL, 0, NULL);

      for (std::size_t x = 0; x < width; ++x) {
        for (std::size_t y = 0; y < height; ++y) {
          uint32_t rgba = 0;

          if (xcb_image_get_pixel(icon_mask, x, y)) {
            uint32_t rgb = xcb_image_get_pixel(icon_rgb, x, y);
            uint8_t * s = (uint8_t *)&rgb;
            uint8_t * d = (uint8_t *)&rgba;

            d[0] = s[0];
            d[1] = s[1];
            d[2] = s[2];
            d[3] = 0xff;
          }

          xcb_image_put_pixel(icon_rgba, x, y, rgba);
        }
      }

      m_wm_hints_icon = xcb_generate_id(m_c());
      xcb_create_pixmap(
          m_c(), 32, m_wm_hints_icon, m_c.root_window(), width, height);

      xcb_gcontext_t gc = xcb_generate_id(m_c());
      xcb_create_gc(m_c(), gc, m_wm_hints_icon, 0, NULL);

      xcb_image_put(m_c(), m_wm_hints_icon, gc, icon_rgba, 0, 0, 0);

      xcb_image_destroy(icon_rgb);
      xcb_image_destroy(icon_mask);
      xcb_image_destroy(icon_rgba);
      xcb_free_gc(m_c(), gc);
    }
  }

  if (r) delete r;
}

// stolen from openbox: tests/icons.c
void
x_client_icon::alpha_transform(uint8_t * data, unsigned int w, unsigned int h)
{
  for (uint32_t j = 0; j < w * h; j++) {
    unsigned char a = (unsigned char)data[j*sizeof(uint32_t)+3];
    unsigned char r = (unsigned char)data[j*sizeof(uint32_t)+0];
    unsigned char g = (unsigned char)data[j*sizeof(uint32_t)+1];
    unsigned char b = (unsigned char)data[j*sizeof(uint32_t)+2];

    // background color
    unsigned char bgr = 0;
    unsigned char bgg = 0;
    unsigned char bgb = 0;

    r = bgr + (r - bgr) * a / 256;
    g = bgg + (g - bgg) * a / 256;
    b = bgb + (b - bgb) * a / 256;

    data[j*sizeof(uint32_t)+0] = (char)r;
    data[j*sizeof(uint32_t)+1] = (char)g;
    data[j*sizeof(uint32_t)+2] = (char)b;
  }
}
