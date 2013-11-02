#include "x_connection.hpp"

#include <climits>
#include <cstring>
#include <xcb/damage.h>
#include <xcb/xfixes.h>
#include <xcb/xinerama.h>
#include <xcb/xcb_keysyms.h>

#include "x_ewmh.hpp"
#include "x_event_source.hpp"

x_connection::x_connection(std::shared_ptr<x_ewmh> ewmh,
                           std::shared_ptr<x_event_source> event_source)
{
  _dpy = XOpenDisplay(NULL);
  XSetEventQueueOwner(_dpy, XCBOwnsEventQueue);
  _screen_number = DefaultScreen(_dpy);
  _c = XGetXCBConnection(_dpy);
  find_default_screen();
  _root_window = _default_screen->root;

  if (event_source == NULL) {
    _event_source =
      std::shared_ptr<x_event_source_t>(new x_event_source(*this));
  } else {
    _event_source = event_source;
  }

  _event_source->register_handler(XCB_CONFIGURE_NOTIFY, this);

  if (ewmh == NULL) {
    _ewmh = std::shared_ptr<x_ewmh>(new x_ewmh(*this));
  } else {
    _ewmh = ewmh;
  }

  init_gl();
  init_composite();
  init_damage();
  init_render();
  init_xfixes();
  init_xinerama();
  update_input(_root_window, XCB_EVENT_MASK_STRUCTURE_NOTIFY);
}

x_connection::~x_connection(void)
{
  _event_source->deregister_handler(XCB_CONFIGURE_NOTIFY, this);
  xcb_disconnect(_c);
}

xcb_connection_t * const
x_connection::operator()(void) const { return _c; }

Display * const
x_connection::dpy(void) const { return _dpy; }

void
x_connection::select_input(xcb_window_t window, uint32_t event_mask) const
{
  uint32_t mask = XCB_CW_EVENT_MASK;
  uint32_t values[] = { event_mask };
  xcb_change_window_attributes(_c, window, mask, values);
}

void
x_connection::update_input(xcb_window_t window, uint32_t event_mask) const
{
  xcb_get_window_attributes_cookie_t window_attributes_cookie =
    xcb_get_window_attributes(_c, window);
  xcb_get_window_attributes_reply_t * window_attributes_reply =
    xcb_get_window_attributes_reply(_c, window_attributes_cookie, NULL);

  if (window_attributes_reply) {
    event_mask |= window_attributes_reply->your_event_mask;
  }

  delete window_attributes_reply;

  select_input(window, event_mask);
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

xcb_visualtype_t * const
x_connection::find_visual(unsigned int depth)
{
  xcb_depth_iterator_t depth_iter =
    xcb_screen_allowed_depths_iterator(default_screen());

  if (depth_iter.data) {
    for (; depth_iter.rem; xcb_depth_next (&depth_iter)) {
      if (depth_iter.data->depth == depth) {
        xcb_visualtype_iterator_t visual_iter =
          xcb_depth_visuals_iterator(depth_iter.data);
        for (; visual_iter.rem; xcb_visualtype_next(&visual_iter)) {
          return visual_iter.data;
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

x_connection::modifier_map
x_connection::modifier_mapping(void) const
{
  modifier_map result;

  xcb_get_modifier_mapping_reply_t * modifier_mapping_reply =
    xcb_get_modifier_mapping_reply(_c, xcb_get_modifier_mapping(_c), NULL);

  if (modifier_mapping_reply == NULL) { return result; }

  xcb_keycode_t * keycodes =
    xcb_get_modifier_mapping_keycodes(modifier_mapping_reply);

  int keycodes_length =
    xcb_get_modifier_mapping_keycodes_length(modifier_mapping_reply);

  uint8_t keycodes_per_modifier = modifier_mapping_reply->keycodes_per_modifier;

  xcb_mod_mask_t modifiers[] = { XCB_MOD_MASK_SHIFT,
                                 XCB_MOD_MASK_LOCK,
                                 XCB_MOD_MASK_CONTROL,
                                 XCB_MOD_MASK_1, XCB_MOD_MASK_2,
                                 XCB_MOD_MASK_3, XCB_MOD_MASK_4, XCB_MOD_MASK_5 };

  for (int ks = 0, m = 0; ks < keycodes_length; ks += keycodes_per_modifier) {
    for (int k = 0; k < keycodes_per_modifier; ++k) {
      xcb_keycode_t keycode = keycodes[m * keycodes_per_modifier + k];
      if (keycode != 0) { result[modifiers[m]].push_back(keycode); }
    }
    ++m;
  }

  delete modifier_mapping_reply;
  return result;
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

std::string
x_connection::keysym_to_string(xcb_keysym_t keysym) const
{
  char * key_name = XKeysymToString(keysym);
  if (key_name == NULL) {
    return std::string("(null)");
  } else {
    return std::string(key_name);
  }
}

std::tuple<xcb_window_t, std::vector<xcb_window_t>>
x_connection::query_tree(xcb_window_t parent)
{
  xcb_generic_error_t * error;
  xcb_query_tree_cookie_t cookie = xcb_query_tree(_c, parent);
  xcb_query_tree_reply_t * reply = xcb_query_tree_reply(_c, cookie, &error);

  if (error) {
    delete error;
    return std::make_tuple(XCB_NONE, std::vector<xcb_window_t>());
  }

  std::tuple<xcb_window_t, std::vector<xcb_window_t>> result;

  if (reply) {
    int length = xcb_query_tree_children_length(reply);
    xcb_window_t * windows = xcb_query_tree_children(reply);
    result = std::make_tuple(
        reply->parent, std::vector<xcb_window_t>(windows, windows + length));
    delete reply;
  }

  return result;
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
  return _ewmh->net_active_window();
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

// root, window
std::pair<position, position>
x_connection::query_pointer(const xcb_window_t & window) const
{
  xcb_generic_error_t * error;
  xcb_query_pointer_cookie_t c =
    xcb_query_pointer(_c, window == XCB_NONE ? _root_window : window);
  xcb_query_pointer_reply_t * r = xcb_query_pointer_reply(_c, c, &error);

  if (error) {
    delete error;
    throw "query_pointer failed";
  } else {
    position root = { r->root_x, r->root_y };
    position win  = { r->win_x,  r->win_y };
    delete r;
    return { root, win };
  }
}

rectangle
x_connection::get_geometry(const xcb_window_t & window) const
{
  xcb_generic_error_t * error;
  xcb_get_geometry_cookie_t c =
    xcb_get_geometry(_c, window == XCB_NONE ? _root_window : window);
  xcb_get_geometry_reply_t * r = xcb_get_geometry_reply(_c, c, &error);

  if (error) {
    delete error;
    throw "get_geometry failed";
  } else {
    rectangle rect;
    rect._position = { r->x, r->y };
    rect._dimension = { r->width, r->height };
    delete r;
    return rect;
  }
}

rectangle
x_connection::current_screen(const position & p) const
{
  if (! _screens.empty()) {
    for (auto & screen : _screens) {
      if (p.x >= screen.x() && p.x <= screen.x() + (int)screen.width()
          && p.y >= screen.y() && p.y <= screen.y() + (int)screen.height()) {
        return screen;
      }
    }
  }

  throw "no screens available";
}

bool
x_connection::handle(xcb_generic_event_t * ge)
{
  if (XCB_CONFIGURE_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_configure_request_event_t * e = (xcb_configure_request_event_t *)ge;
    if (e->window == _root_window) {
      update_xinerama();
    }
  }

  return true;
}

void
x_connection::register_handler(unsigned int event_id, x_event_handler_t * eh)
{
  _event_source->register_handler(event_id, eh);
}

void
x_connection::deregister_handler(unsigned int event_id, x_event_handler_t * eh)
{
  _event_source->deregister_handler(event_id, eh);
}

void
x_connection::run_event_loop(void)
{
  _event_source->run_event_loop();
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
x_connection::init_gl(void)
{
  glXBindTexImageEXT = (PFNGLXBINDTEXIMAGEEXTPROC)
    glXGetProcAddress((const GLubyte *)"glXBindTexImageEXT");
  glXReleaseTexImageEXT = (PFNGLXRELEASETEXIMAGEEXTPROC)
    glXGetProcAddress((const GLubyte *)"glXReleaseTexImageEXT");
  glXGetFBConfigs = (PFNGLXGETFBCONFIGSPROC)
    glXGetProcAddress((const GLubyte *)"glXGetFBConfigs");
  glCreateShader = (PFNGLCREATESHADERPROC)
    glXGetProcAddress((const GLubyte *)"glCreateShader");
  glCreateShader = (PFNGLCREATESHADERPROC)
    glXGetProcAddress((const GLubyte *)"glCreateShader");
  glShaderSource = (PFNGLSHADERSOURCEPROC)
    glXGetProcAddress((const GLubyte *)"glShaderSource");
  glCompileShader = (PFNGLCOMPILESHADERPROC)
    glXGetProcAddress((const GLubyte *)"glCompileShader");
  glCreateProgram = (PFNGLCREATEPROGRAMPROC)
    glXGetProcAddress((const GLubyte *)"glCreateProgram");
  glAttachShader = (PFNGLATTACHSHADERPROC)
    glXGetProcAddress((const GLubyte *)"glAttachShader");
  glDetachShader = (PFNGLDETACHSHADERPROC)
    glXGetProcAddress((const GLubyte *)"glDetachShader");
  glLinkProgram = (PFNGLLINKPROGRAMPROC)
    glXGetProcAddress((const GLubyte *)"glLinkProgram");
  glUseProgram = (PFNGLUSEPROGRAMPROC)
    glXGetProcAddress((const GLubyte *)"glUseProgram");
  glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)
    glXGetProcAddress((const GLubyte *)"glGetProgramInfoLog");
  glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)
    glXGetProcAddress((const GLubyte *)"glGetShaderInfoLog");
  glDeleteShader = (PFNGLDELETESHADERPROC)
    glXGetProcAddress((const GLubyte *)"glDeleteShader");
  glDeleteProgram = (PFNGLDELETEPROGRAMPROC)
    glXGetProcAddress((const GLubyte *)"glDeleteProgram");
  glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC)
    glXGetProcAddress((const GLubyte *)"glGenerateMipmapEXT");
}

void
x_connection::init_composite(void)
{
  xcb_prefetch_extension_data(_c, &xcb_composite_id);

  xcb_composite_query_version(_c, XCB_COMPOSITE_MAJOR_VERSION,
                                  XCB_COMPOSITE_MINOR_VERSION);

  xcb_composite_redirect_subwindows(_c, _root_window,
                                    XCB_COMPOSITE_REDIRECT_AUTOMATIC);
}

void
x_connection::init_damage(void)
{
  xcb_prefetch_extension_data(_c, &xcb_damage_id);

  const xcb_query_extension_reply_t * extension_reply =
    xcb_get_extension_data(_c, &xcb_damage_id);

  _damage_event_id = extension_reply->first_event + XCB_DAMAGE_NOTIFY;

  // necessary to get xdamage of the ground
  xcb_damage_query_version(_c, XCB_DAMAGE_MAJOR_VERSION,
                                XCB_DAMAGE_MINOR_VERSION);
}

void
x_connection::init_render(void) { xcb_prefetch_extension_data(_c, &xcb_render_id); }

void
x_connection::init_xfixes(void)
{
  xcb_prefetch_extension_data(_c, &xcb_xfixes_id);
  xcb_xfixes_query_version(_c, XCB_XFIXES_MAJOR_VERSION,
                               XCB_XFIXES_MINOR_VERSION);
}

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

  if (query_screens_reply
      && 0 < xcb_xinerama_query_screens_screen_info_length(query_screens_reply))
  {
    _screens.clear();
    xcb_xinerama_screen_info_t * screen_info =
      xcb_xinerama_query_screens_screen_info(query_screens_reply);
    int length =
      xcb_xinerama_query_screens_screen_info_length(query_screens_reply);
    for (int i = 0; i < length; ++i) {
      _screens.push_back(
          rectangle(screen_info[i].x_org, screen_info[i].y_org,
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
