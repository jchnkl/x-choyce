#ifndef _X_CLIENT_THUMBNAIL_GL_HPP
#define _X_CLIENT_THUMBNAIL_GL_HPP

#include <fstream>
#include <memory>
#include <cmath>
#include <unordered_map>
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
      return _thumbnail_window == other._thumbnail_window;
    }

    void show(void);
    void hide(void);
    void select(void);
    void update(void);
    void update(const rectangle &);
    const xcb_window_t & id(void);
    const xcb_window_t & window(void);
    void highlight(bool want_highlight);

    bool handle(xcb_generic_event_t * ge);

    class factory : public thumbnail_t::factory {
      public:
        thumbnail_t::ptr
          make(x_connection &, const xcb_window_t &, const rectangle &) const;
    };

  private:
    typedef std::shared_ptr<x_client> x_client_ptr;

    x_connection & _c;
    x_client_ptr _x_client;

    int _border_width = 4;

    // red, green, blue, alpha
    std::tuple<double, double, double, double> _focused_border_color =
      std::make_tuple(0.855, 0.648, 0.125, 0.75); // goldenrod
    std::tuple<double, double, double, double> _unfocused_border_color =
      std::make_tuple(0.25, 0.25, 0.25, 0.5);

    double _scale;
    bool _highlight = false, _visible = false, _purge = false;
    rectangle _rectangle;

    xcb_window_t _thumbnail_window;
    xcb_damage_damage_t _damage;

    GLXContext _gl_ctx;
    GLXPixmap _thumbnail_gl_pixmap;
    std::unordered_map<std::string, GLuint> _programs;

    void purge(void);
    void update(int x, int y, unsigned int width, unsigned int height);
    void configure_thumbnail_window(void);
    void configure_gl(XVisualInfo * vi = NULL);
    void init_gl_shader(void);
    void release_gl(void);
};

bool
operator==(const x_client_thumbnail & thumbnail, const xcb_window_t & window);

bool
operator==(const xcb_window_t & window, const x_client_thumbnail & thumbnail);

#endif
