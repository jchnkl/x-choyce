#ifndef X_CLIENT_ICON_HPP
#define X_CLIENT_ICON_HPP

#include "x_connection.hpp"
#include "x_client.hpp"

class x_client_icon : public x_event_handler_t {
  public:
    x_client_icon(x_connection & c, x_client & x_client);
    ~x_client_icon(void);

    const xcb_pixmap_t & icon(void) const;
    const xcb_pixmap_t & net_wm_icon(void) const;
    const xcb_pixmap_t & wm_hints_icon(void) const;
    const std::pair<unsigned int, unsigned int> & icon_geometry(void) const;
    bool handle(xcb_generic_event_t *);

  private:
    x_connection & m_c;
    x_client & m_x_client;

    xcb_pixmap_t m_net_wm_icon = XCB_NONE;
    xcb_pixmap_t m_wm_hints_icon = XCB_NONE;

    std::pair<unsigned int, unsigned int> m_icon_geometry;

    xcb_atom_t m_a_wm_hints = m_c.intern_atom("WM_HINTS");
    xcb_atom_t m_a_net_wm_icon = m_c.intern_atom("_NET_WM_ICON");

    void update_net_wm_icon(void);
    void update_wm_hints_icon(void);
    void alpha_transform(uint8_t * data, unsigned int w, unsigned int h);
};

// inline

inline const xcb_pixmap_t &
x_client_icon::icon(void) const
{
  if (m_net_wm_icon == XCB_NONE) {
    return m_wm_hints_icon;
  } else {
    return m_net_wm_icon;
  }
}

inline const xcb_pixmap_t &
x_client_icon::net_wm_icon(void) const
{
  return m_net_wm_icon;
}

inline const xcb_pixmap_t &
x_client_icon::wm_hints_icon(void) const
{
  return m_wm_hints_icon;
}

inline const std::pair<unsigned int, unsigned int> &
x_client_icon::icon_geometry(void) const
{
  return m_icon_geometry;
}

#endif
