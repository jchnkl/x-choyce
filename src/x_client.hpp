#ifndef X_CLIENT_HPP
#define X_CLIENT_HPP

#include <list>
#include <xcb/xcb.h>
#include <xcb/damage.h>

#include "observer.hpp"
#include "data_types.hpp"
#include "x_event_handler_t.hpp"
#include "x_connection.hpp"

class x_client : public x_event_handler_t
               , public observable<x_client>
{
  public:
    friend std::ostream & operator<<(std::ostream &, const x_client &);
    friend bool operator==(const x_client &, const xcb_window_t &);
    friend bool operator==(const xcb_window_t &, const x_client &);

    x_client(x_connection & c, const xcb_window_t & window);
    ~x_client(void);

    bool operator==(const x_client & other)
    {
      return m_window == other.m_window;
    }

    const rectangle &  rect(void) const;
    const xcb_window_t & window(void) const;
    const xcb_window_t & parent(void) const;
    const xcb_pixmap_t & name_window_pixmap(void) const;
          unsigned int   net_wm_desktop(void) const;

    bool handle(xcb_generic_event_t * ge);
    void update_geometry(void);

    void update_parent_window(void);
    void update_name_window_pixmap(void);

  private:
    x_connection & m_c;

    rectangle m_rectangle;
    unsigned int _net_wm_desktop;
    xcb_window_t m_window;
    xcb_window_t _parent;
    xcb_pixmap_t _name_window_pixmap = XCB_NONE;
    xcb_pixmap_t _name_window_dummy = XCB_NONE;

    xcb_atom_t a_net_wm_desktop = m_c.intern_atom("_NET_WM_DESKTOP");

    void make_dummy(void);
    void update_net_wm_desktop(void);
};

std::list<x_client>
make_x_clients(x_connection & c, const std::vector<xcb_window_t> & windows);

bool
operator==(const x_client &, const xcb_window_t &);

bool
operator==(const xcb_window_t &, const x_client &);

#endif
