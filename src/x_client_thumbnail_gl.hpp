#ifndef X_CLIENT_THUMBNAIL_GL_HPP
#define X_CLIENT_THUMBNAIL_GL_HPP

#include <fstream>
#include <unordered_map>
#include <xcb/xcb.h>
#include <xcb/damage.h>

#include "opengl.hpp"
#include "x_xrm.hpp"
#include "observer.hpp"
#include "thumbnail_t.hpp"
#include "x_event_handler_t.hpp"
#include "x_client.hpp"
#include "x_client_icon.hpp"
#include "x_client_name.hpp"

// http://svn.enlightenment.org/svn/e/tags/evas-1.0.2/src/modules/engines/xrender_x11/evas_engine_xcb_render.c
#define DOUBLE_TO_FIXED(d) ((xcb_render_fixed_t) ((d) * 65536))

class x_connection;

class x_client_thumbnail : public x_event_handler_t
                         , public thumbnail_t
                         , public observer<x::xrm>
                         , public observer<x_client>
                         , public observer<x_client_name>
{
  public:
    friend bool operator==(const x_client_thumbnail &, const xcb_window_t &);
    friend bool operator==(const xcb_window_t &, const x_client_thumbnail &);

    x_client_thumbnail(x_connection & c,
                       x::xrm & xrm,
                       const x::gl::api & api,
                       const rectangle & rect,
                       const xcb_window_t & window = XCB_NONE);

    ~x_client_thumbnail(void);

    bool operator==(const x_client_thumbnail & other)
    {
      return _thumbnail_window == other._thumbnail_window;
    }

    thumbnail_t & show(void);
    thumbnail_t & hide(void);
    thumbnail_t & select(void);
    thumbnail_t & update(void);
    thumbnail_t & update(const rectangle &);
    const rectangle & rect(void);
    const xcb_window_t & id(void);
    const xcb_window_t & window(void);
    thumbnail_t & highlight(bool want_highlight);

    bool handle(xcb_generic_event_t * ge);
    void notify(x::xrm *);
    void notify(x_client *);
    void notify(x_client_name *);

    class factory : public thumbnail_t::factory {
      public:
        factory(x_connection & c, x::xrm & xrm);
        thumbnail_t::ptr
          make(const xcb_window_t &, const rectangle &) const;

      private:
        x_connection & m_c;
        x::xrm & m_xrm;
        x::gl::api _gl_api;
    };

  private:
    x_connection & m_c;
    x::xrm & m_xrm;
    const x::gl::api & _gl_api;
    x::gl::context _gl_ctx;
    x_client _x_client;
    x_client_icon _x_client_icon;
    x_client_name _x_client_name;

    double _scale;
    double _icon_scale_x;
    double _icon_scale_y;
    double _title_scale_x;
    double _title_scale_y;

    bool _visible = false;
    bool _highlight = false;
    bool _update_title_pixmap = false;
    bool _update_name_window_pixmap = false;
    bool _configure_thumbnail = true;
    bool _configure_highlight = true;

    rectangle m_rectangle;

    xcb_window_t _thumbnail_window = XCB_NONE;
    xcb_damage_damage_t _damage;

    // >> config options

    unsigned int _icon_size;
    unsigned int _border_width;

    // red, green, blue, alpha
    std::tuple<double, double, double, double> _focused_border_color;
    std::tuple<double, double, double, double> _unfocused_border_color;

    // << config options

    void update(int x, int y, unsigned int width, unsigned int height);
    void update_title_pixmap(void);
    void update_name_window_pixmap(void);
    void configure_highlight(bool now = false);
    void configure_thumbnail_window(bool now = false);
    void load_config(void);
};

bool
operator==(const x_client_thumbnail & thumbnail, const xcb_window_t & window);

bool
operator==(const xcb_window_t & window, const x_client_thumbnail & thumbnail);

#endif
