#include "x_client.hpp"

x_client::x_client(x_connection & c, const xcb_window_t & window)
  : m_c(c), m_window(window)
{
  m_c.attach(0, XCB_MAP_NOTIFY, this);
  m_c.attach(0, XCB_UNMAP_NOTIFY, this);
  m_c.attach(0, XCB_CONFIGURE_NOTIFY, this);
  m_c.attach(0, XCB_PROPERTY_NOTIFY, this);
  m_c.update_input(m_window, XCB_EVENT_MASK_STRUCTURE_NOTIFY
                           | XCB_EVENT_MASK_PROPERTY_CHANGE);
  update_geometry();
  update_net_wm_desktop();
  update_parent_window();
  update_name_window_pixmap();
}

x_client::~x_client(void)
{
  m_c.detach(XCB_MAP_NOTIFY, this);
  m_c.detach(XCB_UNMAP_NOTIFY, this);
  m_c.detach(XCB_CONFIGURE_NOTIFY, this);
  m_c.detach(XCB_PROPERTY_NOTIFY, this);

  xcb_free_pixmap(m_c(), m_name_window_pixmap);
  xcb_free_pixmap(m_c(), _name_window_dummy);
}

const    rectangle & x_client::rect(void)   const { return m_rectangle; }
const xcb_window_t & x_client::window(void) const { return m_window; }
const xcb_window_t & x_client::parent(void) const { return m_parent; }

const xcb_window_t &
x_client::name_window_pixmap(void) const
{
  return m_name_window_pixmap == XCB_NONE
         ? _name_window_dummy
         : m_name_window_pixmap;
}

unsigned int x_client::net_wm_desktop(void) const { return m_net_wm_desktop; }

void x_client::update_geometry(void)
{
  xcb_get_geometry_reply_t * geometry_reply =
    xcb_get_geometry_reply(m_c(), xcb_get_geometry(m_c(), m_window), NULL);

  m_rectangle.x()      = geometry_reply->x;
  m_rectangle.y()      = geometry_reply->y;
  m_rectangle.width()  = geometry_reply->width;
  m_rectangle.height() = geometry_reply->height;

  delete geometry_reply;
}

bool
x_client::handle(xcb_generic_event_t * ge)
{
  auto update = [this](void)
  {
    observable::notify();
  };

  if (XCB_CONFIGURE_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_configure_notify_event_t * e = (xcb_configure_notify_event_t *)ge;
    if (e->window == m_window) {
      m_rectangle.x() = e->x; m_rectangle.y() = e->y;
      m_rectangle.width() = e->width; m_rectangle.height() = e->height;
    }

    if (e->window == m_c.root_window() || e->window == m_window) {
      update();
    }

    return true;

  } else if (XCB_PROPERTY_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_property_notify_event_t * e = (xcb_property_notify_event_t *)ge;

    if (e->window != m_window) return true;

    if (e->atom == a_net_wm_desktop) {
      update_net_wm_desktop();
    }

    return true;

  } else if (XCB_MAP_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_map_notify_event_t * e = (xcb_map_notify_event_t *)ge;

    if (e->window == m_window) {
      update();
    }

    return true;

  } else if (XCB_UNMAP_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_unmap_notify_event_t * e = (xcb_unmap_notify_event_t *)ge;

    if (e->window == m_window) {
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
      m_c(), false, m_window, a_net_wm_desktop, XCB_ATOM_CARDINAL, 0, 32);
  xcb_get_property_reply_t * r = xcb_get_property_reply(m_c(), c, &error);

  if (error || r->value_len == 0) {
    delete error;
    m_net_wm_desktop = 0;
  } else {
    m_net_wm_desktop = *(unsigned int *)xcb_get_property_value(r);
  }

  if (r) delete r;
}

void
x_client::update_parent_window(void)
{
  xcb_window_t next_parent = m_window;

  while (next_parent != m_c.root_window() && next_parent != XCB_NONE) {
    m_parent = next_parent;
    next_parent = std::get<0>(m_c.query_tree(next_parent));
  }
}

void
x_client::update_name_window_pixmap(void)
{
  xcb_free_pixmap(m_c(), m_name_window_pixmap);
  m_name_window_pixmap = xcb_generate_id(m_c());
  xcb_void_cookie_t c = xcb_composite_name_window_pixmap_checked(
      m_c(), m_parent, m_name_window_pixmap);

  xcb_generic_error_t * error = xcb_request_check(m_c(), c);

  if (error) {
    delete error;
    m_name_window_pixmap = XCB_NONE;
    make_dummy();
  }
}

void
x_client::make_dummy(void)
{
  xcb_free_pixmap(m_c(), _name_window_dummy);
  _name_window_dummy = xcb_generate_id(m_c());
  xcb_create_pixmap(m_c(), 24, _name_window_dummy, m_c.root_window(),
      m_rectangle.width(), m_rectangle.height());

  xcb_gcontext_t gc = xcb_generate_id(m_c());
  uint32_t fg = 0xff808080;

  xcb_create_gc(m_c(), gc, _name_window_dummy, XCB_GC_FOREGROUND, &fg);
  xcb_rectangle_t r = {
    0, 0, (uint16_t)m_rectangle.width(), (uint16_t)m_rectangle.height() };
  xcb_poly_fill_rectangle(m_c(), _name_window_dummy, gc, 1, &r);
  xcb_free_gc(m_c(), gc);
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
  return x_client.m_window == window;
}

bool operator==(const xcb_window_t & window, const x_client & x_client)
{
  return x_client == window;
}
