#include "x_connection.hpp"

#include <climits>
#include <cstring>
#include <xcb/damage.h>
#include <xcb/xinerama.h>
#include <xcb/xcb_keysyms.h>

x_connection::x_connection(void)
{
  _c = xcb_connect(NULL, &_screen_number);
  find_default_screen();
  _root_window = _default_screen->root;
  init_damage();
  init_render();
  init_xinerama();
  select_input(_root_window, XCB_EVENT_MASK_STRUCTURE_NOTIFY);
}

x_connection::~x_connection(void)
{
  xcb_disconnect(_c);
  for (auto er : _extension_reply_list) {
    delete er;
  }
}

xcb_connection_t *
x_connection::operator()(void) const { return _c; }

void
x_connection::select_input(xcb_window_t window, xcb_event_mask_t event_mask) const
{
  uint32_t mask = XCB_CW_EVENT_MASK;
  const uint32_t values[] = { event_mask };
  xcb_change_window_attributes(_c, window, mask, values);
}

xcb_visualtype_t *
x_connection::default_visual_of_screen(void)
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

void
x_connection::flush(void) const { xcb_flush(_c); }

xcb_screen_t * const
x_connection::default_screen(void) const
{
  return _default_screen;
}

xcb_window_t const &
x_connection::root_window(void) const
{
  return _root_window;
}

uint8_t
x_connection::damage_event_id(void) const { return _damage_event_id; }

void
x_connection::grab_key(uint16_t modifiers, xcb_keysym_t keysym) const
{
  xcb_keycode_t keycode = keysym_to_keycode(keysym);
  if (keycode != 0) {
    xcb_grab_key(_c, false, _root_window, modifiers, keycode,
                 XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
  }
}

void
x_connection::ungrab_key(uint16_t modifiers, xcb_keysym_t keysym) const
{
  xcb_keycode_t keycode = keysym_to_keycode(keysym);
  if (keycode != 0) {
    xcb_ungrab_key(_c, keycode, _root_window, modifiers);
  }
}

void
x_connection::grab_keyboard(void) const
{
  xcb_grab_keyboard_cookie_t grab_keyboard_cookie =
    xcb_grab_keyboard(_c, false, root_window(), XCB_TIME_CURRENT_TIME,
                      XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
  xcb_grab_keyboard_reply_t * grab_keyboard_reply =
    xcb_grab_keyboard_reply(_c, grab_keyboard_cookie, NULL);
  delete grab_keyboard_reply;
}

void
x_connection::ungrab_keyboard(void) const
{
  xcb_ungrab_keyboard(_c, XCB_TIME_CURRENT_TIME);
}

xcb_keysym_t
x_connection::keycode_to_keysym(xcb_keycode_t keycode) const
{
  xcb_key_symbols_t * keysyms;
  xcb_keysym_t keysym;

  if (!(keysyms = xcb_key_symbols_alloc(_c))) { return 0; }
  keysym = xcb_key_symbols_get_keysym(keysyms, keycode, 0);
  xcb_key_symbols_free(keysyms);

  return keysym;
}

xcb_keycode_t
x_connection::keysym_to_keycode(xcb_keysym_t keysym) const
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
x_connection::net_client_list_stacking(void) const
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
x_connection::intern_atom(const std::string & atom_name) const
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
x_connection::net_active_window(void) const
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

void
x_connection::request_change_current_desktop(unsigned int desktop_id) const
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

void
x_connection::request_change_active_window(xcb_window_t window) const
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

rectangle_t
x_connection::current_screen(void) const
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

void
x_connection::handle(xcb_generic_event_t * ge)
{
  if (XCB_CONFIGURE_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_configure_request_event_t * e = (xcb_configure_request_event_t *)ge;
    if (e->window == _root_window) {
      update_xinerama();
    }
  }
}

// private

void
x_connection::find_default_screen(void)
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

void
x_connection::init_damage(void)
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

void
x_connection::init_render(void) { xcb_prefetch_extension_data(_c, &xcb_render_id); }

void
x_connection::init_xinerama(void)
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

void
x_connection::update_xinerama(void)
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