#ifndef _X_CLIENT_CHOOSER
#define _X_CLIENT_CHOOSER

#include <xcb/xcb.h>

#include "layout_t.hpp"
#include "x_connection.hpp"
#include "x_event_handler.hpp"
#include "x_client_container.hpp"

class x_client_chooser : public x_event_handler {
  public:
    x_client_chooser(const x_connection & c,
                     const layout_t * layout,
                     x_client_container & x_clients,
                     xcb_keysym_t action_keysym,
                     xcb_mod_mask_t action_modmask);

    void handle(xcb_generic_event_t * ge);

  private:
    bool _active = false;
    xcb_window_t _active_window;

    const x_connection & _c;
    const layout_t * _layout;
    x_client_container & _x_clients;
    x_client_container::iterator _current_client;
    x_connection::modifier_map _modifier_map;

    xcb_keycode_t _action_keycode;
    xcb_mod_mask_t _action_modmask;

    void move_client(bool next);
    void configure_clients_preview(void);
};

#endif
