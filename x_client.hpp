#ifndef _X_CLIENT_HPP
#define _X_CLIENT_HPP

#include <list>
#include <xcb/xcb.h>
#include <xcb/damage.h>

#include "data_types.hpp"
#include "x_connection.hpp"

class x_client {
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
          unsigned int   net_wm_desktop(void) const;

    void handle(xcb_generic_event_t * ge);
    void update_geometry(void);

  private:
    x_connection & _c;
    rectangle _rectangle;
    unsigned int _net_wm_desktop;
    xcb_window_t _window;

    void get_net_wm_desktop(void);
};


std::list<x_client>
make_x_clients(x_connection & c, const std::vector<xcb_window_t> & windows);

bool
operator==(const x_client &, const xcb_window_t &);

bool
operator==(const xcb_window_t &, const x_client &);

#endif
