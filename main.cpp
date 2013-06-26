#include <climits>
#include <cstdlib>
#include <iostream>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/damage.h>
#include <xcb/composite.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <cairo/cairo.h>
#include <cairo/cairo-gl.h>
#include <cairo/cairo-xcb.h>

typedef void (*t_glx_bind)(Display *, GLXDrawable, int , const int *);
typedef void (*t_glx_release)(Display *, GLXDrawable, int);

t_glx_bind glXBindTexImageEXT = 0;
t_glx_release glXReleaseTexImageEXT = 0;

typedef struct {
  bool first;
  xcb_connection_t * c;
  xcb_visualtype_t * vt;
  XVisualInfo * vi;
  GLXContext gl_ctx;
  Display * dpy;
  int width, height;
  double scale;
  xcb_drawable_t dstwin, srcwin;
  cairo_device_t * device;
  cairo_surface_t * dst_surface, * src_surface;
} thumbnail_ctx;

xcb_screen_t *
screen_of_display(xcb_connection_t * c, int screen)
{
  xcb_screen_iterator_t iter;
  iter = xcb_setup_roots_iterator(xcb_get_setup(c));
  for (; iter.rem; --screen, xcb_screen_next(&iter))
    if (screen == 0)
      return iter.data;
  return NULL;
}

xcb_visualtype_t *
default_visual_of_screen(xcb_screen_t * screen)
{
  if (!screen) { return NULL; }

  xcb_depth_iterator_t depth_iterator =
    xcb_screen_allowed_depths_iterator(screen);

  if (depth_iterator.data) {
    for (; depth_iterator.rem; xcb_depth_next(&depth_iterator)) {
      xcb_visualtype_iterator_t visual_iterator =
        xcb_depth_visuals_iterator(depth_iterator.data);
      for (; visual_iterator.rem; xcb_visualtype_next (&visual_iterator)) {
        if (screen->root_visual == visual_iterator.data->visual_id) {
          return visual_iterator.data;
        }
      }
    }
  }

  return NULL;
}

  // xcb_connection_t * c, xcb_visualtype_t * visualtype,
  //              int width, int height, double scale,
  //              xcb_drawable_t dstwin, xcb_drawable_t srcwin)
void
draw_thumbnail(thumbnail_ctx * tn)
{
  // thumbnail.src_surface =
  //   cairo_xcb_surface_create(tn->c, tn->srcwin, tn->vt, tn->width, tn->height);
    // cairo_gl_surface_create_for_window(tn->device, tn->srcwin,
    //                                    tn->width, tn->height);

  cairo_surface_t * src_surface_scaled =
    cairo_surface_create_similar(tn->src_surface,
        CAIRO_CONTENT_COLOR_ALPHA, tn->width, tn->height);

  cairo_t * cr = cairo_create(src_surface_scaled);

  // Scale *before* setting the source surface (1)
  cairo_scale(cr, tn->scale, tn->scale);
  cairo_set_source_surface(cr, tn->src_surface, 0, 0);
  // To avoid getting the edge pixels blended with 0 alpha, which would occur
  // with the default EXTEND_NONE. Use EXTEND_PAD for 1.2 or newer (2)
  cairo_pattern_set_extend(cairo_get_source(cr), CAIRO_EXTEND_NONE);
  // Replace the destination with the source instead of overlaying
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  // Do the actual drawing
  cairo_paint(cr);
  cairo_destroy(cr);

  cr = cairo_create(src_surface_scaled);
  // cairo_set_source_rgb(cp, 0, 0, 0); // 0.5, 0.5, 0.5);
  // cairo_set_operator(cr, CAIRO_OPERATOR_HSL_SATURATION);
  cairo_set_operator(cr, CAIRO_OPERATOR_HSL_COLOR);
  cairo_paint(cr);
  cairo_destroy(cr);
  goto skip;
skip:

  // tn->dst_surface =
  //   cairo_xcb_surface_create(tn->c, tn->dstwin, tn->vt, tn->width, tn->height);
    // cairo_gl_surface_create_for_window(tn->device, tn->dstwin,
    //                                    tn->width, tn->height);
  cr = cairo_create(tn->dst_surface);
  cairo_set_source_surface(cr, src_surface_scaled, 0, 0);
  cairo_paint(cr);
  cairo_destroy(cr);

  // cairo_surface_destroy(tn->dst_surface);
  // cairo_surface_destroy(tn->src_surface);
  cairo_surface_destroy(src_surface_scaled);
}

int main(int argc, char ** argv)
{
  double scale = 0.2;
  int width = 450, height = 450;
      // old_width = 1200, old_height = 1920;
  // int screen_number;
  xcb_window_t root_window;

  Display * dpy = XOpenDisplay(NULL);
  xcb_connection_t * c = XGetXCBConnection(dpy);
  // xcb_connection_t * c = xcb_connect(NULL, &screen_number);

  // xcb_screen_t * default_screen = screen_of_display(c, screen_number);
  xcb_screen_t * default_screen = screen_of_display(c, DefaultScreen(dpy));

  if (default_screen) {
    root_window = default_screen->root;
  } else {
    exit(EXIT_FAILURE);
  }

  xcb_window_t window = xcb_generate_id(c);
  xcb_create_window(c,                             /* Connection          */
                    XCB_COPY_FROM_PARENT,          /* depth (same as root)*/
                    window,                        /* window Id           */
                    default_screen->root,          /* parent window       */
                    0, 0,                          /* x, y                */
                    width, height,                 /* width, height       */
                    1,                             /* border_width        */
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class               */
                    default_screen->root_visual,   /* visual              */
                    0, NULL );                     /* masks, not used yet */

  xcb_map_window(c, window);
  xcb_flush(c);

//   cairo_public cairo_device_t * cairo_glx_device_create (Display *dpy, GLXContext gl_ctx);
// 
//   cairo_public Display *
//     cairo_glx_device_get_display (cairo_device_t *device);
// 
//   cairo_public GLXContext
//     cairo_glx_device_get_context (cairo_device_t *device);
// 
//   cairo_public cairo_surface_t *
//     cairo_gl_surface_create_for_window (cairo_device_t *device,
//         Window win,
//         int width, int height);
// 
  xcb_visualtype_t * visualtype = default_visual_of_screen(default_screen);
  if (! visualtype) { return EXIT_FAILURE; }

  std::string atom_name = "_NET_CLIENT_LIST_STACKING";
  xcb_intern_atom_cookie_t atom_cookie =
    xcb_intern_atom(c, false, atom_name.length(), atom_name.c_str());
  xcb_intern_atom_reply_t * atom_reply =
    xcb_intern_atom_reply(c, atom_cookie, NULL);

  xcb_get_property_cookie_t property_cookie =
    xcb_get_property(c, false, root_window,
                     atom_reply->atom, XCB_ATOM_WINDOW, 0, UINT_MAX);

  xcb_get_property_reply_t * property_reply =
    xcb_get_property_reply(c, property_cookie, NULL);

  xcb_window_t * windows =
    (xcb_window_t *)xcb_get_property_value(property_reply);

  std::cerr << "windows length: " << property_reply->length << std::endl;

  for (uint i = 0; i < property_reply->value_len; ) {
    std::cerr << std::hex << windows[i];
    if (++i < property_reply->value_len) { std::cerr << ", "; }
  }
  std::cerr << std::endl;

  int wid = 6;
  std::cerr << "using window[" << wid << "]: " << windows[wid] << std::endl;
  // windows[wid] = 65011872;

  xcb_pixmap_t pixmap = xcb_generate_id(c);
  xcb_composite_name_window_pixmap(c, windows[wid], pixmap);

  xcb_get_geometry_cookie_t geometry_cookie = xcb_get_geometry(c, windows[wid]);
  xcb_get_geometry_reply_t * geometry_reply =
    xcb_get_geometry_reply(c, geometry_cookie, NULL);

  width  = geometry_reply->width  * scale;
  height = geometry_reply->height * scale;

  std::cerr << std::dec
            << "Scaling " << geometry_reply->width
            << "x"        << geometry_reply->height
            << " to "     << width
            << "x"        << height
            << std::endl;

  uint16_t value_mask = XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
  uint32_t value_list[] = { (uint32_t)width, (uint32_t)height };
  xcb_configure_window(c, window, value_mask, value_list);

  xcb_prefetch_extension_data(c, &xcb_damage_id);

  const xcb_query_extension_reply_t * extension_reply =
    xcb_get_extension_data(c, &xcb_damage_id);

  uint8_t damage_event_id = extension_reply->first_event + XCB_DAMAGE_NOTIFY;

  xcb_damage_query_version_cookie_t damage_query_version_cookie =
    xcb_damage_query_version(c, XCB_DAMAGE_MAJOR_VERSION,
                             XCB_DAMAGE_MINOR_VERSION);

  xcb_damage_query_version_reply_t * damage_query_version_reply =
    xcb_damage_query_version_reply(c, damage_query_version_cookie, NULL);

  std::cerr << "damage major: " << damage_query_version_reply->major_version
            << ", minor: "      << damage_query_version_reply->minor_version
            << std::endl;

  xcb_damage_damage_t damage = xcb_generate_id(c);
  xcb_damage_create(c, damage, windows[wid],
                    XCB_DAMAGE_REPORT_LEVEL_BOUNDING_BOX);

  thumbnail_ctx thumbnail;
  thumbnail.c = c;
  thumbnail.vt = visualtype;
  thumbnail.width = width;
  thumbnail.height = height;
  thumbnail.scale = scale;
  thumbnail.dstwin = window;
  thumbnail.srcwin = pixmap;
  thumbnail.dpy = dpy;

  GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
  thumbnail.vi = glXChooseVisual(dpy, 0, att);
  thumbnail.gl_ctx = glXCreateContext(dpy, thumbnail.vi, NULL, GL_TRUE);
  // glXMakeCurrent(dpy, win, thumbnail.gl_ctx);
  // glEnable(GL_DEPTH_TEST);
  thumbnail.device = cairo_glx_device_create(dpy, thumbnail.gl_ctx);

  GLuint texture_id;

  const int pixmap_config[] = {
    GLX_BIND_TO_TEXTURE_RGBA_EXT, True,
    GLX_DRAWABLE_TYPE, GLX_PIXMAP_BIT,
    GLX_BIND_TO_TEXTURE_TARGETS_EXT, GLX_TEXTURE_2D_BIT_EXT,
    GLX_DOUBLEBUFFER, False,
    GLX_Y_INVERTED_EXT,
    None
  };

  const int pixmap_attribs[] = {
    GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
    GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGB_EXT,
    None
  };

  int nc = 0;
  GLXFBConfig * configs = glXChooseFBConfig(dpy, 0, pixmap_config, &nc);

  glXBindTexImageEXT = (t_glx_bind) glXGetProcAddress((const GLubyte *)"glXBindTexImageEXT");
  glXReleaseTexImageEXT = (t_glx_release) glXGetProcAddress((const GLubyte *)"glXReleaseTexImageEXT");

  GLXPixmap glxpixmap =
    glXCreatePixmap(dpy, configs[0], pixmap, pixmap_attribs);
  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glXBindTexImageEXT(dpy, glxpixmap, GLX_FRONT_EXT, NULL);

  thumbnail.src_surface =
    cairo_gl_surface_create_for_texture(thumbnail.device,
        CAIRO_CONTENT_COLOR_ALPHA,
        texture_id, width, height);

    // cairo_gl_surface_create_for_window(thumbnail.device, thumbnail.srcwin,
    //                                    thumbnail.width, thumbnail.height);
    // cairo_xcb_surface_create(thumbnail.c, thumbnail.srcwin, thumbnail.vt,
    //                          thumbnail.width, thumbnail.height);

  thumbnail.dst_surface =
    cairo_gl_surface_create_for_window(thumbnail.device, thumbnail.dstwin,
                                       thumbnail.width, thumbnail.height);
    // cairo_xcb_surface_create(thumbnail.c, thumbnail.dstwin, thumbnail.vt,
    //                          thumbnail.width, thumbnail.height);

  while (true) {
    xcb_flush(c);
    xcb_generic_event_t * ge = xcb_wait_for_event(c);

    if (! ge) {
      continue;
    } else if (damage_event_id == (ge->response_type & ~0x80)) {
      xcb_damage_notify_event_t * e = (xcb_damage_notify_event_t *)ge;
      xcb_damage_subtract(c, e->damage, XCB_NONE, XCB_NONE);
      draw_thumbnail(&thumbnail);

      std::cerr << "XCB_DAMAGE_NOTIFY" << std::endl;
    } else {
      std::cerr << "EVENT: " << (ge->response_type & ~0x80) << std::endl;
    }

    delete ge;
  }

  return 0;
}
