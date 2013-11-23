#include "x_client_thumbnail_gl.hpp"

#include <xcb/composite.h>

x_client_thumbnail::x_client_thumbnail(x_connection & c,
                                       x::xrm & xrm,
                                       const rectangle & rect,
                                       const xcb_window_t & window,
                                       std::shared_ptr<x_client> xclient)
      : _c(c), _xrm(xrm)
{
  if (window == XCB_NONE && xclient == NULL) {
    throw std::invalid_argument(
        "x_client_thumbnail requires either window or xclient parameter");
  } else if (xclient == NULL) {
    _x_client = x_client_ptr(new x_client(_c, window));
  } else {
    _x_client = xclient;
  }

  _x_client_icon = std::shared_ptr<x_client_icon>(
      new x_client_icon(_c, _x_client.get()));

  _x_client_name = std::shared_ptr<x_client_name>(
      new x_client_name(_c, _xrm, _x_client.get()));

  load_config();

  update(rect);

  _x_client_name->make_title();

  _c.attach(0, _c.damage_event_id(), this);

  _xrm.attach(this);
  _x_client->attach(this);
  _x_client_name->attach(this);

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

  _gl_xfb_configs = glXChooseFBConfig(_c.dpy(), _c.screen_number(),
                                      _gl_pixmap_config, &_gl_xfb_nconfigs);

  configure_gl();
  init_gl_shader();
  with_context([this]() { configure_highlight(true); });
}

x_client_thumbnail::~x_client_thumbnail(void)
{
  _c.detach(_c.damage_event_id(), this);
  _xrm.detach(this);
  _x_client->detach(this);
  _x_client_name->detach(this);
  release_gl();
  if (_gl_xfb_configs != NULL) delete _gl_xfb_configs;
  xcb_destroy_window(_c(), _thumbnail_window);
}

thumbnail_t &
x_client_thumbnail::show(void)
{
  if (_visible) return *this;

  _visible = true;
  xcb_damage_create(_c(), _damage, _x_client->window(),
                    XCB_DAMAGE_REPORT_LEVEL_NON_EMPTY);

  configure_thumbnail_window(true);

  if (_purge) {
    _purge = false;
    purge();
  }

  return *this;
}

thumbnail_t &
x_client_thumbnail::hide(void)
{
  _visible = false;
  xcb_damage_destroy(_c(), _damage);
  xcb_unmap_window(_c(), _thumbnail_window);
  return *this;
}

thumbnail_t &
x_client_thumbnail::select(void)
{
  _c.request_change_current_desktop(_x_client->net_wm_desktop());
  _c.request_change_active_window(_x_client->window());
  return *this;
}

thumbnail_t &
x_client_thumbnail::update(void)
{
  configure_thumbnail_window();

  with_context([this]()
  {
    configure_highlight();
    update(0, 0, _rectangle.width(), _rectangle.height());
  });

  return *this;
}

thumbnail_t &
x_client_thumbnail::update(const rectangle & r)
{
  _configure_thumbnail = true;

  _scale = std::min((double)r.width() / _x_client->rect().width(),
                    (double)r.height() / _x_client->rect().height());
  _scale = std::min(1.0, _scale);

  _rectangle.width() = _x_client->rect().width() * _scale;
  _rectangle.height() = _x_client->rect().height() * _scale;

  _rectangle.x() = r.x() + (r.width() - _rectangle.width()) / 2;
  _rectangle.y() = r.y() + (r.height() - _rectangle.height()) / 2;

  _icon_scale_x = _icon_size / (double)_rectangle.width();
  _icon_scale_y = _icon_size / (double)_rectangle.height();

  _x_client_name->title_width(_rectangle.width());
  _x_client_name->title_height(_icon_size + _border_width);

  _title_scale_x = (double)_rectangle.width() / (double)_rectangle.width();
  _title_scale_y = (_icon_size + _border_width) / (double)_rectangle.height();

  return *this;
}

const rectangle &
x_client_thumbnail::rect(void)
{
  return _rectangle;
}

void
x_client_thumbnail::purge(void)
{
  _x_client_name->make_title();
  release_gl();
  configure_gl();
  init_gl_shader();
  with_context([this]() { configure_highlight(true); });
}

void
x_client_thumbnail::update(int x, int y, unsigned int width, unsigned int height)
{
  glEnable(GL_SCISSOR_TEST);
  glScissor(x, y, width, height);

  glViewport(_border_width, _border_width,
             _rectangle.width() - 2 * _border_width,
             _rectangle.height() - 2 * _border_width);

  auto * bc = _highlight ? &_focused_border_color : &_unfocused_border_color;

  // r, g, b, a
  glClearColor(std::get<0>(*bc), std::get<1>(*bc),
               std::get<2>(*bc), std::get<3>(*bc));

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  with_texture(0, [this](GLuint &) {
      glPushMatrix();
      glBegin(GL_QUADS);
      glTexCoord2f(0.0, 0.0); glVertex3f(-1.0,  1.0, 0.0);
      glTexCoord2f(1.0, 0.0); glVertex3f( 1.0,  1.0, 0.0);
      glTexCoord2f(1.0, 1.0); glVertex3f( 1.0, -1.0, 0.0);
      glTexCoord2f(0.0, 1.0); glVertex3f(-1.0, -1.0, 0.0);
      glEnd();
      glPopMatrix();
      });

  if (_gl_pixmap[1] != XCB_NONE) {
    with_texture(1, [this](GLuint &) {
        glPushMatrix();
        glTranslatef(-1.0 + _title_scale_x, -1.0 + _title_scale_y, 0.0);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex3f(-_title_scale_x,  _title_scale_y, 0.0);
        glTexCoord2f(1.0, 0.0); glVertex3f( _title_scale_x,  _title_scale_y, 0.0);
        glTexCoord2f(1.0, 1.0); glVertex3f( _title_scale_x, -_title_scale_y, 0.0);
        glTexCoord2f(0.0, 1.0); glVertex3f(-_title_scale_x, -_title_scale_y, 0.0);
        glEnd();
        glPopMatrix();
        });
  }

  if (_gl_pixmap[2] != XCB_NONE) {
    with_texture(2, [this](GLuint &) {
        glPushMatrix();
        glTranslatef(
          -1.0 + _icon_scale_x + (_border_width / (double)_rectangle.width()),
          -1.0 + _icon_scale_y + (_border_width / (double)_rectangle.height()),
          0.0);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex3f(-_icon_scale_x,  _icon_scale_y, 0.0);
        glTexCoord2f(1.0, 0.0); glVertex3f( _icon_scale_x,  _icon_scale_y, 0.0);
        glTexCoord2f(1.0, 1.0); glVertex3f( _icon_scale_x, -_icon_scale_y, 0.0);
        glTexCoord2f(0.0, 1.0); glVertex3f(-_icon_scale_x, -_icon_scale_y, 0.0);
        glEnd();
        glPopMatrix();
        });
  }

  glXSwapBuffers(_c.dpy(), _thumbnail_window);
  glDisable(GL_SCISSOR_TEST);
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

thumbnail_t &
x_client_thumbnail::highlight(bool want_highlight)
{
  _configure_highlight = _highlight != want_highlight;
  _highlight = want_highlight;
  return *this;
}

bool
x_client_thumbnail::handle(xcb_generic_event_t * ge)
{
  bool result = false;
  if (_c.damage_event_id() == (ge->response_type & ~0x80)) {
    xcb_damage_notify_event_t * e = (xcb_damage_notify_event_t *)ge;
    xcb_damage_subtract(_c(), e->damage, XCB_NONE, XCB_NONE);

    with_context([&, this]()
    {
      if (_highlight) {
        with_texture(0, [this](GLuint &)
        {
          _c.glXBindTexImageEXT(_c.dpy(), _gl_pixmap[0], GLX_FRONT_EXT, NULL);
          _c.glGenerateMipmapEXT(GL_TEXTURE_2D);
        });
      }

      update(e->area.x * _scale, e->area.y * _scale,
             e->area.width * _scale, e->area.height * _scale);
    });

    result = true;

  }

  return result;
}

void
x_client_thumbnail::notify(x::xrm *)
{
  load_config();
}

void
x_client_thumbnail::notify(x_client * c)
{
  with_context([this](){ load_texture(0, _x_client->name_window_pixmap(), true); });
  update();
}

void
x_client_thumbnail::notify(x_client_name * c)
{
  with_context([this](){ load_texture(1, _x_client_name->title(), true); });
  update();
}

void
x_client_thumbnail::configure_highlight(bool now)
{
  if (now || _configure_highlight) {
    _configure_highlight = false;
  } else {
    return;
  }

  auto use_program = [this](const std::string & program)
  {
    _c.glUseProgram(_programs[program]);
    for (auto tid : { 0, 1, 2 }) {
      GLint location = _c.glGetUniformLocationEXT(
          _programs[program], ("texture_" + std::to_string(tid)).c_str());
      _c.glUniform1iEXT(location, tid);
    }
  };

  if (_highlight) {
    use_program("normal_shader");
    with_texture(0, [this](GLuint &)
    {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
      _c.glGenerateMipmapEXT(GL_TEXTURE_2D);
    });

  } else {
    use_program("grayscale_shader");
    with_texture(0, [this](GLuint &)
    {
      _c.glXBindTexImageEXT(_c.dpy(), _gl_pixmap[0], GLX_FRONT_EXT, NULL);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    });
  }
}

void
x_client_thumbnail::configure_thumbnail_window(bool now)
{
  if (now || _configure_thumbnail) {
    _configure_thumbnail = false;
  } else {
    return;
  }

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

  double ar = (double)_rectangle.width() / _rectangle.height();
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
x_client_thumbnail::load_texture(GLuint id, const xcb_pixmap_t & p, bool rgba)
{
  if (_gl_pixmap[id] != XCB_NONE) {
    _c.glXReleaseTexImageEXT(_c.dpy(), _gl_pixmap[id], GLX_FRONT_EXT);
    glXDestroyGLXPixmap(_c.dpy(), _gl_pixmap[id]);
    glDeleteTextures(1, &_gl_texture_id[id]);
    _gl_pixmap[id] = XCB_NONE;
  }

  if (p == XCB_NONE || _gl_xfb_nconfigs == 0) {
    return;
  }

  const int pixmap_attr[] = {
    GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
    GLX_TEXTURE_FORMAT_EXT,
      rgba ? GLX_TEXTURE_FORMAT_RGBA_EXT : GLX_TEXTURE_FORMAT_RGB_EXT,
    None
  };

  glGenTextures(1, &_gl_texture_id[id]);

  _gl_pixmap[id] =
    glXCreatePixmap(_c.dpy(), _gl_xfb_configs[0], p, pixmap_attr);

  auto make_texture = [&, this](GLuint &)
  {
    _c.glXBindTexImageEXT(_c.dpy(), _gl_pixmap[id], GLX_FRONT_EXT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  };

  with_texture(id, make_texture);
}

void
x_client_thumbnail::configure_gl(XVisualInfo * vi)
{
  auto create_ctx = [this, &vi]()
  {
    _gl_ctx = glXCreateContext(_c.dpy(), vi, NULL, GL_TRUE);
  };

  if (vi == NULL) {
    GLint gl_vi_attr[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    vi = glXChooseVisual(_c.dpy(), _c.screen_number(), gl_vi_attr);
    create_ctx();
    delete vi;
  } else {
    create_ctx();
  }

  with_context([&, this]()
  {
    load_texture(0, _x_client->name_window_pixmap(), false);
    load_texture(1, _x_client_name->title(), true);
    load_texture(2, _x_client_icon->icon(), true);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  });
}

void
x_client_thumbnail::init_gl_shader(void)
{
  load_gl_shader("./normal.frag", "normal_shader");
  load_gl_shader("./grayscale.frag", "grayscale_shader");
}

void
x_client_thumbnail::load_gl_shader(const std::string & filename,
                                   const std::string & name)
{
  std::ifstream file(filename);
  std::string shader_source_str((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
  file.close();

  const GLchar * shader_source[]  = { shader_source_str.c_str() };
  const GLint shader_source_len[] = { (GLint)(shader_source_str.length()) };

  with_context([&, this]()
  {
    GLsizei log_length = 0, max_len = 1024;
    GLchar log_buffer[max_len];

    GLuint shader = _c.glCreateShader(GL_FRAGMENT_SHADER);
    _c.glShaderSource(shader, 1, shader_source, shader_source_len);
    _c.glCompileShader(shader);

    _c.glGetShaderInfoLog(shader, max_len, &log_length, log_buffer);
    if (log_length > 0) {
      std::cerr << "Shader compilation for " << name << " failed:" << std::endl
                << log_buffer << std::endl;
    }

    _programs[name] = _c.glCreateProgram();
    _c.glAttachShader(_programs[name], shader);
    _c.glLinkProgram(_programs[name]);

    _c.glGetProgramInfoLog(_programs[name],
                           max_len, &log_length, log_buffer);
    if (log_length > 0) {
      std::cerr << "Program creation for " << name << " failed:" << std::endl
                << log_buffer << std::endl;
    }
  });
}

void
x_client_thumbnail::release_gl(void)
{
  with_context([&, this]()
  {
    for (auto id : { 0, 1, 2 }) {
      if (_gl_pixmap[id] != XCB_NONE) {
        _c.glXReleaseTexImageEXT(_c.dpy(), _gl_pixmap[id], GLX_FRONT_EXT);
        glXDestroyGLXPixmap(_c.dpy(), _gl_pixmap[id]);
        _gl_pixmap[id] = XCB_NONE;
      }
    }

    glDeleteTextures(3, _gl_texture_id);
  });

  glXDestroyContext(_c.dpy(), _gl_ctx);
  _gl_ctx = XCB_NONE;
}

void
x_client_thumbnail::with_context(std::function<void(void)> f)
{
  if (_gl_ctx != XCB_NONE) {
    glXMakeCurrent(_c.dpy(), _thumbnail_window, _gl_ctx);
    f();
    glXMakeCurrent(_c.dpy(), None, NULL);
  }
}

void
x_client_thumbnail::with_texture(GLuint tid, std::function<void(GLuint &)> f)
{
  glActiveTexture(GL_TEXTURE0 + tid);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, _gl_texture_id[tid]);
  f(_gl_texture_id[tid]);
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0);
}

void
x_client_thumbnail::load_config(void)
{
  _icon_size    = _xrm["iconsize"].v.num;
  _border_width = _xrm["borderwidth"].v.num;

  // #xxxxxx: r: [1,2]; g: [3,4], b: [5,6]
  // 0123456
  auto fa = _xrm["focusedalpha"].v.dbl;
  auto fc = _xrm["focusedcolor"].v.str;
  auto fr = std::strtol(fc->substr(1,2).c_str(), NULL, 16) / (double)0xff;
  auto fg = std::strtol(fc->substr(3,2).c_str(), NULL, 16) / (double)0xff;
  auto fb = std::strtol(fc->substr(5,2).c_str(), NULL, 16) / (double)0xff;

  auto ua = _xrm["unfocusedalpha"].v.dbl;
  auto uc = _xrm["unfocusedcolor"].v.str;
  auto ur = std::strtol(uc->substr(1,2).c_str(), NULL, 16) / (double)0xff;
  auto ug = std::strtol(uc->substr(3,2).c_str(), NULL, 16) / (double)0xff;
  auto ub = std::strtol(uc->substr(5,2).c_str(), NULL, 16) / (double)0xff;

  _focused_border_color   = std::make_tuple(fr, fg, fb, fa);
  _unfocused_border_color = std::make_tuple(ur, ug, ub, ua);
}

bool operator==(const x_client_thumbnail & thumbnail, const xcb_window_t & window)
{
  return *(thumbnail._x_client) == window;
}

bool operator==(const xcb_window_t & window, const x_client_thumbnail & thumbnail)
{
  return thumbnail == window;
}

x_client_thumbnail::factory::factory(x_connection & c, x::xrm & xrm)
  : _c(c), _xrm(xrm)
{}

thumbnail_t::ptr
x_client_thumbnail::factory::
make(const xcb_window_t & w, const rectangle & r) const
{
  return thumbnail_t::ptr(new x_client_thumbnail(_c, _xrm, r, w));
}
