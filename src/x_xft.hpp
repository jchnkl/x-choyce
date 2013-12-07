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
        unsigned int width, unsigned int height);
    ~xft(void);

    xft(const xft &) = delete;
    xft & operator=(const xft &) = delete;

    xft &
    draw_string_utf8(const std::string & text,
                     const unsigned int & x, const unsigned int & y,
                     const x::type::fontname & fontname,
                     const x::type::colorname & colorname,
                     double alpha = 1.0);

    XGlyphInfo
    text_extents_utf8(const x::type::fontname &, const std::string & text);

    XftColor * const operator[](const x::type::colorname & colorname);

    XftFont * const operator[](const x::type::fontname & fontname);

    const Drawable & drawable(void)
    {
      return m_drawable;
    }

    xft & fontname(const x::type::fontname & font)
    {
      m_font = *(*this)[font];
      return *this;
    }

    xft & foreground(const x::type::colorname & foreground)
    {
      m_fg_color = *(*this)[foreground];
      return *this;
    }

    xft & background(const x::type::colorname & background)
    {
      m_fg_color = *(*this)[background];
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
    Drawable m_drawable;
    Colormap m_colormap;
    XVisualInfo * m_visual_info;

    unsigned int m_width = 0;
    unsigned int m_height = 0;

    XftFont m_font;
    XftColor m_fg_color;
    XftColor m_bg_color;

    std::unordered_map<std::string, XftColor> m_colors;
    std::unordered_map<std::string, XftFont *> m_fonts;
}; // class xft

}; // namespace x

#endif // X_XFT_HPP
