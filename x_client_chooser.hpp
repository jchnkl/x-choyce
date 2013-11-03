#ifndef _X_CLIENT_CHOOSER
#define _X_CLIENT_CHOOSER

#include <xcb/xcb.h>

#include "chooser_t.hpp"
#include "x_event_handler_t.hpp"
#include "x_connection.hpp"

class x_client_chooser : public x_event_handler_t {
  public:
    x_client_chooser(x_connection & c,
                     chooser_t * chooser,
                     xcb_keysym_t quit_keysym,
                     xcb_keysym_t action_keysym,
                     xcb_mod_mask_t action_modmask);

    ~x_client_chooser(void);

    bool handle(xcb_generic_event_t * ge);

  private:
    bool _active = false;
    bool _ignore_release = false;

    xcb_window_t _last_motion = XCB_NONE;

    x_connection & _c;
    chooser_t * _chooser;
    x_connection::modifier_map _modifier_map;

    xcb_keycode_t _quit_keycode;
    xcb_keycode_t _action_keycode;
    xcb_mod_mask_t _action_modmask;

    void quit(void);
};

#endif
