#include <climits>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/damage.h>
#include <xcb/composite.h>

// http://svn.enlightenment.org/svn/e/tags/evas-1.0.2/src/modules/engines/xrender_x11/evas_engine_xcb_render.c
#define DOUBLE_TO_FIXED(d) ((xcb_render_fixed_t) ((d) * 65536))

class x_connection;
class x_client;

xcb_render_pictformat_t
render_find_visual_format(const x_connection & c, xcb_visualid_t visual);

xcb_render_picture_t
make_picture(const x_connection & c, xcb_window_t window);

std::vector<xcb_window_t>
stacked_window_list(const x_connection & c);

std::vector<x_client>
make_thumbnails(const x_connection & c, const std::vector<xcb_window_t> & windows);

struct dimension_t {
  dimension_t(void) {}
  dimension_t(unsigned int w, unsigned int h) : width(w), height(h) {}
  unsigned int width, height;
};

struct position_t {
  position_t(void) {}
  position_t(int x, int y) : x(x), y(y) {}
  int x, y;
};

struct rectangle_t {
  rectangle_t(void) {}
  rectangle_t(dimension_t size, position_t position)
    : size(size), position(position) {}
  dimension_t size;
  position_t position;
};

class x_event_handler {
  public:
    virtual void handle(xcb_generic_event_t *) = 0;
};

class x_connection {
  public:
    x_connection(void)
    {
      _c = xcb_connect(NULL, &_screen_number);
      find_default_screen();
      _root_window = _default_screen->root;
      init_damage();
      init_render();
    }

    ~x_connection(void) {
      xcb_disconnect(_c);
      for (auto er : _extension_reply_list) {
        delete er;
      }
    }

    xcb_connection_t * operator()(void) const { return _c; }

    void register_x_event_handler(x_event_handler * eh)
    {
      _x_event_handler_list.push_back(eh);
    }

    void run_event_loop(void)
    {
      xcb_generic_event_t * ge = NULL;
      while (true) {
        flush();
        ge = xcb_wait_for_event(_c);

        if (! ge) {
          continue;
        } else {
          for (auto eh : _x_event_handler_list) {
            eh->handle(ge);
          }
          delete ge;
        }
      }
    }

    xcb_visualtype_t * default_visual_of_screen(void)
    {
      xcb_depth_iterator_t depth_iterator =
        xcb_screen_allowed_depths_iterator(_default_screen);

      if (depth_iterator.data) {
        for (; depth_iterator.rem; xcb_depth_next(&depth_iterator)) {
          xcb_visualtype_iterator_t visual_iterator =
            xcb_depth_visuals_iterator(depth_iterator.data);
          for (; visual_iterator.rem; xcb_visualtype_next(&visual_iterator)) {
            if (_default_screen->root_visual == visual_iterator.data->visual_id) {
              return visual_iterator.data;
            }
          }
        }
      }

      return NULL;
    }

    void flush(void) const { xcb_flush(_c); }

    xcb_screen_t * const default_screen(void) const
    {
      return _default_screen;
    }

    xcb_window_t const & root_window(void) const
    {
      return _root_window;
    }

    uint8_t damage_event_id(void) const { return _damage_event_id; }

  private:
    uint8_t _damage_event_id;
    int _screen_number = 0;
    xcb_window_t _root_window = 0;
    xcb_connection_t * _c = NULL;
    xcb_screen_t * _default_screen = NULL;

    std::vector<x_event_handler *> _x_event_handler_list;
    std::vector<const xcb_query_extension_reply_t *> _extension_reply_list;


    void find_default_screen(void)
    {
      int screen = _screen_number;
      xcb_screen_iterator_t iter;
      iter = xcb_setup_roots_iterator(xcb_get_setup(_c));
      for (; iter.rem; --screen, xcb_screen_next(&iter))
        if (screen == 0) {
          _default_screen = iter.data;
          return;
        }

      throw std::string("could not find default screen");
    }

    void init_damage(void)
    {
      xcb_prefetch_extension_data(_c, &xcb_damage_id);

      const xcb_query_extension_reply_t * extension_reply =
        xcb_get_extension_data(_c, &xcb_damage_id);

      _damage_event_id = extension_reply->first_event + XCB_DAMAGE_NOTIFY;

      _extension_reply_list.push_back(extension_reply);

      // necessary to get xdamage of the ground
      xcb_damage_query_version(_c, XCB_DAMAGE_MAJOR_VERSION,
                                    XCB_DAMAGE_MINOR_VERSION);
    }

    void init_render(void) { xcb_prefetch_extension_data(_c, &xcb_render_id); }

};

class x_client : public x_event_handler {
  public:
    x_client(const x_connection & c, xcb_window_t window)
      : _c(c), _window(window)
    {
      std::cerr << "c'tor for 0x" << std::hex << window << std::dec;
      std::cerr << " and " << this << std::endl;
      // const_cast<x_connection &>(_c).register_x_event_handler(this);
      // c.register_x_event_handler(this);
      _window_pixmap = xcb_generate_id(_c());
      xcb_composite_name_window_pixmap(_c(), _window, _window_pixmap);

      uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT;
      uint32_t values[] = { 0, true };
      _parent = xcb_generate_id(c());
      xcb_create_window(_c(), XCB_COPY_FROM_PARENT, _parent,
                        _c.default_screen()->root,
                        0, 0, 1, 1, 0,
                        // _position.x, _position.y, _size.width, _size.height, 0,
                        XCB_WINDOW_CLASS_INPUT_OUTPUT,
                        _c.default_screen()->root_visual, mask, values);

      _parent_pixmap = xcb_generate_id(_c());
      xcb_composite_name_window_pixmap(_c(), _parent, _parent_pixmap);

      _damage = xcb_generate_id(_c());
      xcb_damage_create(_c(), _damage, _window,
                        XCB_DAMAGE_REPORT_LEVEL_BOUNDING_BOX);
    }

    double &       scale(void)    { return _scale; }
    dimension_t &         size(void)     { return _size; }
    position_t &     position(void) { return _position; }
    xcb_window_t & window(void)   { return _window; }

    void handle(xcb_generic_event_t * ge)
    {
      if (_c.damage_event_id() == (ge->response_type & ~0x80)) {
        xcb_damage_notify_event_t * e = (xcb_damage_notify_event_t *)ge;
        xcb_damage_subtract(_c(), e->damage, XCB_NONE, XCB_NONE);
        update();
      }
    }

    void render(void)
    {
      xcb_get_geometry_reply_t * geometry_reply =
        xcb_get_geometry_reply(_c(), xcb_get_geometry(_c(), _window), NULL);

      _size.width  = geometry_reply->width  * _scale;
      _size.height = geometry_reply->height * _scale;

      delete geometry_reply;

      uint32_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
                    | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT
                    | XCB_CONFIG_WINDOW_STACK_MODE;
      uint32_t values[] = { (uint32_t)_position.x, (uint32_t)_position.y,
                            _size.width, _size.height,
                            XCB_STACK_MODE_ABOVE };

      xcb_configure_window(_c(), _parent, mask, values);
      xcb_map_window(_c(), _parent);

      _window_picture = make_picture(_c, _window);
      _parent_picture = make_picture(_c, _parent);

      // xcb_render_picture_t _src = make_picture(_c, _window);
      // xcb_render_picture_t _dst = make_picture(_c, _parent);

      xcb_render_transform_t transform_matrix =
        { DOUBLE_TO_FIXED(1), DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(     0)
        , DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(1), DOUBLE_TO_FIXED(     0)
        , DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(_scale)
        };

      // xcb_render_set_picture_transform(_c(), _src, transform_matrix);
      xcb_render_set_picture_transform(_c(), _window_picture, transform_matrix);

      // update();
    }

    void update(void)
    {
      // xcb_render_picture_t src = make_picture(_c, _window);

      uint8_t op = XCB_RENDER_PICT_OP_OVER;
      xcb_render_composite(_c(), op,
                           // _src, XCB_NONE, _dst,
                           _window_picture, XCB_NONE, _parent_picture,
                           // int16_t src_x, int16_t src_y,
                           0, 0,
                           // int16_t mask_x, int16_t mask_y,
                           0, 0,
                           // int16_t dst_x, int16_t dst_y,
                           0, 0,
                           // _position.x, _position.y,
                           // uint16_t width, uint16_t height
                           _size.width, _size.height);
    }

  private:
    const x_connection & _c;
    double _scale;
    dimension_t _size;
    position_t _position;
    xcb_window_t _window;
    xcb_window_t _parent;
    xcb_pixmap_t _window_pixmap;
    xcb_pixmap_t _parent_pixmap;
    xcb_render_picture_t _window_picture;
    xcb_render_picture_t _parent_picture;
    xcb_damage_damage_t _damage;
};

class x_user_input : public x_event_handler {
  public:
    x_user_input(const x_connection & c) : _c(c) {}

    void grab_keyboard(void)
    {
      xcb_grab_keyboard_cookie_t grab_keyboard_cookie =
        xcb_grab_keyboard(_c(), false, _c.root_window(), XCB_TIME_CURRENT_TIME,
                          XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
      xcb_grab_keyboard_reply_t * grab_keyboard_reply =
        xcb_grab_keyboard_reply(_c(), grab_keyboard_cookie, NULL);
      delete grab_keyboard_reply;
    }

    void handle(xcb_generic_event_t * ge)
    {
      if (XCB_KEY_PRESS == (ge->response_type & ~0x80)) {
        std::exit(EXIT_SUCCESS);
      }
    }

  private:
    const x_connection & _c;
};

xcb_render_pictformat_t
render_find_visual_format(const x_connection & c, xcb_visualid_t visual)
{
  // http://lists.freedesktop.org/archives/xcb/2004-December/000236.html

  xcb_render_query_pict_formats_reply_t * query_pict_formats_reply =
    xcb_render_query_pict_formats_reply(
        c(), xcb_render_query_pict_formats(c()), NULL);

  xcb_render_pictscreen_iterator_t pictscreen_iterator =
    xcb_render_query_pict_formats_screens_iterator(query_pict_formats_reply);

  while (pictscreen_iterator.rem) {
    xcb_render_pictdepth_iterator_t pictdepth_iterator =
      xcb_render_pictscreen_depths_iterator(pictscreen_iterator.data);

    while (pictdepth_iterator.rem) {
      xcb_render_pictvisual_iterator_t pictvisual_iterator =
        xcb_render_pictdepth_visuals_iterator(pictdepth_iterator.data);

      while (pictvisual_iterator.rem) {
        if (pictvisual_iterator.data->visual == visual) {
          delete query_pict_formats_reply;
          return pictvisual_iterator.data->format;
        }
        xcb_render_pictvisual_next(&pictvisual_iterator);
      }
      xcb_render_pictdepth_next(&pictdepth_iterator);
    }
    xcb_render_pictscreen_next(&pictscreen_iterator);
  }

  delete query_pict_formats_reply;
  return 0;
}

xcb_render_picture_t
make_picture(const x_connection & c, xcb_window_t window)
{
  xcb_get_window_attributes_reply_t * window_attributes_reply =
    xcb_get_window_attributes_reply(c(),
                                    xcb_get_window_attributes(c(), window),
                                    NULL);

  xcb_render_pictformat_t format =
    render_find_visual_format(c, window_attributes_reply->visual);

  delete window_attributes_reply;

  uint32_t mask = XCB_RENDER_CP_REPEAT;
  uint32_t list[] = { XCB_RENDER_REPEAT_NONE };

  xcb_render_picture_t picture = xcb_generate_id(c());
  xcb_render_create_picture(c(), picture, window, format, mask, list);

  return picture;
}

std::vector<xcb_window_t>
stacked_window_list(const x_connection & c)
{
  std::string atom_name = "_NET_CLIENT_LIST_STACKING";
  xcb_intern_atom_cookie_t atom_cookie =
    xcb_intern_atom(c(), false, atom_name.length(), atom_name.c_str());
  xcb_intern_atom_reply_t * atom_reply =
    xcb_intern_atom_reply(c(), atom_cookie, NULL);

  xcb_get_property_cookie_t property_cookie =
    xcb_get_property(c(), false, c.root_window(),
                     atom_reply->atom, XCB_ATOM_WINDOW, 0, UINT_MAX);

  xcb_get_property_reply_t * property_reply =
    xcb_get_property_reply(c(), property_cookie, NULL);

  xcb_window_t * windows =
    (xcb_window_t *)xcb_get_property_value(property_reply);

  return std::vector<xcb_window_t>(windows, windows + property_reply->length);
}

std::vector<x_client>
make_x_clients(x_connection & c, const std::vector<xcb_window_t> & windows)
{
  std::vector<x_client> thumbnails;
  for (auto & window : windows) { thumbnails.emplace_back(c, window); }
  return thumbnails;
}

xcb_visualtype_t *
argb_visual(const x_connection & c)
{
  xcb_depth_iterator_t depth_iter =
    xcb_screen_allowed_depths_iterator(c.default_screen());

  if (depth_iter.data) {
    for (; depth_iter.rem; xcb_depth_next (&depth_iter)) {
      if (depth_iter.data->depth == 32) {
        xcb_visualtype_iterator_t visual_iter = xcb_depth_visuals_iterator(depth_iter.data);
        for (; visual_iter.rem; xcb_visualtype_next(&visual_iter)) {
          return visual_iter.data;
        }
      }
    }
  }

  return NULL;
}

int main(int argc, char ** argv)
{
  x_connection c;

  auto x_clients = make_x_clients(c, stacked_window_list(c));

  int xo = 0, yo = 0;
  for (auto & xc : x_clients) {
    c.register_x_event_handler(&xc);
    xc.scale() = 0.2;
    if (xo * 300 + 10 > 1200) { xo = 0; ++yo; }
    xc.position() = position_t(xo++ * 300 + 10, yo * 300 + 10);
    xc.render();
  }

  x_user_input xui(c);
  xui.grab_keyboard();
  c.register_x_event_handler(&xui);

  c.run_event_loop();

  return 0;
}
