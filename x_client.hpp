#ifndef _X_CLIENT
#define _X_CLIENT

#include <list>
#include <xcb/xcb.h>

#include "data_types.hpp"
#include "x_event_handler.hpp"
#include "x_connection.hpp"

class x_client : public x_event_handler {
  public:
    friend std::ostream & operator<<(std::ostream &, const x_client &);

    x_client(const x_connection & c, xcb_window_t window);
    ~x_client(void);

    bool operator==(const xcb_window_t & window);

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
};

std::ostream & operator<<(std::ostream & os, const x_client & xc);

std::list<x_client>
make_x_clients(const x_connection & c, const std::vector<xcb_window_t> & windows);

#endif
