#ifndef _X_CLIENT
#define _X_CLIENT

#include <list>
#include <xcb/xcb.h>
#include <xcb/damage.h>

#include "data_types.hpp"
#include "x_connection.hpp"

class x_client {
  public:
    friend std::ostream & operator<<(std::ostream &, const x_client_t &);
    friend bool operator==(const x_client_t &, const xcb_window_t &);
    friend bool operator==(const xcb_window_t &, const x_client_t &);

    x_client_t(const x_connection & c, xcb_window_t window);
    ~x_client_t(void);

    bool operator==(const x_client_t & other)
    {
      return _window == other._window;
    }

    double &       preview_scale(void);
    rectangle_t &  rectangle(void);
    position_t &   preview_position(void);
    rectangle_t &  preview_rectangle(void);
    unsigned int   net_wm_desktop(void) const;
    xcb_window_t & window(void);

    void handle(xcb_generic_event_t * ge);
    void update_geometry(void);

  private:
    const x_connection & _c;
    rectangle_t _rectangle;
    unsigned int _net_wm_desktop;
    xcb_window_t _window;

    void get_net_wm_desktop(void);
};

std::ostream & operator<<(std::ostream & os, const x_client_t & xc);

std::list<x_client_t>
make_x_clients(const x_connection & c, const std::vector<xcb_window_t> & windows);

bool
operator==(const x_client_t &, const xcb_window_t &);

bool
operator==(const xcb_window_t &, const x_client_t &);

#endif
