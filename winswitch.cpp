#include <climits>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <list>
#include <deque>
#include <vector>
#include <X11/keysym.h>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/damage.h>
#include <xcb/xinerama.h>
#include <xcb/composite.h>

#include "data_types.hpp"
#include "x_event_handler.hpp"

// http://svn.enlightenment.org/svn/e/tags/evas-1.0.2/src/modules/engines/xrender_x11/evas_engine_xcb_render.c
#define DOUBLE_TO_FIXED(d) ((xcb_render_fixed_t) ((d) * 65536))

class x_client;
class x_connection;
class x_event_source;

xcb_render_pictformat_t
render_find_visual_format(const x_connection & c, xcb_visualid_t visual);

xcb_render_picture_t
make_picture(const x_connection & c, xcb_window_t window);

std::list<x_client>
make_x_clients(const x_connection & c, const std::vector<xcb_window_t> & windows);

std::vector<x_client>
make_thumbnails(const x_connection & c, const std::vector<xcb_window_t> & windows);

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
                        XCB_DAMAGE_REPORT_LEVEL_NON_EMPTY);
    }

    ~x_client(void)
    {
      xcb_damage_destroy(_c(), _damage);
      xcb_render_free_picture(_c(), _window_picture);
      xcb_render_free_picture(_c(), _preview_picture);
      xcb_destroy_window(_c(), _preview);
    }

    bool operator==(const xcb_window_t & window) { return _window == window; }

    double &       preview_scale(void)        { return _preview_scale; }
    rectangle_t &  rectangle(void)            { return _rectangle; }
    position_t &   preview_position(void)     { return _preview_position; }
    rectangle_t &  preview_rectangle(void)    { return _preview_rectangle; }
    unsigned int   net_wm_desktop(void) const { return _net_wm_desktop; }
    xcb_window_t & window(void)               { return _window; }

    void handle(xcb_generic_event_t * ge)
    {
      if (_c.damage_event_id() == (ge->response_type & ~0x80)) {
        xcb_damage_notify_event_t * e = (xcb_damage_notify_event_t *)ge;
        xcb_damage_subtract(_c(), e->damage, XCB_NONE, XCB_NONE);
        compose(rectangle_t(e->area.x, e->area.y, e->area.width, e->area.height));
      }
    }

    void hide_preview(void) const {
      xcb_unmap_window(_c(), _preview);
      xcb_render_free_picture(_c(), _window_picture);
      xcb_render_free_picture(_c(), _preview_picture);
    }

    void update_preview(bool is_active)
    {
      _preview_is_active = is_active;
      compose(rectangle_t(0, 0, _rectangle.width(), _rectangle.height()));
    }

    void show_preview(bool is_active)
    {
      _preview_is_active = is_active;

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

      compose(rectangle_t(0, 0, _rectangle.width(), _rectangle.height()));
    }

    void compose(const rectangle_t & rectangle)
    {
      int16_t x = rectangle.x() * _preview_scale;
      int16_t y = rectangle.y() * _preview_scale;
      uint16_t width = rectangle.width() * _preview_scale;
      uint16_t height = rectangle.height() * _preview_scale;

      uint8_t op = XCB_RENDER_PICT_OP_SRC;
      xcb_render_composite(_c(), op,
                           _window_picture, XCB_NONE, _preview_picture,
                           // int16_t src_x, int16_t src_y,
                           x, y,
                           // int16_t mask_x, int16_t mask_y,
                           0, 0,
                           // int16_t dst_x, int16_t dst_y,
                           x, y,
                           // uint16_t width, uint16_t height
                           width, height);

      if (_preview_is_active) { return; }

      op = XCB_RENDER_PICT_OP_OVER;
      xcb_render_color_t color = { (uint16_t)(0.0f * 0xffff),
                                   (uint16_t)(0.0f * 0xffff),
                                   (uint16_t)(0.0f * 0xffff),
                                   (uint16_t)(0.25f * 0xffff) };
      xcb_rectangle_t r = { x, y, width, height };
      xcb_render_fill_rectangles(_c(), op, _preview_picture, color, 1, &r);
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

    void
    get_net_wm_desktop(void)
    {
      std::string atom_name = "_NET_WM_DESKTOP";
      xcb_intern_atom_cookie_t atom_cookie =
        xcb_intern_atom(_c(), false, atom_name.length(), atom_name.c_str());
      xcb_intern_atom_reply_t * atom_reply =
        xcb_intern_atom_reply(_c(), atom_cookie, NULL);

      xcb_get_property_cookie_t property_cookie =
        xcb_get_property(_c(), false, _window,
                         atom_reply->atom, XCB_ATOM_CARDINAL, 0, 32);

      delete atom_reply;

      xcb_generic_error_t * error = NULL;
      xcb_get_property_reply_t * property_reply =
        xcb_get_property_reply(_c(), property_cookie, &error);

      if (error || property_reply->value_len == 0) {
        delete error;
        _net_wm_desktop = 0;
      } else {
        _net_wm_desktop =
          *(unsigned int *)xcb_get_property_value(property_reply);
      }

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
            << " on desktop " << xc._net_wm_desktop
            << " preview with scale "
            << xc._preview_scale << " @ "
            << xc._preview_position.x << "x"
            << xc._preview_position.y << "+"
            << (uint)(xc._rectangle.width() * xc._preview_scale) << "+"
            << (uint)(xc._rectangle.height() * xc._preview_scale)
            ;
}

class x_client_container {
  public:
    typedef std::list<x_client> container_t;
    typedef container_t::iterator iterator;
    typedef container_t::const_iterator const_iterator;
    typedef container_t::reverse_iterator reverse_iterator;
    typedef container_t::const_reverse_iterator const_reverse_iterator;

    iterator begin(void)                      { return _x_clients.begin(); }
    iterator end(void)                        { return _x_clients.end(); }
    const_iterator begin(void) const          { return _x_clients.cbegin(); }
    const_iterator end(void) const            { return _x_clients.cend(); }
    reverse_iterator rbegin(void)             { return _x_clients.rbegin(); }
    reverse_iterator rend(void)               { return _x_clients.rend(); }
    const_reverse_iterator rbegin(void) const { return _x_clients.crbegin(); }
    const_reverse_iterator rend(void) const   { return _x_clients.crend(); }
    const size_t size(void) const             { return _x_clients.size(); }

    x_client_container(const x_connection & c, x_event_source & es)
      : _c(c), _x_event_source(es) {}

    void update(void)
    {
      for (auto & xc : _x_clients) { _x_event_source.unregister_handler(&xc); }
      _x_clients.clear();
      _windows.clear();
      _windows = _c.net_client_list_stacking();
      _x_clients = make_x_clients(_c, _windows);
      for (auto & xc : _x_clients) { _x_event_source.register_handler(&xc); }
    }

  private:
    container_t _x_clients;
    std::vector<xcb_window_t> _windows;
    const x_connection & _c;
    x_event_source & _x_event_source;
};

class layout_t {
  public:
    virtual void arrange(const rectangle_t &, x_client_container &) const = 0;
};

class grid_t : public layout_t {
  public:
    void arrange(const rectangle_t & screen, x_client_container & clients) const
    {
      int gap = 5;

      int factor = std::round(std::sqrt(clients.size()));
      int rest = (factor * factor) - clients.size();

      auto cells = decompose(factor, factor * factor);

      if (rest >= 0) {
        for (auto rit = cells.rbegin(); rit != cells.rend(); ) {
          if (rest == 0) {
            break;
          } else if (rest < *rit) {
            *rit -= rest;
            break;
          } else if (rest >= *rit) {
            rest -= *rit;
            ++rit;
            cells.pop_back();
          }
        }
      } else {
        cells.push_back((-1) * rest);
      }

      int ncol = cells.size();
      int colw = screen.width() / ncol;

      std::vector<rectangle_t> rects;
      for (int c = 0; c < ncol; ++c) {
        int nrow = cells[c];
        int rowh = screen.height() / nrow;
        for (int r = 0; r < nrow; ++r) {
          rects.push_back(rectangle_t(c * colw + screen.x() + gap,
                                      r * rowh + screen.y() + gap,
                                      colw - 2 * gap, rowh - 2 * gap));
        }
      }

      int i = 0;
      for (auto & client : clients) {
        double scale_x = (double)rects[i].width()
                       / (double)client.rectangle().width();
        double scale_y = (double)rects[i].height()
                       / (double)client.rectangle().height();
        client.preview_scale() = std::min(scale_x, scale_y);
        client.preview_position().x = rects[i].x();
        client.preview_position().y = rects[i].y();

        unsigned int realwidth  = client.rectangle().width()
                                * client.preview_scale();
        unsigned int realheight = client.rectangle().height()
                                * client.preview_scale();

        if (realwidth < rects[i].width()) {
          client.preview_position().x += (rects[i].width() - realwidth) / 2;
        }
        if (realheight < rects[i].height()) {
          client.preview_position().y += (rects[i].height() - realheight) / 2;
        }
        i++;
      }
    }

  private:
    std::deque<int> decompose(int f, int n) const
    {
      std::deque<int> result;
      while (true) {
        n -= f;
        if (n > 0) {
          result.push_back(f);
        } else {
          result.push_back(n + f);
          break;
        }
      }
      return result;
    }
};

class x_clients_preview : public x_event_handler {
  public:
    x_clients_preview(const x_connection & c,
                      const layout_t * layout,
                      x_client_container & x_clients)
      : _c(c), _layout(layout), _x_clients(x_clients) {}

    void handle(xcb_generic_event_t * ge)
    {
      if (XCB_KEY_PRESS == (ge->response_type & ~0x80)) {
        xcb_key_press_event_t * e = (xcb_key_press_event_t *)ge;
        xcb_keysym_t keysym = _c.keycode_to_keysym(e->detail);
        if (keysym == XK_Escape && e->state == 0) {
          _active = false;

        } else if (keysym == XK_Tab
            && (e->state == XCB_MOD_MASK_4
              || e->state == (XCB_MOD_MASK_4 | XCB_MOD_MASK_SHIFT))) {
          if (_active) {
            _current_client->update_preview(false);
            move_client(e->state == XCB_MOD_MASK_4);
            _current_client->update_preview(true);
          } else {
            _active = true;
            _c.grab_keyboard();
            _active_window = _c.net_active_window();
            _x_clients.update();

            _layout->arrange(_c.current_screen(), _x_clients);

            _current_client =
              std::find(_x_clients.begin(), _x_clients.end(), _active_window);
            move_client(e->state == XCB_MOD_MASK_4);

            for (auto & client : _x_clients) {
              client.show_preview(client == _current_client->window());
            }
          }
        }

      } else if (XCB_KEY_RELEASE == (ge->response_type & ~0x80)) {
        xcb_key_release_event_t * e = (xcb_key_release_event_t *)ge;
        xcb_keysym_t keysym = _c.keycode_to_keysym(e->detail);
        if (keysym == XK_Super_L) {
          for (auto & client : _x_clients) {
            client.hide_preview();
          }
          _c.request_change_active_window(_current_client->window());
          _active = false;
          _c.ungrab_keyboard();
        }
      }
    }

  private:
    bool _active = false;
    xcb_window_t _active_window;

    const x_connection & _c;
    const layout_t * _layout;
    x_client_container & _x_clients;
    x_client_container::iterator _current_client;

    void move_client(bool next)
    {
      if (next) {
        if (++_current_client == _x_clients.end()) {
          _current_client = _x_clients.begin();
        }
      } else {
        if (_current_client == _x_clients.begin()) {
          _current_client = _x_clients.end();
        }
        --_current_client;
      }
    }
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

  if (window_attributes_reply) {
    xcb_render_pictformat_t format =
      render_find_visual_format(c, window_attributes_reply->visual);

    delete window_attributes_reply;

    uint32_t mask = XCB_RENDER_CP_REPEAT | XCB_RENDER_CP_SUBWINDOW_MODE;
    uint32_t list[] = { XCB_RENDER_REPEAT_NONE,
                        XCB_SUBWINDOW_MODE_INCLUDE_INFERIORS };

    xcb_render_picture_t picture = xcb_generate_id(c());
    xcb_render_create_picture(c(), picture, window, format, mask, list);

    return picture;
  }

  return XCB_NONE;
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

  x_event_source es(c);
  x_client_container cc(c, es);

  grid_t grid;
  x_clients_preview cp(c, &grid, cc);

  es.register_handler(&c);
  es.register_handler(&cp);

  es.run_event_loop();

  return 0;
}
