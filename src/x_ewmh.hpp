#ifndef X_EWMH_HPP
#define X_EWMH_HPP

#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

#include "x_connection.hpp"
#include "x_event_handler_t.hpp"

class x_connection;

class x_ewmh : public x_event_handler_t {
  public:
    x_ewmh(x_connection & c);
    ~x_ewmh(void);
    xcb_ewmh_connection_t * connection(void) const;
    xcb_window_t net_active_window(void) const;
    bool handle(xcb_generic_event_t * ge);

  private:
    x_connection & m_c;
    xcb_ewmh_connection_t m_ewmh;
    xcb_window_t m_net_active_window = XCB_NONE;

    void update_net_active_window(void);
};

#endif
