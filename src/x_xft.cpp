#ifndef X_XFT_CPP
#define X_XFT_CPP

#include "x_xft.hpp"

using namespace x;
namespace t = x::type;

xft::xft(Display * dpy, const Drawable & drawable)
  : m_dpy(dpy), m_drawable(drawable)
{
  unsigned int depth;
  {
    Window root;
    int x, y;
    unsigned int w, h, bw;
    XGetGeometry(m_dpy, drawable, &root, &x, &y, &w, &h, &bw, &depth);
  }

  XMatchVisualInfo(m_dpy, DefaultScreen(dpy), depth, TrueColor, &m_visual_info);

  m_colormap = XCreateColormap(
      m_dpy, m_drawable, m_visual_info.visual, AllocNone);

  m_xftdraw = XftDrawCreate(
      m_dpy, m_drawable, m_visual_info.visual, m_colormap);
}

xft::xft(Display * dpy, const Pixmap & pixmap, int depth)
  : m_dpy(dpy), m_drawable(pixmap)
{
  XMatchVisualInfo(m_dpy, DefaultScreen(dpy), depth, TrueColor, &m_visual_info);

  m_colormap = XCreateColormap(
      m_dpy, DefaultRootWindow(m_dpy), m_visual_info.visual, AllocNone);

  m_xftdraw = XftDrawCreateAlpha(m_dpy, m_drawable, depth);
}

xft::~xft(void)
{
  XftDrawDestroy(m_xftdraw);

  for (auto & item : m_colors) {
    XftColorFree(m_dpy, m_visual_info.visual, m_colormap, &item.second);
  }

  for (auto & item : m_fonts) {
    XftFontClose(m_dpy, item.second);
  }
  XFreeColormap(m_dpy, m_colormap);
}

xft &
xft::draw_string_utf8(const std::string & text,
                      const unsigned int & x, const unsigned int & y,
                      const x::type::fontname & fontname,
                      const x::type::colorname & colorname,
                      double alpha)
{
  XftColor color = *(*this)[colorname];

  color.color.red   *= alpha;
  color.color.green *= alpha;
  color.color.blue  *= alpha;
  color.color.alpha *= alpha;

  XftDrawStringUtf8(m_xftdraw, &color, (*this)[fontname], x, y,
                    (FcChar8 *)text.c_str(), text.length());
  return *this;
}

XGlyphInfo
xft::text_extents_utf8(const t::fontname & fontname, const std::string & text)
{
  XGlyphInfo extents;
  XftTextExtentsUtf8(m_dpy, (*this)[fontname],
                     (FcChar8 *)text.c_str(), text.length(), &extents);
  return extents;
}

XftColor * const
xft::operator[](const x::type::colorname & colorname)
{
  try {
    return &m_colors.at(*colorname);
  } catch (...) {
    XftColorAllocName(m_dpy, m_visual_info.visual, m_colormap,
                      colorname->c_str(), &m_colors[*colorname]);
    return &m_colors[*colorname];
  }
}

XftFont * const
xft::operator[](const t::fontname & fontname)
{
  try {
    return m_fonts.at(*fontname);
  } catch (...) {
    m_fonts[*fontname] =
      XftFontOpenName(m_dpy, DefaultScreen(m_dpy), fontname->c_str());
    return m_fonts[*fontname];
  }
}

#endif // X_XFT_CPP
