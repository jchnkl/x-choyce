#include "x_client_thumbnail_gl.hpp"

#include <cmath>
#include <xcb/composite.h>

#include "x_connection.hpp"
#include "shader/shader.hpp"

x_client_thumbnail::x_client_thumbnail(x_connection & c,
                                       x::xrm & xrm,
                                       const gl::config & config,
                                       const rectangle & rect,
                                       const xcb_window_t & window)
  : m_c(c), m_xrm(xrm), m_gl_api(config.api())
  , m_gl_ctx(config)
  , m_x_client(m_c, window)
  , m_x_client_icon(m_c, m_x_client)
  , m_x_client_name(m_c, m_xrm, m_x_client, config.visual_info(), config.colormap())
{
  load_config();
  update(rect);

  m_c.attach(0, m_c.damage_event_id(), this);

  m_xrm.attach(this);
  m_x_client.attach(this);
  m_x_client_name.attach(this);

  m_thumbnail_window = xcb_generate_id(m_c());

  uint32_t valuemask = XCB_CW_BACK_PIXEL
                     | XCB_CW_BORDER_PIXEL
                     | XCB_CW_OVERRIDE_REDIRECT
                     | XCB_CW_COLORMAP
                     ;

  uint32_t valuelist[] = { 0, 0, 1, (uint32_t)config.colormap() };

  xcb_create_window(m_c(),
                    config.visual_info()->depth,
                    m_thumbnail_window,
                    m_c.root_window(), 0, 0,
                    m_x_client.rect().width(), m_x_client.rect().height(),
                    0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    config.visual_info()->visualid,
                    valuemask, valuelist);

  // m_x_client_name.make_title();

  m_gl_ctx.drawable(m_thumbnail_window);

  m_gl_ctx.run([&](gl::context & ctx)
  {
    ctx.load<GL_VERTEX_SHADER>("position", position_src);
    ctx.load<GL_FRAGMENT_SHADER>("focused", focused_src);
    ctx.load<GL_FRAGMENT_SHADER>("unfocused", unfocused_src);

    ctx.compile("focused", "position", "focused");
    ctx.compile("unfocused", "position", "unfocused");

    glEnable(GL_SCISSOR_TEST);
  });
}

x_client_thumbnail::~x_client_thumbnail(void)
{
  m_c.detach(m_c.damage_event_id(), this);
  m_xrm.detach(this);
  m_x_client.detach(this);
  m_x_client_name.detach(this);
  xcb_damage_destroy(m_c(), m_damage);
  xcb_destroy_window(m_c(), m_thumbnail_window);
}

thumbnail_t &
x_client_thumbnail::show(void)
{
  if (m_visible) return *this;
  m_visible = true;

  m_damage = xcb_generate_id(m_c());
  xcb_damage_create(m_c(), m_damage, m_x_client.window(),
                    XCB_DAMAGE_REPORT_LEVEL_NON_EMPTY);

  xcb_map_window(m_c(), m_thumbnail_window);

  return *this;
}

thumbnail_t &
x_client_thumbnail::hide(void)
{
  m_visible = false;
  xcb_damage_destroy(m_c(), m_damage);
  xcb_unmap_window(m_c(), m_thumbnail_window);
  return *this;
}

thumbnail_t &
x_client_thumbnail::select(void)
{
  m_c.request_change_current_desktop(m_x_client.net_wm_desktop());
  m_c.request_change_active_window(m_x_client.window());
  return *this;
}

thumbnail_t &
x_client_thumbnail::update(void)
{
  if (m_rectangle_update) configure_thumbnail();

  m_gl_ctx.run([&](gl::context & ctx)
  {
    if (m_rectangle_update) {
      update_uniforms(ctx.program(m_highlight ? "focused" : "unfocused"));
    }

    if (m_icon_update) {
      update_icon_pixmap();
    }

    if (m_title_update || m_rectangle_update) {
      update_title_pixmap();
    }

    if (m_name_window_update) {
      update_name_window_pixmap();
    }

    if (m_highlight_update) {
      update_highlight();
    }

    update(0, 0, m_rectangle.width(), m_rectangle.height());
  });

  m_icon_update = false;
  m_title_update = false;
  m_rectangle_update = false;
  m_highlight_update = false;
  m_name_window_update = false;

  return *this;
}

thumbnail_t &
x_client_thumbnail::update(const rectangle & r)
{
  m_rectangle_update = true;

  m_scale = std::min((double)r.width() / m_x_client.rect().width(),
                    (double)r.height() / m_x_client.rect().height());
  m_scale = std::min(1.0, m_scale);

  m_rectangle.width() = m_x_client.rect().width() * m_scale;
  m_rectangle.height() = m_x_client.rect().height() * m_scale;

  m_rectangle.x() = r.x() + (r.width() - m_rectangle.width()) / 2;
  m_rectangle.y() = r.y() + (r.height() - m_rectangle.height()) / 2;

  m_x_client_name.title_width(m_rectangle.width() - 2 * m_border_width);
  m_x_client_name.title_height(m_icon_size + m_border_width);

  m_icon_scale.first  = m_icon_size / (double)m_rectangle.width();
  m_icon_scale.second = m_icon_size / (double)m_rectangle.height();

  m_title_scale.first  = 1.0;
  m_title_scale.second = (double)(m_icon_size + m_border_width)
                       / (double)m_rectangle.height();

  double icon_offset   = 0.5 * (double)m_border_width / (double)m_icon_size;
  m_icon_offset.first  = icon_offset;
  m_icon_offset.second = ((double)m_rectangle.height() / (double)m_icon_size)
                       - icon_offset - 1.0;

  m_title_offset.first  = 0.0;
  m_title_offset.second = (double)m_rectangle.height()
                        / m_x_client_name.title_height() - 1.0;

  return *this;
}

const rectangle &
x_client_thumbnail::rect(void)
{
  return m_rectangle;
}

void
x_client_thumbnail::update(int x, int y, unsigned int width, unsigned int height)
{
  glScissor(x, y, width, height);

  glViewport(m_border_width, m_border_width,
             m_rectangle.width() - 2 * m_border_width,
             m_rectangle.height() - 2 * m_border_width);

  auto * bc = m_highlight ? &m_focused_border_color : &m_unfocused_border_color;

  // r, g, b, a
  glClearColor(std::get<0>(*bc), std::get<1>(*bc),
               std::get<2>(*bc), std::get<3>(*bc));

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glLoadIdentity();

  glPushMatrix();
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0); glVertex3f(-1.0,  1.0, 0.0);
  glTexCoord2f(1.0, 0.0); glVertex3f( 1.0,  1.0, 0.0);
  glTexCoord2f(1.0, 1.0); glVertex3f( 1.0, -1.0, 0.0);
  glTexCoord2f(0.0, 1.0); glVertex3f(-1.0, -1.0, 0.0);
  glEnd();
  glPopMatrix();

  glXSwapBuffers(m_c.dpy(), m_thumbnail_window);
}

void
x_client_thumbnail::update_uniforms(const GLuint & program)
{
  GLint l;

  for (auto tid : { 0, 1, 2 }) {
    l = m_gl_api.glGetUniformLocation(program, ("texture_" + std::to_string(tid)).c_str());
    m_gl_api.glUniform1i(l, tid);
  }

  l = m_gl_api.glGetUniformLocation(program, "t1_scale");
  m_gl_api.glUniform2f(l, m_title_scale.first, m_title_scale.second);

  l = m_gl_api.glGetUniformLocation(program, "t2_scale");
  m_gl_api.glUniform2f(l, m_icon_scale.first, m_icon_scale.second);

  l = m_gl_api.glGetUniformLocation(program, "t1_offset");
  m_gl_api.glUniform2f(l, m_title_offset.first, m_title_offset.second);

  l = m_gl_api.glGetUniformLocation(program, "t2_offset");
  m_gl_api.glUniform2f(l, m_icon_offset.first, m_icon_offset.second);
}

void
x_client_thumbnail::update_icon_pixmap(void)
{
  // m_gl_ctx.load("icon", m_x_client_icon.icon(), GLX_TEXTURE_FORMAT_RGBA_EXT);
  // m_gl_ctx.bind(2, "icon");
  // m_gl_ctx.active_texture(2);
  // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
}

void
x_client_thumbnail::update_title_pixmap(void)
{
  // m_x_client_name.make_title();
  // m_gl_ctx.load("title", m_x_client_name.title(), GLX_TEXTURE_FORMAT_RGBA_EXT);
  // m_gl_ctx.bind(1, "title");
  // m_gl_ctx.active_texture(1);
  // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
}

void
x_client_thumbnail::update_name_window_pixmap(void)
{
  m_x_client.update_parent_window();
  m_x_client.update_name_window_pixmap();
  m_gl_ctx.load("window", m_x_client.name_window_pixmap(), GLX_TEXTURE_FORMAT_RGB_EXT);
  m_gl_ctx.bind(0, "window");
  m_gl_ctx.active_texture(0);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
}

const xcb_window_t &
x_client_thumbnail::id(void)
{
  return m_thumbnail_window;
}

const xcb_window_t &
x_client_thumbnail::window(void)
{
  return m_x_client.window();
}

thumbnail_t &
x_client_thumbnail::highlight(bool want_highlight)
{
  m_highlight_update = true;
  m_highlight = want_highlight;
  return *this;
}

void
x_client_thumbnail::update_highlight(void)
{
  auto use_program = [this](const std::string & name)
  {
    auto & program = m_gl_ctx.program(name);
    m_gl_api.glUseProgram(program);
    update_uniforms(program);
  };

  if (m_highlight) {
    use_program("focused");
    m_gl_ctx.active_texture(0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    m_gl_api.glGenerateMipmap(GL_TEXTURE_2D);

  } else {
    use_program("unfocused");
    m_gl_ctx.active_texture(0);
    m_gl_ctx.pixmap("window", [this](const GLXPixmap & p)
    {
      m_gl_api.glXReleaseTexImageEXT(m_c.dpy(), p, GLX_FRONT_EXT);
      m_gl_api.glXBindTexImageEXT(m_c.dpy(), p, GLX_FRONT_EXT, NULL);
    });
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }
}

bool
x_client_thumbnail::handle(xcb_generic_event_t * ge)
{
  bool result = false;
  if (m_c.damage_event_id() == (ge->response_type & ~0x80)) {
    xcb_damage_notify_event_t * e = (xcb_damage_notify_event_t *)ge;

    if (e->drawable == m_x_client.window()) {
      xcb_damage_subtract(m_c(), e->damage, XCB_NONE, XCB_NONE);

      m_gl_ctx.run([&](gl::context &)
      {
        if (m_highlight) {
          m_gl_ctx.active_texture(0);
          m_gl_ctx.pixmap("window", [&](const GLXPixmap & p)
          {
            m_gl_api.glXReleaseTexImageEXT(m_c.dpy(), p, GLX_FRONT_EXT);
            m_gl_api.glXBindTexImageEXT(m_c.dpy(), p, GLX_FRONT_EXT, NULL);
          });
          m_gl_api.glGenerateMipmap(GL_TEXTURE_2D);
        }

        update(e->area.x * m_scale, e->area.y * m_scale,
               e->area.width * m_scale, e->area.height * m_scale);
      });
    }

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
  m_name_window_update = true;
  if (m_visible) update();
}

void
x_client_thumbnail::notify(x_client_name * c)
{
  m_title_update = true;
  if (m_visible) update();
}

void
x_client_thumbnail::configure_thumbnail(void)
{
  xcb_xfixes_region_t region = xcb_generate_id(m_c());
  xcb_xfixes_create_region_from_window(m_c(), region, m_x_client.window(),
                                       XCB_SHAPE_SK_BOUNDING);

  xcb_xfixes_fetch_region_cookie_t fetch_region_cookie =
    xcb_xfixes_fetch_region(m_c(), region);
  xcb_xfixes_fetch_region_reply_t * fetch_region_reply =
    xcb_xfixes_fetch_region_reply(m_c(), fetch_region_cookie, NULL);

  int nrects = xcb_xfixes_fetch_region_rectangles_length(fetch_region_reply);
  xcb_rectangle_t * rects =
    xcb_xfixes_fetch_region_rectangles(fetch_region_reply);

  delete fetch_region_reply;
  xcb_xfixes_destroy_region(m_c(), region);

  double ar = (double)m_rectangle.width() / m_rectangle.height();
  unsigned int width_cutoff = std::ceil(10.0 * ar * m_scale) ;
  unsigned int height_cutoff = std::ceil(10.0 * (1.0/ar) * m_scale);
  for (int i = 0; i < nrects; ++i) {
    rects[i].x *= m_scale;
    rects[i].y *= m_scale;
    if (rects[i].width > width_cutoff) {
      rects[i].width = std::round(m_scale * (double)rects[i].width);
    }
    if (rects[i].height > height_cutoff) {
      rects[i].height = std::round(m_scale * (double)rects[i].height);
    }
  }

  region = xcb_generate_id(m_c());
  xcb_xfixes_create_region(m_c(), region, nrects, rects);

  xcb_xfixes_set_window_shape_region(m_c(), m_thumbnail_window,
                                     XCB_SHAPE_SK_BOUNDING, 0, 0, region);
  xcb_xfixes_destroy_region(m_c(), region);

  uint32_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
                | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT
                | XCB_CONFIG_WINDOW_STACK_MODE;

  uint32_t values[] = { (uint32_t)m_rectangle.x(),
                        (uint32_t)m_rectangle.y(),
                        (uint32_t)m_rectangle.width(),
                        (uint32_t)m_rectangle.height(),
                        XCB_STACK_MODE_ABOVE };

  xcb_configure_window(m_c(), m_thumbnail_window, mask, values);
}

void
x_client_thumbnail::load_config(void)
{
  m_icon_size    = m_xrm["iconsize"].v.num;
  m_border_width = m_xrm["borderwidth"].v.num;

  // #xxxxxx: r: [1,2]; g: [3,4], b: [5,6]
  // 0123456
  auto fa = m_xrm["focusedalpha"].v.dbl;
  auto fc = m_xrm["focusedcolor"].v.str;
  auto fr = std::strtol(fc->substr(1,2).c_str(), NULL, 16) / (double)0xff;
  auto fg = std::strtol(fc->substr(3,2).c_str(), NULL, 16) / (double)0xff;
  auto fb = std::strtol(fc->substr(5,2).c_str(), NULL, 16) / (double)0xff;

  auto ua = m_xrm["unfocusedalpha"].v.dbl;
  auto uc = m_xrm["unfocusedcolor"].v.str;
  auto ur = std::strtol(uc->substr(1,2).c_str(), NULL, 16) / (double)0xff;
  auto ug = std::strtol(uc->substr(3,2).c_str(), NULL, 16) / (double)0xff;
  auto ub = std::strtol(uc->substr(5,2).c_str(), NULL, 16) / (double)0xff;

  m_focused_border_color   = std::make_tuple(fr, fg, fb, fa);
  m_unfocused_border_color = std::make_tuple(ur, ug, ub, ua);
}

bool operator==(const x_client_thumbnail & thumbnail, const xcb_window_t & window)
{
  return thumbnail.m_x_client == window;
}

bool operator==(const xcb_window_t & window, const x_client_thumbnail & thumbnail)
{
  return thumbnail == window;
}

x_client_thumbnail::factory::factory(x_connection & c, x::xrm & xrm)
  : m_c(c), m_xrm(xrm), m_gl_config(c.dpy())
{}

thumbnail_t::ptr
x_client_thumbnail::factory::
make(const xcb_window_t & w, const rectangle & r) const
{
  return thumbnail_t::ptr(
      new x_client_thumbnail(m_c, m_xrm, m_gl_config, r, w));
}
