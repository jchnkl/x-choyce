#ifndef _X_CLIENT_CONTAINER
#define _X_CLIENT_CONTAINER

#include <list>
#include <vector>

#include "x_client.hpp"
#include "x_event_source.hpp"

class x_client_container {
  public:
    typedef std::list<x_client_t> container_t;
    typedef container_t::iterator iterator;
    typedef container_t::const_iterator const_iterator;
    typedef container_t::reverse_iterator reverse_iterator;
    typedef container_t::const_reverse_iterator const_reverse_iterator;

    iterator begin(void)                      { return _x_clients.begin(); }
    iterator end(void)                        { return _x_clients.end(); }
    const_iterator begin(void) const          { return _x_clients.cbegin(); }
    const_iterator end(void) const            { return _x_clients.cend(); }
    reverse_iterator rbegin(void)             { return _x_clients.rbegin(); }
    reverse_iterator rend(void)               { return _x_clients.rend(); }
    const_reverse_iterator rbegin(void) const { return _x_clients.crbegin(); }
    const_reverse_iterator rend(void) const   { return _x_clients.crend(); }
    const size_t size(void) const             { return _x_clients.size(); }

    x_client_container(const x_connection & c, x_event_source & es)
      : _c(c), _x_event_source(es) {}

    void update(void)
    {
      for (auto & xc : _x_clients) { _x_event_source.unregister_handler(&xc); }
      _x_clients.clear();
      _windows.clear();
      _windows = _c.net_client_list_stacking();
      _x_clients = make_x_clients(_c, _windows);
      for (auto & xc : _x_clients) { _x_event_source.register_handler(&xc); }
    }

  private:
    container_t _x_clients;
    std::vector<xcb_window_t> _windows;
    const x_connection & _c;
    x_event_source & _x_event_source;
};

#endif
