#ifndef _X_CLIENTS_PREVIEW
#define _X_CLIENTS_PREVIEW

#include <xcb/xcb.h>

#include "layout_t.hpp"
#include "x_connection.hpp"
#include "x_event_handler.hpp"
#include "x_client_container.hpp"

class x_clients_preview : public x_event_handler {
  public:
    x_clients_preview(const x_connection & c,
                      const layout_t * layout,
                      x_client_container & x_clients);

    void handle(xcb_generic_event_t * ge);

  private:
    bool _active = false;
    xcb_window_t _active_window;

    const x_connection & _c;
    const layout_t * _layout;
    x_client_container & _x_clients;
    x_client_container::iterator _current_client;
    x_connection::modifier_map _modifier_map;

    void move_client(bool next);
    void configure_clients_preview(void);
};

#endif
