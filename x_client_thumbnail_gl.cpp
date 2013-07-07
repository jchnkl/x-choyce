#include "x_client_thumbnail_gl.hpp"

#include <xcb/composite.h>
#include <xcb/xcb_renderutil.h>

x_client_thumbnail::x_client_thumbnail(x_connection & c,
                                       const rectangle & rect,
                                       const xcb_window_t & window,
                                       std::shared_ptr<x_client> xclient)
      : _c(c)
{
  if (window == XCB_NONE && xclient == NULL) {
    throw std::invalid_argument(
        "x_client_thumbnail requires either window or xclient parameter");
  } else if (xclient == NULL) {
    _x_client = x_client_ptr(new x_client(_c, window));
  } else {
    _x_client = xclient;
  }

  update_rectangle(rect);

  _c.register_handler(_c.damage_event_id(), this);

  _parent_pixmap = xcb_generate_id(_c());
  xcb_composite_name_window_pixmap(_c(), _x_client->window(), _parent_pixmap);

  uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT;
  uint32_t values[] = { 0, true };

  _thumbnail_window = xcb_generate_id(_c());
  xcb_create_window(_c(), XCB_COPY_FROM_PARENT, _thumbnail_window,
                    _c.default_screen()->root,
                    0, 0, 1, 1, 0,
                    // _position.x, _position.y, _size.width, _size.height, 0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    _c.default_screen()->root_visual, mask, values);

  _damage = xcb_generate_id(_c());

  _parent_pixmap = xcb_generate_id(_c());
}

x_client_thumbnail::~x_client_thumbnail(void)
{
  _c.deregister_handler(_c.damage_event_id(), this);
  xcb_destroy_window(_c(), _thumbnail_window);
}

void
x_client_thumbnail::show(void)
{
  xcb_damage_create(_c(), _damage, _x_client->window(),
                    XCB_DAMAGE_REPORT_LEVEL_NON_EMPTY);
  configure_thumbnail_window();
  configure_gl();
  update();
}

void
x_client_thumbnail::hide(void)
{
  xcb_damage_destroy(_c(), _damage);
  xcb_unmap_window(_c(), _thumbnail_window);
}

void
x_client_thumbnail::select(void)
{
  _c.request_change_current_desktop(_x_client->net_wm_desktop());
  _c.request_change_active_window(_x_client->window());
}

void
x_client_thumbnail::update(void)
{
  update(0, 0, _rectangle.width(), _rectangle.height());
}

void
x_client_thumbnail::update(int x, int y, unsigned int width, unsigned int height)
{
  glXMakeCurrent(_c.dpy(), _thumbnail_window, _gl_ctx);

  glViewport(0, 0, _rectangle.width(), _rectangle.height());
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0); glVertex3f(-1.0,  1.0, 0.0);
  glTexCoord2f(1.0, 0.0); glVertex3f( 1.0,  1.0, 0.0);
  glTexCoord2f(1.0, 1.0); glVertex3f( 1.0, -1.0, 0.0);
  glTexCoord2f(0.0, 1.0); glVertex3f(-1.0, -1.0, 0.0);
  glEnd();

  glXSwapBuffers(_c.dpy(), _thumbnail_window);
}

void
x_client_thumbnail::highlight(bool want_highlight)
{
  update();
}

bool
x_client_thumbnail::handle(xcb_generic_event_t * ge)
{
  if (_c.damage_event_id() == (ge->response_type & ~0x80)) {
    xcb_damage_notify_event_t * e = (xcb_damage_notify_event_t *)ge;
    xcb_damage_subtract(_c(), e->damage, XCB_NONE, XCB_NONE);
    update(e->area.x * _scale, e->area.y * _scale,
           e->area.width * _scale, e->area.height * _scale);
    return true;
  }

  return false;
}

void
x_client_thumbnail::update_rectangle(const rectangle & rect)
{
  _scale = std::min((double)rect.width() / _x_client->rect().width(),
                    (double)rect.height() / _x_client->rect().height());

  _rectangle.x() = rect.x();
  _rectangle.y() = rect.y();
  _rectangle.width() = _x_client->rect().width() * _scale;
  _rectangle.height() = _x_client->rect().height() * _scale;
}

void
x_client_thumbnail::configure_thumbnail_window(void)
{
  uint32_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
                | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT
                | XCB_CONFIG_WINDOW_STACK_MODE;

  uint32_t values[] = { (uint32_t)_rectangle.x(),
                        (uint32_t)_rectangle.y(),
                        (uint32_t)_rectangle.width(),
                        (uint32_t)_rectangle.height(),
                        XCB_STACK_MODE_ABOVE };

  xcb_configure_window(_c(), _thumbnail_window, mask, values);
  xcb_map_window(_c(), _thumbnail_window);
}

void
x_client_thumbnail::configure_gl(XVisualInfo * vi)
{
  const int pixmap_config[] = {
    GLX_BIND_TO_TEXTURE_RGBA_EXT, True,
    GLX_DRAWABLE_TYPE, GLX_PIXMAP_BIT,
    GLX_BIND_TO_TEXTURE_TARGETS_EXT, GLX_TEXTURE_2D_BIT_EXT,
    GLX_DOUBLEBUFFER, False,
    GLX_Y_INVERTED_EXT,
    None
  };

  const int pixmap_attr[] = {
    GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
    GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGB_EXT,
    None
  };

  if (vi == NULL) {
    GLint gl_vi_attr[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    vi = glXChooseVisual(_c.dpy(), DefaultScreen(_c.dpy()), gl_vi_attr);
    delete vi;
  }
  _gl_ctx = glXCreateContext(_c.dpy(), vi, NULL, GL_TRUE);
  glXMakeCurrent(_c.dpy(), _thumbnail_window, _gl_ctx);

  int config = 0;
  _gl_configs = glXChooseFBConfig(_c.dpy(), 0, pixmap_config, &config);

  _thumbnail_gl_pixmap =
      glXCreatePixmap(_c.dpy(), _gl_configs[0], _parent_pixmap, pixmap_attr);

  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &_thumbnail_gl_texture_id);
  glBindTexture(GL_TEXTURE_2D, _thumbnail_gl_texture_id);
  _c.glXBindTexImageEXT(_c.dpy(), _thumbnail_gl_pixmap, GLX_FRONT_EXT, NULL);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

bool operator==(const x_client_thumbnail & thumbnail, const xcb_window_t & window)
{
  return *(thumbnail._x_client) == window;
}

bool operator==(const xcb_window_t & window, const x_client_thumbnail & thumbnail)
{
  return thumbnail == window;
}
