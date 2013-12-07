#ifndef X_CLIENT_NAME_HPP
#define X_CLIENT_NAME_HPP

#include "observer.hpp"
#include "x_xrm.hpp"
#include "x_xft.hpp"
#include "x_client.hpp"
#include "x_connection.hpp"

class x_client_name : public x_event_handler_t
                    , public observer<x::xrm>
                    , public observer<x_client>
                    , public observable<x_client_name>
{
  public:
    x_client_name(x_connection & c, x::xrm & xrm, x_client & x_client,
                  XVisualInfo * const visual_info, const Colormap & colormap);
    ~x_client_name(void);

    const std::string & net_wm_name(void) const;
    const std::string & wm_name(void) const;
    const std::string & wm_class_name(void) const;
    const std::string & wm_instance_name(void) const;

    xcb_pixmap_t title(void) const;

    const unsigned int & title_width(void) const { return m_title_width; }
    const unsigned int & title_height(void) const { return m_title_height; }

    x_client_name & title_width(const unsigned int & w)
    {
      m_reset_xft = true;
      m_title_width = w;
      return *this;
    }

    x_client_name & title_height(const unsigned int & h)
    {
      m_reset_xft = true;
      m_title_height = h;
      return *this;
    }

    x_client_name & make_title(void);
    bool handle(xcb_generic_event_t *);
    void notify(x::xrm *);
    void notify(x_client *);

  private:
    x_connection & m_c;
    x::xrm & m_xrm;
    x_client & m_x_client;
    XVisualInfo * m_visual_info;
    Colormap m_colormap;

    std::shared_ptr<x::xft> m_xft;

    std::string m_net_wm_name;
    std::string m_wm_name;
    std::string m_class_name;
    std::string m_instance_name;

    xcb_atom_t m_a_wm_name = m_c.intern_atom("WM_NAME");
    xcb_atom_t m_a_wm_class = m_c.intern_atom("WM_CLASS");
    xcb_atom_t m_a_net_wm_name = m_c.intern_atom("_NET_WM_NAME");

    bool m_reset_xft = true;
    unsigned int m_title_width = 0;
    unsigned int m_title_height = 0;

    // >> config options

    int m_border_width;
    int m_icon_size;

    x::type::fontname m_pnamefont;
    x::type::fontname m_titlefont;

    double m_bg_alpha;
    x::type::colorname m_fg_color;
    x::type::colorname m_bg_color;

    // << config options

    void load_config(void);
    void update_wm_name(void);
    void update_wm_class(void);
    void update_net_wm_name(void);
};

#endif // X_CLIENT_NAME_HPP
