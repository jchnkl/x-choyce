#ifndef _X_CONNECTION
#define _X_CONNECTION

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <xcb/xcb.h>
#include <xcb/render.h>

#include "data_types.hpp"
#include "x_event_handler.hpp"
#include "x_event_source_t.hpp"

#include <X11/Xlib.h>
class x_event_source;

class x_connection : public x_event_handler
                   , public x_event_source_t {
  public:
    typedef std::map<xcb_mod_mask_t, std::vector<xcb_keycode_t>> modifier_map;

    x_connection(std::shared_ptr<x_event_source> event_source = NULL);
    ~x_connection(void);

    xcb_connection_t * operator()(void) const;

    void select_input(xcb_window_t window, xcb_event_mask_t event_mask) const;
    xcb_visualtype_t * default_visual_of_screen(void);
    void flush(void) const;
    xcb_screen_t * const default_screen(void) const;
    xcb_window_t const & root_window(void) const;
    uint8_t damage_event_id(void) const;
    void grab_key(uint16_t modifiers, xcb_keysym_t keysym) const;
    void ungrab_key(uint16_t modifiers, xcb_keysym_t keysym) const;
    void grab_keyboard(void) const;
    void ungrab_keyboard(void) const;
    modifier_map modifier_mapping(void) const;
    xcb_keysym_t keycode_to_keysym(xcb_keycode_t keycode) const;
    xcb_keycode_t keysym_to_keycode(xcb_keysym_t keysym) const;
    std::string keysym_to_string(xcb_keysym_t keysym) const;
    std::vector<xcb_window_t> net_client_list_stacking(void) const;
    xcb_atom_t intern_atom(const std::string & atom_name) const;
    xcb_window_t net_active_window(void) const;
    void request_change_current_desktop(unsigned int desktop_id) const;
    void request_change_active_window(xcb_window_t window) const;
    rectangle_t current_screen(void) const;
    void handle(xcb_generic_event_t * ge);

    void register_handler(x_event_handler * eh);
    void unregister_handler(x_event_handler * eh);
    void run_event_loop(void);

  private:
    uint8_t _damage_event_id;
    int _screen_number = 0;
    xcb_window_t _root_window = 0;
    xcb_connection_t * _c = NULL;
    xcb_screen_t * _default_screen = NULL;

    std::vector<rectangle_t> _screens;

    std::shared_ptr<x_event_source_t> _event_source;

    void find_default_screen(void);
    void init_damage(void);
    void init_render(void);
    void init_xinerama(void);
    void update_xinerama(void);
};

xcb_render_pictformat_t
render_find_visual_format(const x_connection & c, xcb_visualid_t visual);

xcb_render_picture_t
make_picture(const x_connection & c, xcb_window_t window);

xcb_visualtype_t *
argb_visual(const x_connection & c);

#endif
