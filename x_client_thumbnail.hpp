#ifndef _X_CLIENT_THUMBNAIL_HPP
#define _X_CLIENT_THUMBNAIL_HPP

#include <memory>
#include <xcb/xcb.h>
#include <xcb/damage.h>

#include "thumbnail_t.hpp"
#include "x_event_handler_t.hpp"
#include "x_client.hpp"
#include "x_connection.hpp"

// http://svn.enlightenment.org/svn/e/tags/evas-1.0.2/src/modules/engines/xrender_x11/evas_engine_xcb_render.c
#define DOUBLE_TO_FIXED(d) ((xcb_render_fixed_t) ((d) * 65536))

class x_client_thumbnail : public thumbnail_t {
  public:
    x_client_thumbnail(x_connection & c,
                       const rectangle_t & rectangle,
                       const xcb_window_t & window = XCB_NONE,
                       std::shared_ptr<x_client> xclient = NULL);

    ~x_client_thumbnail(void);

  private:
    typedef std::shared_ptr<x_client> x_client_ptr;

    x_connection & _c;
    x_client_ptr _x_client;

    double _scale;
    rectangle_t _rectangle;

    xcb_window_t _preview;
    xcb_damage_damage_t _damage;
};

#endif
