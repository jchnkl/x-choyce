#ifndef X_CLIENT_CHOOSER
#define X_CLIENT_CHOOSER

#include <xcb/xcb.h>

#include "observer.hpp"
#include "chooser_t.hpp"
#include "x_event_handler_t.hpp"
#include "x_connection.hpp"
#include "x_xrm.hpp"

class x_client_chooser : public x_event_handler_t
                       , public observer<x::xrm> {
  public:
    x_client_chooser(x_connection & c, x::xrm & xrm, chooser_t * chooser);

    ~x_client_chooser(void);

    bool handle(xcb_generic_event_t * ge);
    void notify(x::xrm *);

  private:
    bool m_active = false;
    bool _ignore_release = false;

    xcb_window_t _last_motion = XCB_NONE;

    x_connection & m_c;
    x::xrm & _xrm;
    chooser_t * _chooser;
    x_connection::modifier_map _modifier_map;

    // necessary for {un,}grabbing
    xcb_keysym_t _action_keysym = XCB_NONE;
    xcb_keycode_t _east_keycode;
    xcb_keycode_t _west_keycode;
    xcb_keycode_t _north_keycode;
    xcb_keycode_t _south_keycode;
    xcb_keycode_t _quit_keycode;
    xcb_keycode_t _action_keycode;
    xcb_mod_mask_t _action_modmask;

    void quit(void);
    void load_config(void);
};

#endif
