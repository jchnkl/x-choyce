#ifndef X_XFT_HPP
#define X_XFT_HPP

#include <iostream>
#include <unordered_map>

#include <X11/Xft/Xft.h>

#include "data_types.hpp"

namespace x {

class xft {
  public:
    xft(Display * dpy, const Drawable & drawable);
    xft(Display * dpy, const Pixmap & pixmap, int depth);
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

  private:
    xft(const xft & other);
    xft & operator=(const xft & other);

    XftDraw * m_xftdraw;
    Display * m_dpy;
    Drawable m_drawable;
    Colormap m_colormap;
    XVisualInfo m_visual_info;

    std::unordered_map<std::string, XftColor> m_colors;
    std::unordered_map<std::string, XftFont *> m_fonts;
}; // class xft

}; // namespace x

#endif // X_XFT_HPP
