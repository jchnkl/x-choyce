#ifndef X_CLIENT_THUMBNAIL_HPP
#define X_CLIENT_THUMBNAIL_HPP

#include <memory>
#include <xcb/xcb.h>
#include <xcb/damage.h>

#include "thumbnail_t.hpp"
#include "x_event_handler_t.hpp"
#include "x_client.hpp"
#include "x_connection.hpp"

// http://svn.enlightenment.org/svn/e/tags/evas-1.0.2/src/modules/engines/xrender_x11/evas_engine_xcb_render.c
#define DOUBLE_TO_FIXED(d) ((xcb_render_fixed_t) ((d) * 65536))

class x_client_thumbnail : public x_event_handler_t
                         , public thumbnail_t {
  public:
    friend bool operator==(const x_client_thumbnail &, const xcb_window_t &);
    friend bool operator==(const xcb_window_t &, const x_client_thumbnail &);

    x_client_thumbnail(x_connection & c,
                       const rectangle & rect,
                       const xcb_window_t & window = XCB_NONE,
                       std::shared_ptr<x_client> xclient = NULL);

    ~x_client_thumbnail(void);

    bool operator==(const x_client_thumbnail & other)
    {
      return m_thumbnail_window == other.m_thumbnail_window;
    }

    void show(void);
    void hide(void);
    void select(void);
    void update(void);
    void highlight(bool want_highlight);

    bool handle(xcb_generic_event_t * ge);

    void update_rectangle(const rectangle & rect);

  private:
    typedef std::shared_ptr<x_client> x_client_ptr;

    x_connection & m_c;
    x_client_ptr m_x_client;

    double m_scale;
    rectangle m_rectangle;

    xcb_window_t m_thumbnail_window;
    xcb_damage_damage_t m_damage;
    xcb_render_picture_t m_alpha_picture;
    xcb_render_picture_t m_window_picture;
    xcb_render_picture_t m_thumbnail_picture;

    uint16_t m_alpha_value = (uint16_t)(0.75f * 0xffff);

    void update(int x, int y, unsigned int width, unsigned int height);
    void configure_thumbnail_window(void);
    void configure_thumbnail_picture(void);
    void configure_alpha_picture(uint16_t alpha_value);
};

bool
operator==(const x_client_thumbnail & thumbnail, const xcb_window_t & window);

bool
operator==(const xcb_window_t & window, const x_client_thumbnail & thumbnail);

#endif
