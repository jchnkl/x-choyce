#ifndef X_XFT_HPP
#define X_XFT_HPP

#include <iostream>
#include <unordered_map>

#include <X11/Xft/Xft.h>

#include "data_types.hpp"

namespace x {

class xft {
  public:
    xft(Display * dpy,
        XVisualInfo * const visual_info, const Colormap & colormap,
        unsigned int width, unsigned int height)
      : m_dpy(dpy),
        m_visual_info(visual_info), m_colormap(colormap),
        m_width(width), m_height(height)
    {
      m_drawable = XCreatePixmap(
          m_dpy, DefaultRootWindow(m_dpy), width, height, m_visual_info->depth);
      m_xftdraw = XftDrawCreate(m_dpy, m_drawable, m_visual_info->visual, m_colormap);
    }

    ~xft(void)
    {
      XftDrawDestroy(m_xftdraw);

      for (auto & item : m_colors) {
        XftColorFree(m_dpy, m_visual_info->visual, m_colormap, &item.second);
      }

      for (auto & item : m_fonts) {
        XftFontClose(m_dpy, item.second);
      }

      if (m_drawable != None) XFreePixmap(m_dpy, m_drawable);
    }

    xft(const xft &) = delete;
    xft & operator=(const xft &) = delete;

    xft &
    draw_string_utf8(const std::string & text,
                     const unsigned int & x, const unsigned int & y)
    {
      XftDrawStringUtf8(m_xftdraw, &m_fg_color, m_font, x, y,
                        (FcChar8 *)text.c_str(), text.length());
      return *this;
    }

    XGlyphInfo
    text_extents_utf8(const x::type::fontname & fontname,
                      const std::string & text)
    {
      XGlyphInfo extents;
      XftTextExtentsUtf8(m_dpy, (*this)[fontname],
                         (FcChar8 *)text.c_str(), text.length(), &extents);
      return extents;
    }

    XftFont * const
    operator[](const x::type::fontname & fontname)
    {
      try {
        return m_fonts.at(*fontname);
      } catch (...) {
        m_fonts[*fontname] =
          XftFontOpenName(m_dpy, DefaultScreen(m_dpy), fontname->c_str());
        return m_fonts[*fontname];
      }
    }

    XftColor * const
    operator[](const x::type::colorname & colorname)
    {
      try {
        return &m_colors.at(*colorname);
      } catch (...) {
        XftColorAllocName(m_dpy, m_visual_info->visual, m_colormap,
                          colorname->c_str(), &m_colors[*colorname]);
        return &m_colors[*colorname];
      }
    }

    const Drawable & drawable(void)
    {
      return m_drawable;
    }

    xft & font(const x::type::fontname & font)
    {
      m_font = (*this)[font];
      return *this;
    }

    xft & foreground(const x::type::colorname & foreground)
    {
      m_fg_color = *(*this)[foreground];
      return *this;
    }

    xft & background(const x::type::colorname & background)
    {
      m_bg_color = *(*this)[background];
      return *this;
    }

    xft & fg_alpha(const double & a)
    {
      m_fg_color.color.alpha *= a;
      return *this;
    }

    xft & bg_alpha(const double & a)
    {
      m_bg_color.color.alpha *= a;
      return *this;
    }

    xft & fill(unsigned int x = 0, unsigned int y = 0,
               unsigned int width = 0, unsigned int height = 0)
    {
      width = width == 0 ? m_width : width;
      height = height == 0 ? m_height : height;
      XftDrawRect(m_xftdraw, &m_bg_color, x, y, m_width, m_height);
      return *this;
    }

  private:
    XftDraw * m_xftdraw;
    Display * m_dpy;
    Drawable m_drawable = None;

    XVisualInfo * const m_visual_info;
    const Colormap & m_colormap;

    unsigned int m_width = 0;
    unsigned int m_height = 0;

    XftFont * m_font;
    XftColor m_fg_color;
    XftColor m_bg_color;

    std::unordered_map<std::string, XftColor> m_colors;
    std::unordered_map<std::string, XftFont *> m_fonts;
}; // class xft

}; // namespace x

#endif // X_XFT_HPP
