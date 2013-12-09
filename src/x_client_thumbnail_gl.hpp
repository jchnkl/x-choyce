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
                       const gl::config & config,
                       const rectangle & rect,
                       const xcb_window_t & window = XCB_NONE);

    ~x_client_thumbnail(void);

    bool operator==(const x_client_thumbnail & other)
    {
      return m_thumbnail_window == other.m_thumbnail_window;
    }

    thumbnail_t & show(void);
    thumbnail_t & hide(void);
    thumbnail_t & raise(void);
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
        gl::config m_gl_config;
    };

  private:
    x_connection & m_c;
    x::xrm & m_xrm;
    const gl::api & m_gl_api;
    gl::context m_gl_ctx;
    x_client m_x_client;
    x_client_icon m_x_client_icon;
    x_client_name m_x_client_name;

    double m_scale;

    std::pair<double, double> m_icon_scale;
    std::pair<double, double> m_icon_offset;
    std::pair<double, double> m_title_scale;
    std::pair<double, double> m_title_offset;

    bool m_visible = false;
    bool m_highlight = false;

    bool m_icon_update = true;
    bool m_title_update = true;
    bool m_rectangle_update = true;
    bool m_highlight_update = true;
    bool m_name_window_update = true;

    rectangle m_rectangle;

    xcb_window_t m_thumbnail_window = XCB_NONE;
    xcb_damage_damage_t m_damage;

    // >> config options

    unsigned int m_icon_size;
    unsigned int m_border_width;

    // red, green, blue, alpha
    std::tuple<double, double, double, double> m_focused_border_color;
    std::tuple<double, double, double, double> m_unfocused_border_color;

    // << config options

    void load_config(void);
    void update_highlight(void);
    void update_icon_pixmap(void);
    void update_title_pixmap(void);
    void configure_thumbnail(void);
    void update_name_window_pixmap(void);
    void update_uniforms(const GLuint & program);
    void update(int x, int y, unsigned int width, unsigned int height);
};

bool
operator==(const x_client_thumbnail & thumbnail, const xcb_window_t & window);

bool
operator==(const xcb_window_t & window, const x_client_thumbnail & thumbnail);

#endif
