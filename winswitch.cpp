#include <climits>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <X11/keysym.h>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/damage.h>
#include <xcb/xinerama.h>
#include <xcb/composite.h>

// http://svn.enlightenment.org/svn/e/tags/evas-1.0.2/src/modules/engines/xrender_x11/evas_engine_xcb_render.c
#define DOUBLE_TO_FIXED(d) ((xcb_render_fixed_t) ((d) * 65536))

class x_connection;
class x_client;

xcb_render_pictformat_t
render_find_visual_format(const x_connection & c, xcb_visualid_t visual);

xcb_render_picture_t
make_picture(const x_connection & c, xcb_window_t window);

std::list<x_client>
make_x_clients(const x_connection & c, const std::vector<xcb_window_t> & windows);

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
  rectangle_t(position_t position, dimension_t dimension)
    : position(position), dimension(dimension) {}
  rectangle_t(int x, int y, unsigned int width, unsigned int height)
    : position(x, y), dimension(width, height) {}

  int & x(void) { return position.x; }
  int const & x(void) const { return position.x; }
  int & y(void) { return position.y; }
  int const & y(void) const { return position.y; }
  unsigned int & width(void) { return dimension.width; }
  unsigned int const & width(void) const { return dimension.width; }
  unsigned int & height(void) { return dimension.height; }
  unsigned int const & height(void) const { return dimension.height; }

  position_t position;
  dimension_t dimension;
};

std::ostream & operator<<(std::ostream & os, const rectangle_t & rectangle)
{
  return os << rectangle.x() << "x" << rectangle.y() << "+"
            << rectangle.width() << "+" << rectangle.height();
}

class x_event_handler {
  public:
    virtual void handle(xcb_generic_event_t *) = 0;
};

class x_connection : public x_event_handler {
  public:
    x_connection(void)
    {
      _c = xcb_connect(NULL, &_screen_number);
      find_default_screen();
      _root_window = _default_screen->root;
      init_damage();
      init_render();
      init_xinerama();
      select_input(_root_window, XCB_EVENT_MASK_STRUCTURE_NOTIFY);
    }

    ~x_connection(void) {
      xcb_disconnect(_c);
      for (auto er : _extension_reply_list) {
        delete er;
      }
    }

    xcb_connection_t * operator()(void) const { return _c; }

    void select_input(xcb_window_t window, xcb_event_mask_t event_mask) const
    {
      uint32_t mask = XCB_CW_EVENT_MASK;
      const uint32_t values[] = { event_mask };
      xcb_change_window_attributes(_c, window, mask, values);
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

    void grab_key(uint16_t modifiers, xcb_keysym_t keysym) const
    {
      xcb_keycode_t keycode = keysym_to_keycode(keysym);
      if (keycode != 0) {
        xcb_grab_key(_c, false, _root_window, modifiers, keycode,
                     XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
      }
    }

    void grab_keyboard(void) const
    {
      xcb_grab_keyboard_cookie_t grab_keyboard_cookie =
        xcb_grab_keyboard(_c, false, root_window(), XCB_TIME_CURRENT_TIME,
                          XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
      xcb_grab_keyboard_reply_t * grab_keyboard_reply =
        xcb_grab_keyboard_reply(_c, grab_keyboard_cookie, NULL);
      delete grab_keyboard_reply;
    }

    void ungrab_keyboard(void) const
    {
      xcb_ungrab_keyboard(_c, XCB_TIME_CURRENT_TIME);
    }

    xcb_keysym_t
    keycode_to_keysym(xcb_keycode_t keycode) const
    {
      xcb_key_symbols_t * keysyms;
      xcb_keysym_t keysym;

      if (!(keysyms = xcb_key_symbols_alloc(_c))) { return 0; }
      keysym = xcb_key_symbols_get_keysym(keysyms, keycode, 0);
      xcb_key_symbols_free(keysyms);

      return keysym;
    }

    xcb_keycode_t
    keysym_to_keycode(xcb_keysym_t keysym) const
    {
      xcb_key_symbols_t * keysyms;
      xcb_keycode_t keycode, * keycode_reply;

      if (!(keysyms = xcb_key_symbols_alloc(_c))) { return 0; }
      keycode_reply = xcb_key_symbols_get_keycode(keysyms, keysym);
      xcb_key_symbols_free(keysyms);

      keycode = *keycode_reply;
      delete keycode_reply;

      return keycode;
    }

    std::vector<xcb_window_t>
    net_client_list_stacking(void) const
    {
      std::string atom_name = "_NET_CLIENT_LIST_STACKING";
      xcb_intern_atom_cookie_t atom_cookie =
        xcb_intern_atom(_c, false, atom_name.length(), atom_name.c_str());
      xcb_intern_atom_reply_t * atom_reply =
        xcb_intern_atom_reply(_c, atom_cookie, NULL);

      xcb_get_property_cookie_t property_cookie =
        xcb_get_property(_c, false, _root_window,
                         atom_reply->atom, XCB_ATOM_WINDOW, 0, UINT_MAX);

      delete atom_reply;

      xcb_get_property_reply_t * property_reply =
        xcb_get_property_reply(_c, property_cookie, NULL);

      xcb_window_t * windows =
        (xcb_window_t *)xcb_get_property_value(property_reply);

      std::vector<xcb_window_t> result;
      for (int i = property_reply->length - 1; i >= 0; --i) {
        result.push_back(windows[i]);
      }

      delete property_reply;
      return result;
    }

    xcb_atom_t
    intern_atom(const std::string & atom_name) const
    {
      xcb_intern_atom_cookie_t atom_cookie =
        xcb_intern_atom(_c, false, atom_name.length(), atom_name.c_str());
      xcb_intern_atom_reply_t * atom_reply =
        xcb_intern_atom_reply(_c, atom_cookie, NULL);

      if (atom_reply) {
        xcb_atom_t atom = atom_reply->atom;
        delete atom_reply;
        return atom;
      }

      return XCB_ATOM_NONE;
    }

    xcb_window_t
    net_active_window(void) const
    {
      xcb_atom_t atom = intern_atom("_NET_ACTIVE_WINDOW");
      if (atom == XCB_ATOM_NONE) { return XCB_NONE; }

      xcb_get_property_cookie_t property_cookie =
        xcb_get_property(_c, false, _root_window, atom, XCB_ATOM_WINDOW, 0, 32);

      xcb_generic_error_t * error = NULL;
      xcb_get_property_reply_t * property_reply =
        xcb_get_property_reply(_c, property_cookie, &error);

      xcb_window_t active_window;
      if (error || property_reply->value_len == 0) {
        delete error;
        active_window = XCB_NONE;
      } else {
        active_window = *(xcb_window_t *)xcb_get_property_value(property_reply);
      }

      delete property_reply;
      return active_window;
    }

    void request_change_current_desktop(unsigned int desktop_id) const
    {
      xcb_client_message_event_t event;
      memset(&event, 0, sizeof(xcb_client_message_event_t));

      event.response_type = XCB_CLIENT_MESSAGE;
      event.format = 32;
      event.type = intern_atom("_NET_CURRENT_DESKTOP");

      event.data.data32[0] = desktop_id;
      event.data.data32[1] = XCB_TIME_CURRENT_TIME;

      uint32_t mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
                    | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;

      xcb_send_event(_c, false, _root_window, mask, (const char *)&event);
    }

    void request_change_active_window(xcb_window_t window) const
    {
      xcb_client_message_event_t event;
      memset(&event, 0, sizeof(xcb_client_message_event_t));

      event.response_type = XCB_CLIENT_MESSAGE;
      event.format = 32;
      event.window = window;
      event.type = intern_atom("_NET_ACTIVE_WINDOW");

      // source indication; 1 == application, 2 == pagers and other clients
      event.data.data32[0] = 2;
      event.data.data32[1] = XCB_TIME_CURRENT_TIME;
      event.data.data32[2] = XCB_NONE;

      uint32_t mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
                    | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;

      xcb_send_event(_c, false, _root_window, mask, (const char *)&event);
    }


    rectangle_t current_screen(void) const
    {
      xcb_get_geometry_cookie_t geometry_cookie =
        xcb_get_geometry(_c, net_active_window());
      xcb_get_geometry_reply_t * geometry_reply =
        xcb_get_geometry_reply(_c, geometry_cookie, NULL);

      if (geometry_reply) {
        int cx = geometry_reply->x + geometry_reply->width / 2;
        int cy = geometry_reply->y + geometry_reply->height / 2;
        delete geometry_reply;
        for (auto & screen : _screens) {
          if (cx >= screen.x() && cx <= screen.x() + (int)screen.width()
              && cy >= screen.y() && cy <= screen.y() + (int)screen.height()) {
            return screen;
          }
        }
      }

      return rectangle_t(0, 0, 800, 600);
    }

    void handle(xcb_generic_event_t * ge)
    {
      if (XCB_CONFIGURE_NOTIFY == (ge->response_type & ~0x80)) {
        xcb_configure_request_event_t * e = (xcb_configure_request_event_t *)ge;
        if (e->window == _root_window) {
          update_xinerama();
        }
      }
    }

  private:
    uint8_t _damage_event_id;
    int _screen_number = 0;
    xcb_window_t _root_window = 0;
    xcb_connection_t * _c = NULL;
    xcb_screen_t * _default_screen = NULL;

    std::vector<rectangle_t> _screens;
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

    void init_xinerama(void)
    {
      xcb_prefetch_extension_data(_c, &xcb_xinerama_id);
      xcb_xinerama_query_version(_c, XCB_XINERAMA_MAJOR_VERSION,
                                     XCB_XINERAMA_MINOR_VERSION);

      xcb_xinerama_is_active_cookie_t is_active_cookie =
        xcb_xinerama_is_active(_c);
      xcb_xinerama_is_active_reply_t * is_active_reply =
        xcb_xinerama_is_active_reply(_c, is_active_cookie, NULL);

      if (is_active_reply && is_active_reply->state) {
        delete is_active_reply;
        update_xinerama();
      }
    }

    void update_xinerama(void)
    {
      xcb_xinerama_query_screens_cookie_t query_screens_cookie =
        xcb_xinerama_query_screens(_c);

      xcb_xinerama_query_screens_reply_t * query_screens_reply =
        xcb_xinerama_query_screens_reply(_c, query_screens_cookie, NULL);

      if (query_screens_reply) {
        _screens.clear();
        xcb_xinerama_screen_info_t * screen_info =
          xcb_xinerama_query_screens_screen_info(query_screens_reply);
        int length =
          xcb_xinerama_query_screens_screen_info_length(query_screens_reply);
        for (int i = 0; i < length; ++i) {
          _screens.push_back(
              rectangle_t(screen_info[i].x_org, screen_info[i].y_org,
                screen_info[i].width, screen_info[i].height));
        }
        delete query_screens_reply;
      }
    }
};

class x_event_source {
  public:
    x_event_source(const x_connection & c) : _c(c) {}

    virtual void register_handler(x_event_handler * eh)
    {
      _handler_list.push_back(eh);
    }

    virtual void unregister_handler(x_event_handler * eh)
    {
      _handler_list.remove(eh);
    }

    void run_event_loop(void)
    {
      xcb_generic_event_t * ge = NULL;
      while (true) {
        _c.flush();
        ge = xcb_wait_for_event(_c());

        if (! ge) {
          continue;
        } else {
          for (auto eh : _handler_list) { eh->handle(ge); }
          delete ge;
        }
      }
    }

  private:
    const x_connection & _c;
    std::list<x_event_handler *> _handler_list;
};

class x_client : public x_event_handler {
  public:
    friend std::ostream & operator<<(std::ostream &, const x_client &);

    x_client(const x_connection & c, xcb_window_t window)
      : _c(c), _window(window)
    {
      update_geometry();
      get_net_wm_desktop();

      uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT;
      uint32_t values[] = { 0, true };
      _preview = xcb_generate_id(c());
      xcb_create_window(_c(), XCB_COPY_FROM_PARENT, _preview,
                        _c.default_screen()->root,
                        0, 0, 1, 1, 0,
                        // _position.x, _position.y, _size.width, _size.height, 0,
                        XCB_WINDOW_CLASS_INPUT_OUTPUT,
                        _c.default_screen()->root_visual, mask, values);


      _damage = xcb_generate_id(_c());
      xcb_damage_create(_c(), _damage, _window,
                        XCB_DAMAGE_REPORT_LEVEL_BOUNDING_BOX);
    }

    ~x_client(void)
    {
      xcb_damage_destroy(_c(), _damage);
      xcb_render_free_picture(_c(), _window_picture);
      xcb_render_free_picture(_c(), _preview_picture);
      xcb_destroy_window(_c(), _preview);
    }

    double &       preview_scale(void)        { return _preview_scale; }
    rectangle_t &  rectangle(void)            { return _rectangle; }
    position_t &   preview_position(void)     { return _preview_position; }
    unsigned int   net_wm_desktop(void) const { return _net_wm_desktop; }
    xcb_window_t & window(void)               { return _window; }

    void handle(xcb_generic_event_t * ge)
    {
      if (_c.damage_event_id() == (ge->response_type & ~0x80)) {
        xcb_damage_notify_event_t * e = (xcb_damage_notify_event_t *)ge;
        xcb_damage_subtract(_c(), e->damage, XCB_NONE, XCB_NONE);
        compose();
      }
    }

    void preview(void)
    {
      xcb_get_geometry_reply_t * geometry_reply =
        xcb_get_geometry_reply(_c(), xcb_get_geometry(_c(), _window), NULL);

      delete geometry_reply;

      uint32_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
                    | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT
                    | XCB_CONFIG_WINDOW_STACK_MODE;
      uint32_t values[] = { (uint32_t)_preview_position.x,
                            (uint32_t)_preview_position.y,
                            (uint32_t)(_rectangle.width() * _preview_scale),
                            (uint32_t)(_rectangle.height() * _preview_scale),
                            XCB_STACK_MODE_ABOVE };

      xcb_configure_window(_c(), _preview, mask, values);
      xcb_map_window(_c(), _preview);

      _window_picture = make_picture(_c, _window);
      _preview_picture = make_picture(_c, _preview);

      xcb_render_transform_t transform_matrix =
        { DOUBLE_TO_FIXED(1), DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(             0)
        , DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(1), DOUBLE_TO_FIXED(             0)
        , DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(0), DOUBLE_TO_FIXED(_preview_scale)
        };

      xcb_render_set_picture_transform(_c(), _window_picture, transform_matrix);
    }

    void compose(void)
    {
      uint8_t op = XCB_RENDER_PICT_OP_OVER;
      xcb_render_composite(_c(), op,
                           _window_picture, XCB_NONE, _preview_picture,
                           // int16_t src_x, int16_t src_y,
                           0, 0,
                           // int16_t mask_x, int16_t mask_y,
                           0, 0,
                           // int16_t dst_x, int16_t dst_y,
                           0, 0,
                           // _position.x, _position.y,
                           // uint16_t width, uint16_t height
                           _rectangle.width(), _rectangle.height());
    }

    void update_geometry(void)
    {
      xcb_get_geometry_reply_t * geometry_reply =
        xcb_get_geometry_reply(_c(), xcb_get_geometry(_c(), _window), NULL);

      _rectangle.x()      = geometry_reply->x;
      _rectangle.y()      = geometry_reply->y;
      _rectangle.width()  = geometry_reply->width;
      _rectangle.height() = geometry_reply->height;

      delete geometry_reply;
    }

  private:
    const x_connection & _c;
    double _preview_scale;
    rectangle_t _rectangle;
    position_t _preview_position;
    unsigned int _net_wm_desktop;
    xcb_window_t _window;
    xcb_window_t _preview;
    xcb_render_picture_t _window_picture;
    xcb_render_picture_t _preview_picture;
    xcb_damage_damage_t _damage;

    void
    get_net_wm_desktop(void)
    {
      std::string atom_name = "_NET_WM_DESKTOP";
      xcb_intern_atom_cookie_t atom_cookie =
        xcb_intern_atom(_c(), false, atom_name.length(), atom_name.c_str());
      xcb_intern_atom_reply_t * atom_reply =
        xcb_intern_atom_reply(_c(), atom_cookie, NULL);

      xcb_get_property_cookie_t property_cookie =
        xcb_get_property(_c(), false, _c.root_window(),
                         atom_reply->atom, XCB_ATOM_WINDOW, 0, 32);

      delete atom_reply;

      xcb_get_property_reply_t * property_reply =
        xcb_get_property_reply(_c(), property_cookie, NULL);

      _net_wm_desktop = *(unsigned int *)xcb_get_property_value(property_reply);

      delete property_reply;
    }
};

std::ostream & operator<<(std::ostream & os, const x_client & xc)
{
  return os << "0x" << std::hex << xc._window << std::dec << " @ "
            << xc._rectangle.x()     << "x"
            << xc._rectangle.y()     << "+"
            << xc._rectangle.width() << "+"
            << xc._rectangle.height()
            << " on desktop " << xc._net_wm_desktop;
}

  public:

    {
      }
    }

    {
    }

    {
    }

    void handle(xcb_generic_event_t * ge)
    {
      if (XCB_KEY_PRESS == (ge->response_type & ~0x80)) {
        xcb_key_press_event_t * e = (xcb_key_press_event_t *)ge;
        xcb_keysym_t keysym = _c.keycode_to_keysym(e->detail);
        if (keysym == XK_Escape && e->state == 0) {
          std::exit(EXIT_SUCCESS);
        } else if (keysym == XK_Tab && e->state == XCB_MOD_MASK_4) {
          _c.grab_keyboard();
          std::cerr << "XK_Tab + XCB_MOD_MASK_4" << std::endl;
        }

      } else if (XCB_KEY_RELEASE == (ge->response_type & ~0x80)) {
        xcb_key_release_event_t * e = (xcb_key_release_event_t *)ge;
        xcb_keysym_t keysym = _c.keycode_to_keysym(e->detail);
        if (keysym == XK_Super_L) {
          _c.ungrab_keyboard();
          std::cerr << "release" << std::endl;
        }
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

std::list<x_client>
make_x_clients(const x_connection & c, const std::vector<xcb_window_t> & windows)
{
  std::list<x_client> x_clients;
  for (auto & window : windows) { x_clients.emplace_back(c, window); }
  return x_clients;
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
  c.grab_key(XCB_MOD_MASK_4, XK_Tab);

  c.grab_key_0, XK_Escape);

  auto x_clients = make_x_clients(c, c.net_client_list_stacking());
  x_event_source es(c);

  int xo = 0, yo = 0;
  for (auto & xc : x_clients) {
    es.register_handler(&xc);

    if (xo * 300 + 10 > 1200) { xo = 0; ++yo; }
    xc.preview_scale() = 0.2;
    xc.preview_position() = position_t(xo++ * 300 + 10, yo * 300 + 10);
    xc.preview();
  }

  x_user_input xui(c);
  es.register_handler(&xui);

  es.run_event_loop();

  return 0;
}
