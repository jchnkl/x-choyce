#ifndef _X_CLIENT_HPP
#define _X_CLIENT_HPP

#include <list>
#include <xcb/xcb.h>
#include <xcb/damage.h>

#include "data_types.hpp"
#include "x_event_handler_t.hpp"
#include "x_connection.hpp"

class x_client : public x_event_handler_t {
  public:
    friend std::ostream & operator<<(std::ostream &, const x_client &);
    friend bool operator==(const x_client &, const xcb_window_t &);
    friend bool operator==(const xcb_window_t &, const x_client &);

    x_client(x_connection & c, const xcb_window_t & window);
    ~x_client(void);

    bool operator==(const x_client & other)
    {
      return _window == other._window;
    }

          rectangle &  rect(void);
    const rectangle &  rect(void) const;
          xcb_window_t & window(void);
    const xcb_window_t & window(void) const;
          xcb_window_t & parent(void);
    const xcb_window_t & parent(void) const;
          xcb_pixmap_t & name_window_pixmap(void);
    const xcb_pixmap_t & name_window_pixmap(void) const;
          unsigned int   net_wm_desktop(void) const;
    const xcb_pixmap_t & icon_pixmap(void) const;
    const std::pair<unsigned int, unsigned int> & icon_geometry(void) const;

    const std::string & net_wm_name(void) const;
    const std::string & wm_name(void) const;
    const std::string & wm_class_name(void) const;
    const std::string & wm_instance_name(void) const;

    bool handle(xcb_generic_event_t * ge);
    void update_geometry(void);

    void update_name_window_pixmap(void);

  private:
    x_connection & _c;

    rectangle _rectangle;
    unsigned int _net_wm_desktop;
    xcb_window_t _window;
    xcb_window_t _parent;
    xcb_pixmap_t _net_wm_icon_pixmap = XCB_NONE;
    xcb_pixmap_t _wm_hints_icon_pixmap = XCB_NONE;
    xcb_pixmap_t _name_window_pixmap = XCB_NONE;

    std::string _net_wm_name;
    std::string _wm_name;
    std::string _class_name;
    std::string _instance_name;

    xcb_atom_t a_wm_name = _c.intern_atom("WM_NAME");
    xcb_atom_t a_wm_class = _c.intern_atom("WM_CLASS");
    xcb_atom_t a_wm_hints = _c.intern_atom("WM_HINTS");
    xcb_atom_t a_net_wm_icon = _c.intern_atom("_NET_WM_ICON");
    xcb_atom_t a_net_wm_name = _c.intern_atom("_NET_WM_NAME");
    xcb_atom_t a_net_wm_desktop = _c.intern_atom("_NET_CURRENT_DESKTOP");

    std::pair<unsigned int, unsigned int> _icon_geometry;

    void update_net_wm_icon(void);
    void update_net_wm_name(void);
    void update_wm_name(void);
    void update_wm_class(void);
    void update_wm_hints_icon(void);
    void update_net_wm_desktop(void);
    void update_parent_window(void);
    void alpha_transform(uint8_t * data, unsigned int w, unsigned int h);
};


std::list<x_client>
make_x_clients(x_connection & c, const std::vector<xcb_window_t> & windows);

bool
operator==(const x_client &, const xcb_window_t &);

bool
operator==(const xcb_window_t &, const x_client &);

#endif
