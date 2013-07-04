#ifndef _X_EWMH_HPP
#define _X_EWMH_HPP

#include <xcb/xcb.h>

#include "x_connection.hpp"
#include "x_event_handler.hpp"

class x_ewmh : public x_event_handler {
  public:
    x_ewmh(x_connection & c);
    ~x_ewmh(void);
    xcb_window_t net_active_window(void) const;
    void handle(xcb_generic_event_t * ge);

  private:
    x_connection & _c;
    xcb_window_t _net_active_window = XCB_NONE;

    void update_net_active_window(void);
};

#endif
