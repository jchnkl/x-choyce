#ifndef _X_CLIENT
#define _X_CLIENT

#include <list>
#include <xcb/xcb.h>
#include <xcb/damage.h>

#include "data_types.hpp"
#include "x_event_handler.hpp"
#include "x_connection.hpp"

// http://svn.enlightenment.org/svn/e/tags/evas-1.0.2/src/modules/engines/xrender_x11/evas_engine_xcb_render.c
#define DOUBLE_TO_FIXED(d) ((xcb_render_fixed_t) ((d) * 65536))

class x_client_t : public x_event_handler {
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
    void hide_preview(void) const;
    void update_preview(bool is_active);
    void show_preview(bool is_active);
    void compose(const rectangle_t & rectangle);
    void update_geometry(void);

  private:
    const x_connection & _c;
    bool _preview_is_active = false;
    double _preview_scale;
    rectangle_t _rectangle;
    rectangle_t _preview_rectangle;
    position_t _preview_position;
    unsigned int _net_wm_desktop;
    xcb_window_t _window;
    xcb_window_t _preview;
    xcb_render_picture_t _window_picture;
    xcb_render_picture_t _preview_picture;
    xcb_damage_damage_t _damage;

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
