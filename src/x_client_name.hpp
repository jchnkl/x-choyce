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
    x_client_name(x_connection & c, x::xrm & xrm, x_client & x_client);
    ~x_client_name(void);

    const std::string & net_wm_name(void) const;
    const std::string & wm_name(void) const;
    const std::string & wm_class_name(void) const;
    const std::string & wm_instance_name(void) const;

    const xcb_pixmap_t & title(void) const;

    const unsigned int & title_width(void) const { return _title_width; }
    const unsigned int & title_height(void) const { return _title_height; }
    void title_width(const unsigned int & w) { _title_width = w; }
    void title_height(const unsigned int & h) { _title_height = h; }

    void make_title(void);
    bool handle(xcb_generic_event_t *);
    void notify(x::xrm *);
    void notify(x_client *);

  private:
    x_connection & m_c;
    x::xrm & m_xrm;
    x_client & _x_client;
    std::shared_ptr<x::xft> _x_xft;

    std::string _net_wm_name;
    std::string _wm_name;
    std::string _class_name;
    std::string _instance_name;

    xcb_pixmap_t _title = XCB_NONE;

    xcb_atom_t _a_wm_name = m_c.intern_atom("WM_NAME");
    xcb_atom_t _a_wm_class = m_c.intern_atom("WM_CLASS");
    xcb_atom_t _a_net_wm_name = m_c.intern_atom("_NET_WM_NAME");

    unsigned int _title_width = 0;
    unsigned int _title_height = 0;

    // >> config options

    int _border_width;
    int _icon_size;

    // 0.375 * 0xff; 0.25 * 0xff
    uint32_t _title_bg_color;

    x::type::fontname _pnamefont;
    x::type::fontname _titlefont;
    x::type::colorname _colorname;

    // << config options

    void load_config(void);
    void update_wm_name(void);
    void update_wm_class(void);
    void update_net_wm_name(void);
};

#endif // X_CLIENT_NAME_HPP
