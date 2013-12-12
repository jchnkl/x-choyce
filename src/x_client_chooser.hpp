#ifndef X_CLIENT_CHOOSER
#define X_CLIENT_CHOOSER

#include <xcb/xcb.h>

#include "observer.hpp"
#include "chooser_t.hpp"
#include "x_event_handler_t.hpp"
#include "x_connection.hpp"
#include "config_t.hpp"

class x_client_chooser : public x_event_handler_t
                       , public observer<generic::config_t> {
  public:
    x_client_chooser(x_connection & c, generic::config_t & config, chooser_t * chooser);

    ~x_client_chooser(void);

    bool handle(xcb_generic_event_t * ge);
    void notify(generic::config_t *);

  private:
    bool m_active = false;
    bool m_ignore_release = false;

    xcb_window_t m_last_motion = XCB_NONE;

    x_connection & m_c;
    generic::config_t & m_config;
    chooser_t * m_chooser;
    x_connection::modifier_map m_modifier_map;

    // necessary for {un,}grabbing
    xcb_keysym_t m_action_keysym = XCB_NONE;
    xcb_keycode_t m_east_keycode;
    xcb_keycode_t m_west_keycode;
    xcb_keycode_t m_north_keycode;
    xcb_keycode_t m_south_keycode;
    xcb_keycode_t m_quit_keycode;
    xcb_keycode_t m_raise_keycode;
    xcb_keycode_t m_action_keycode;
    xcb_mod_mask_t m_action_modmask;

    void quit(void);
    void load_config(void);
};

#endif
