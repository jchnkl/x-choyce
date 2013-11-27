#include "x_client_thumbnail_gl.hpp"

#include <cmath>
#include <xcb/composite.h>
#include "x_connection.hpp"

x_client_thumbnail::x_client_thumbnail(x_connection & c,
                                       x::xrm & xrm,
                                       const x::gl::api & api,
                                       const rectangle & rect,
                                       const xcb_window_t & window)
  : _c(c), _xrm(xrm), _gl_api(api)
  , _gl_ctx(api, _c.dpy(), _c.screen_number())
  , _x_client(_c, window)
  , _x_client_icon(_c, _x_client)
  , _x_client_name(_c, _xrm, _x_client)
{
  load_config();
  update(rect);

  _x_client_name.make_title();

  _c.attach(0, _c.damage_event_id(), this);

  _xrm.attach(this);
  _x_client.attach(this);
  _x_client_name.attach(this);

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
                    _x_client.rect().width(), _x_client.rect().height(),
                    0, XCB_WINDOW_CLASS_INPUT_OUTPUT, vt->visual_id,
                    valuemask, valuelist);

  xcb_free_colormap(_c(), colormap);

  _gl_ctx.drawable(_thumbnail_window);

  _gl_ctx.run([this](x::gl::context &)
  {
    _gl_ctx.load("./normal.frag", "normal_shader");
    _gl_ctx.load("./grayscale.frag", "grayscale_shader");

    _gl_ctx.load(0, _x_client.name_window_pixmap(), 24);
    _gl_ctx.load(1, _x_client_name.title(), 32);
    _gl_ctx.load(2, _x_client_icon.icon(), 32);

    for (auto & t : { 0, 1, 2 }) {
      _gl_ctx.texture(t, [](const GLuint &)
      {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      });
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    configure_highlight(true);
  });
}

x_client_thumbnail::~x_client_thumbnail(void)
{
  _c.detach(_c.damage_event_id(), this);
  _xrm.detach(this);
  _x_client.detach(this);
  _x_client_name.detach(this);
  xcb_damage_destroy(_c(), _damage);
  xcb_destroy_window(_c(), _thumbnail_window);
}

thumbnail_t &
x_client_thumbnail::show(void)
{
  if (_visible) return *this;

  _visible = true;
  xcb_damage_create(_c(), _damage, _x_client.window(),
                    XCB_DAMAGE_REPORT_LEVEL_NON_EMPTY);

  configure_thumbnail_window(true);

  if (_update_name_window_pixmap) {
    update_name_window_pixmap();
    _update_name_window_pixmap = false;
  }

  if (_update_title_pixmap) {
    update_title_pixmap();
    _update_title_pixmap = false;
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
  _c.request_change_current_desktop(_x_client.net_wm_desktop());
  _c.request_change_active_window(_x_client.window());
  return *this;
}

thumbnail_t &
x_client_thumbnail::update(void)
{
  configure_thumbnail_window();

  _gl_ctx.run([this](x::gl::context &)
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

  _scale = std::min((double)r.width() / _x_client.rect().width(),
                    (double)r.height() / _x_client.rect().height());
  _scale = std::min(1.0, _scale);

  _rectangle.width() = _x_client.rect().width() * _scale;
  _rectangle.height() = _x_client.rect().height() * _scale;

  _rectangle.x() = r.x() + (r.width() - _rectangle.width()) / 2;
  _rectangle.y() = r.y() + (r.height() - _rectangle.height()) / 2;

  _icon_scale_x = _icon_size / (double)_rectangle.width();
  _icon_scale_y = _icon_size / (double)_rectangle.height();

  _x_client_name.title_width(_rectangle.width());
  _x_client_name.title_height(_icon_size + _border_width);

  _title_scale_x = (double)_rectangle.width() / (double)_rectangle.width();
  _title_scale_y = (_icon_size + _border_width) / (double)_rectangle.height();

  if (_visible) {
    update_title_pixmap();
    configure_thumbnail_window(true);
  } else {
    _update_title_pixmap = true;
    _configure_thumbnail = true;
  }

  return *this;
}

const rectangle &
x_client_thumbnail::rect(void)
{
  return _rectangle;
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

  _gl_ctx.texture(0, [](const GLuint &)
  {
    glPushMatrix();
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex3f(-1.0,  1.0, 0.0);
    glTexCoord2f(1.0, 0.0); glVertex3f( 1.0,  1.0, 0.0);
    glTexCoord2f(1.0, 1.0); glVertex3f( 1.0, -1.0, 0.0);
    glTexCoord2f(0.0, 1.0); glVertex3f(-1.0, -1.0, 0.0);
    glEnd();
    glPopMatrix();
  });

  _gl_ctx.texture(1, [&](const GLuint &)
  {
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

  _gl_ctx.texture(2, [&](const GLuint &)
  {
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

  glXSwapBuffers(_c.dpy(), _thumbnail_window);
  glDisable(GL_SCISSOR_TEST);
}

void
x_client_thumbnail::update_title_pixmap(void)
{
  _x_client_name.make_title();
  _gl_ctx.run([this](x::gl::context & ctx)
  {
    ctx.load(1, _x_client_name.title(), 32);
    ctx.texture(1, [](const GLuint &)
    {
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    });
  });
}

void
x_client_thumbnail::update_name_window_pixmap(void)
{
  _x_client.update_parent_window();
  _x_client.update_name_window_pixmap();
  _gl_ctx.run([this](x::gl::context & ctx)
  {
    ctx.load(0, _x_client.name_window_pixmap(), 24);
    ctx.texture(0, [](const GLuint &)
    {
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    });
  });
}

const xcb_window_t &
x_client_thumbnail::id(void)
{
  return _thumbnail_window;
}

const xcb_window_t &
x_client_thumbnail::window(void)
{
  return _x_client.window();
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

    _gl_ctx.run([&](x::gl::context &)
    {
      if (_highlight) {
        _gl_ctx.texture(0, [&](const GLuint &)
        {
          _gl_ctx.pixmap(0, [&](const GLXPixmap & p)
          {
            _gl_api.glXReleaseTexImageEXT(_c.dpy(), p, GLX_FRONT_EXT);
            _gl_api.glXBindTexImageEXT(_c.dpy(), p, GLX_FRONT_EXT, NULL);
          });

          _gl_api.glGenerateMipmapEXT(GL_TEXTURE_2D);
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
  if (_visible) {
    update_name_window_pixmap();
    update();
  } else {
    _update_name_window_pixmap = true;
  }
}

void
x_client_thumbnail::notify(x_client_name * c)
{
  if (_visible) {
    update_title_pixmap();
    update();
  } else {
    _update_title_pixmap = true;
  }
}

void
x_client_thumbnail::configure_highlight(bool now)
{
  if (now || _configure_highlight) {
    _configure_highlight = false;
  } else {
    return;
  }

  auto use_program = [this](const std::string & name)
  {
    auto & program = _gl_ctx.program(name);
    _gl_api.glUseProgram(program);
    for (auto tid : { 0, 1, 2 }) {
      GLint location = _gl_api.glGetUniformLocationEXT(
          program, ("texture_" + std::to_string(tid)).c_str());
      _gl_api.glUniform1iEXT(location, tid);
    }
  };

  if (_highlight) {
    use_program("normal_shader");
    _gl_ctx.texture(0, [this](const GLuint &)
    {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
      _gl_api.glGenerateMipmapEXT(GL_TEXTURE_2D);
    });

  } else {
    use_program("grayscale_shader");
    _gl_ctx.texture(0, [this](const GLuint &)
    {
      _gl_ctx.pixmap(0, [this](const GLXPixmap & p)
      {
        _gl_api.glXReleaseTexImageEXT(_c.dpy(), p, GLX_FRONT_EXT);
        _gl_api.glXBindTexImageEXT(_c.dpy(), p, GLX_FRONT_EXT, NULL);
      });

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
  xcb_xfixes_create_region_from_window(_c(), region, _x_client.window(),
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
  return thumbnail._x_client == window;
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
  return thumbnail_t::ptr(new x_client_thumbnail(_c, _xrm, _gl_api, r, w));
}
