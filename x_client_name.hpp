#ifndef X_CLIENT_NAME_HPP
#define X_CLIENT_NAME_HPP

#include "x_connection.hpp"
#include "x_client.hpp"

class x_client_name : public x_event_handler_t {
  public:
    x_client_name(x_connection & c, x_client * const x_client);
    ~x_client_name(void);

    const std::string & net_wm_name(void) const;
    const std::string & wm_name(void) const;
    const std::string & wm_class_name(void) const;
    const std::string & wm_instance_name(void) const;

    bool handle(xcb_generic_event_t *);

  private:
    x_connection & _c;
    x_client * const _x_client;

    std::string _net_wm_name;
    std::string _wm_name;
    std::string _class_name;
    std::string _instance_name;

    xcb_atom_t _a_wm_name = _c.intern_atom("WM_NAME");
    xcb_atom_t _a_wm_class = _c.intern_atom("WM_CLASS");
    xcb_atom_t _a_net_wm_name = _c.intern_atom("_NET_WM_NAME");

    void update_wm_name(void);
    void update_wm_class(void);
    void update_net_wm_name(void);
};

#endif // X_CLIENT_NAME_HPP
