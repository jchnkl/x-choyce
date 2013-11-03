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

  update(rect);

  _c.register_handler(_c.damage_event_id(), this);
  _c.register_handler(XCB_CONFIGURE_NOTIFY, this);

  _damage = xcb_generate_id(_c());

  _thumbnail_window = xcb_generate_id(_c());
  xcb_colormap_t colormap = xcb_generate_id(_c());

  unsigned int depth = 32;
  xcb_visualtype_t * const vt = _c.find_visual(depth);

  xcb_create_colormap(_c(), XCB_COLORMAP_ALLOC_NONE, colormap,
                      _c.root_window(), vt->visual_id);

  uint32_t valuemask = XCB_CW_BACK_PIXEL
                     | XCB_CW_BORDER_PIXEL
                     | XCB_CW_OVERRIDE_REDIRECT
                     | XCB_CW_COLORMAP
                     ;

  uint32_t valuelist[] = { 0, 0, 1, colormap };

  xcb_create_window(_c(), depth, _thumbnail_window,
                    _c.root_window(), 0, 0,
                    _x_client->rect().width(), _x_client->rect().height(),
                    0, XCB_WINDOW_CLASS_INPUT_OUTPUT, vt->visual_id,
                    valuemask, valuelist);

  xcb_free_colormap(_c(), colormap);

  configure_gl();
  init_gl_shader();

  update();
}

x_client_thumbnail::~x_client_thumbnail(void)
{
  _c.deregister_handler(_c.damage_event_id(), this);
  _c.deregister_handler(XCB_CONFIGURE_NOTIFY, this);
  release_gl();
  xcb_destroy_window(_c(), _thumbnail_window);
}

void
x_client_thumbnail::show(void)
{
  if (_visible) return;

  _visible = true;
  xcb_damage_create(_c(), _damage, _x_client->window(),
                    XCB_DAMAGE_REPORT_LEVEL_NON_EMPTY);

  configure_thumbnail_window();

  if (_purge) {
    purge();
    _purge = false;
  } else {
    update();
  }
}

void
x_client_thumbnail::hide(void)
{
  _visible = false;
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
x_client_thumbnail::update(const rectangle & r)
{
  _scale = std::min((double)r.width() / _x_client->rect().width(),
                    (double)r.height() / _x_client->rect().height());
  _scale = std::min(1.0, _scale);

  _rectangle.width() = _x_client->rect().width() * _scale;
  _rectangle.height() = _x_client->rect().height() * _scale;

  _rectangle.x() = r.x() + (r.width() - _rectangle.width()) / 2;
  _rectangle.y() = r.y() + (r.height() - _rectangle.height()) / 2;

  if (_visible) {
    configure_thumbnail_window();
    update();
  }
}

void
x_client_thumbnail::purge(void)
{
  release_gl();
  configure_gl();
  init_gl_shader();
  update();
}

void
x_client_thumbnail::update(int x, int y, unsigned int width, unsigned int height)
{
  glXMakeCurrent(_c.dpy(), _thumbnail_window, _gl_ctx);
  glEnable(GL_SCISSOR_TEST);
  glScissor(x, y, width, height);

  glViewport(_border_width, _border_width,
             _rectangle.width() - 2 * _border_width,
             _rectangle.height() - 2 * _border_width);

  auto * bc = _highlight ? &_focused_border_color : &_unfocused_border_color;

  glClearColor(std::get<0>(*bc), std::get<1>(*bc),
               std::get<2>(*bc), std::get<3>(*bc));

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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

  glDisable(GL_SCISSOR_TEST);
  glXMakeCurrent(_c.dpy(), XCB_NONE, NULL);
}

const xcb_window_t &
x_client_thumbnail::id(void)
{
  return _thumbnail_window;
}

const xcb_window_t &
x_client_thumbnail::window(void)
{
  return _x_client->window();
}

void
x_client_thumbnail::highlight(bool want_highlight)
{
  _highlight = want_highlight;
  glXMakeCurrent(_c.dpy(), _thumbnail_window, _gl_ctx);
  _c.glXBindTexImageEXT(_c.dpy(), _thumbnail_gl_pixmap, GLX_FRONT_EXT, NULL);

  if (want_highlight) {
    _c.glUseProgram(0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    _c.glGenerateMipmapEXT(GL_TEXTURE_2D);

  } else {
    _c.glUseProgram(_programs["grayscale_shader"]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }
  glXMakeCurrent(_c.dpy(), XCB_NONE, NULL);

  update();
}

bool
x_client_thumbnail::handle(xcb_generic_event_t * ge)
{
  bool result = false;
  if (_c.damage_event_id() == (ge->response_type & ~0x80)) {
    xcb_damage_notify_event_t * e = (xcb_damage_notify_event_t *)ge;
    xcb_damage_subtract(_c(), e->damage, XCB_NONE, XCB_NONE);

    if (_highlight) {
      glXMakeCurrent(_c.dpy(), _thumbnail_window, _gl_ctx);

      _c.glXBindTexImageEXT(_c.dpy(), _thumbnail_gl_pixmap, GLX_FRONT_EXT, NULL);
      _c.glGenerateMipmapEXT(GL_TEXTURE_2D);

      glXMakeCurrent(_c.dpy(), XCB_NONE, NULL);
    }

    update(e->area.x * _scale, e->area.y * _scale,
           e->area.width * _scale, e->area.height * _scale);
    result = true;

  } else if (XCB_CONFIGURE_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_configure_notify_event_t * e = (xcb_configure_notify_event_t *)ge;
    if (e->window == _c.root_window() || e->window == _x_client->window()) {
      if (_visible) {
        purge();
      } else {
        _purge = true;
      }
    }

    result = true;
  }

  return result;
}

void
x_client_thumbnail::configure_thumbnail_window(void)
{
  xcb_xfixes_region_t region = xcb_generate_id(_c());
  xcb_xfixes_create_region_from_window(_c(), region, _x_client->window(),
                                       XCB_SHAPE_SK_BOUNDING);

  xcb_xfixes_fetch_region_cookie_t fetch_region_cookie =
    xcb_xfixes_fetch_region(_c(), region);
  xcb_xfixes_fetch_region_reply_t * fetch_region_reply =
    xcb_xfixes_fetch_region_reply(_c(), fetch_region_cookie, NULL);

  int nrects = xcb_xfixes_fetch_region_rectangles_length(fetch_region_reply);
  xcb_rectangle_t * rects =
    xcb_xfixes_fetch_region_rectangles(fetch_region_reply);

  delete fetch_region_reply;
  xcb_xfixes_destroy_region(_c(), region);

  double ar = (double)_x_client->rect().width() / _x_client->rect().height();
  unsigned int width_cutoff = std::ceil(10.0 * ar * _scale) ;
  unsigned int height_cutoff = std::ceil(10.0 * (1.0/ar) * _scale);
  for (int i = 0; i < nrects; ++i) {
    rects[i].x *= _scale;
    rects[i].y *= _scale;
    if (rects[i].width > width_cutoff) {
      rects[i].width = std::round(_scale * (double)rects[i].width);
    }
    if (rects[i].height > height_cutoff) {
      rects[i].height = std::round(_scale * (double)rects[i].height);
    }
  }

  region = xcb_generate_id(_c());
  xcb_xfixes_create_region(_c(), region, nrects, rects);

  xcb_xfixes_set_window_shape_region(_c(), _thumbnail_window,
                                     XCB_SHAPE_SK_BOUNDING, 0, 0, region);
  xcb_xfixes_destroy_region(_c(), region);

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
  while (_x_client->name_window_pixmap() == XCB_NONE) {
    _x_client->update_name_window_pixmap();
  }

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

  auto create_ctx = [this, &vi]()
  {
    _gl_ctx = glXCreateContext(_c.dpy(), vi, NULL, GL_TRUE);
  };

  if (vi == NULL) {
    GLint gl_vi_attr[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    vi = glXChooseVisual(_c.dpy(), DefaultScreen(_c.dpy()), gl_vi_attr);
    create_ctx();
    delete vi;
  } else {
    create_ctx();
  }

  glXMakeCurrent(_c.dpy(), _thumbnail_window, _gl_ctx);

  int config = 0;
  GLXFBConfig * _gl_configs =
    glXChooseFBConfig(_c.dpy(), 0, pixmap_config, &config);
  _thumbnail_gl_pixmap =
      glXCreatePixmap(_c.dpy(), _gl_configs[0],
                      _x_client->name_window_pixmap(), pixmap_attr);
  delete _gl_configs;

  GLuint _thumbnail_gl_texture_id;
  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &_thumbnail_gl_texture_id);
  glBindTexture(GL_TEXTURE_2D, _thumbnail_gl_texture_id);
  _c.glXBindTexImageEXT(_c.dpy(), _thumbnail_gl_pixmap, GLX_FRONT_EXT, NULL);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glXMakeCurrent(_c.dpy(), XCB_NONE, NULL);
}

void
x_client_thumbnail::init_gl_shader(void)
{
  std::string shader_name = "grayscale_shader";
  std::ifstream file("./grayscale.frag");
  std::string shader_source_str((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
  file.close();

  const GLchar * shader_source[]  = { shader_source_str.c_str() };
  const GLint shader_source_len[] = { (GLint)(shader_source_str.length()) };

  GLsizei log_length = 0, max_len = 1024;
  GLchar log_buffer[max_len];

  glXMakeCurrent(_c.dpy(), _thumbnail_window, _gl_ctx);

  GLuint shader = _c.glCreateShader(GL_FRAGMENT_SHADER);
  _c.glShaderSource(shader, 1, shader_source, shader_source_len);
  _c.glCompileShader(shader);

  _c.glGetShaderInfoLog(shader, max_len, &log_length, log_buffer);
  if (log_length > 0) {
    std::cerr << "Shader compilation failed:" << std::endl
              << log_buffer << std::endl;
  }

  _programs[shader_name] = _c.glCreateProgram();
  _c.glAttachShader(_programs[shader_name], shader);
  _c.glLinkProgram(_programs[shader_name]);
  _c.glUseProgram(_programs[shader_name]);

  _c.glGetProgramInfoLog(_programs[shader_name],
                         max_len, &log_length, log_buffer);
  if (log_length > 0) {
    std::cerr << "Program creation failed:" << std::endl
              << log_buffer << std::endl;
  }

  glXMakeCurrent(_c.dpy(), XCB_NONE, NULL);
}

void
x_client_thumbnail::release_gl(void)
{
  glXMakeCurrent(_c.dpy(), _thumbnail_window, _gl_ctx);
  _c.glXReleaseTexImageEXT(_c.dpy(), _thumbnail_gl_pixmap, GLX_FRONT_EXT);
  glXDestroyContext(_c.dpy(), _gl_ctx);
  glXMakeCurrent(_c.dpy(), None, NULL);
}

bool operator==(const x_client_thumbnail & thumbnail, const xcb_window_t & window)
{
  return *(thumbnail._x_client) == window;
}

bool operator==(const xcb_window_t & window, const x_client_thumbnail & thumbnail)
{
  return thumbnail == window;
}

thumbnail_t::ptr
x_client_thumbnail::factory::
make(x_connection & c, const xcb_window_t & w, const rectangle & r) const
{
  return thumbnail_t::ptr(new x_client_thumbnail(c, r, w));
}
