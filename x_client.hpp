#ifndef _X_CLIENT_HPP
#define _X_CLIENT_HPP

#include <list>
#include <xcb/xcb.h>
#include <xcb/damage.h>

#include "data_types.hpp"
#include "x_event_handler_t.hpp"
#include "x_connection.hpp"

class x_client : public x_event_handler_t {
  public:
    friend std::ostream & operator<<(std::ostream &, const x_client &);
    friend bool operator==(const x_client &, const xcb_window_t &);
    friend bool operator==(const xcb_window_t &, const x_client &);

    x_client(x_connection & c, const xcb_window_t & window);
    ~x_client(void);

    bool operator==(const x_client & other)
    {
      return _window == other._window;
    }

          rectangle &  rect(void);
    const rectangle &  rect(void) const;
          xcb_window_t & window(void);
    const xcb_window_t & window(void) const;
          xcb_window_t & parent(void);
    const xcb_window_t & parent(void) const;
          xcb_pixmap_t & name_window_pixmap(void);
    const xcb_pixmap_t & name_window_pixmap(void) const;
          unsigned int   net_wm_desktop(void) const;

    bool handle(xcb_generic_event_t * ge);
    void update_geometry(void);

    void update_name_window_pixmap(void);

  private:
    x_connection & _c;
    rectangle _rectangle;
    unsigned int _net_wm_desktop;
    xcb_window_t _window;
    xcb_window_t _parent;
    xcb_pixmap_t _name_window_pixmap;

    void get_net_wm_desktop(void);
    void update_parent_window(void);
};


std::list<x_client>
make_x_clients(x_connection & c, const std::vector<xcb_window_t> & windows);

bool
operator==(const x_client &, const xcb_window_t &);

bool
operator==(const xcb_window_t &, const x_client &);

#endif
