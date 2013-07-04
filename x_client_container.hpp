#ifndef _X_CLIENT_CONTAINER
#define _X_CLIENT_CONTAINER

#include <list>
#include <vector>

#include "x_client.hpp"
#include "x_event_source.hpp"
#include "x_event_handler_t.hpp"
#include "cyclic_iterator.hpp"

class x_client_container : public x_event_handler_t {
  public:
    typedef x_client_t value_type;
    typedef value_type * pointer;
    typedef value_type & reference;
    typedef std::list<value_type> container_t;
    typedef container_t::iterator iterator;
    typedef container_t::const_iterator const_iterator;
    typedef container_t::reverse_iterator reverse_iterator;
    typedef container_t::const_reverse_iterator const_reverse_iterator;
    typedef cyclic_iterator<container_t> cyclic_x_client_iterator;
    typedef const_cyclic_iterator<container_t> const_cyclic_x_client_iterator;

    iterator begin(void)                      { return _container.begin(); }
    iterator end(void)                        { return _container.end(); }
    const_iterator begin(void) const          { return _container.cbegin(); }
    const_iterator end(void) const            { return _container.cend(); }
    reverse_iterator rbegin(void)             { return _container.rbegin(); }
    reverse_iterator rend(void)               { return _container.rend(); }
    const_reverse_iterator rbegin(void) const { return _container.crbegin(); }
    const_reverse_iterator rend(void) const   { return _container.crend(); }
    const size_t size(void) const             { return _container.size(); }

    cyclic_iterator<container_t> cbegin(void)
    {
      return cyclic_iterator<container_t>(&_container);
    }
    cyclic_iterator<container_t> cend(void)
    {
      return cyclic_iterator<container_t>(&_container);
    }

    const_cyclic_iterator<container_t> ccbegin(void)
    {
      return const_cyclic_iterator<container_t>(&_container);
    }
    const_cyclic_iterator<container_t> ccend(void)
    {
      return const_cyclic_iterator<container_t>(&_container);
    }

    x_client_container(const x_connection & c, x_event_source & es)
      : _c(c), _x_event_source(es)
    {
      update();
      _current_client = _container.begin();
      _c.select_input(_c.root_window(), XCB_EVENT_MASK_PROPERTY_CHANGE);
    }

    void handle(xcb_generic_event_t * ge)
    {
      if (XCB_PROPERTY_NOTIFY == (ge->response_type & ~0x80)) {
        xcb_property_notify_event_t * e = (xcb_property_notify_event_t *)ge;
        if (e->window == _c.root_window()
            && e->atom == _c.intern_atom("_NET_CLIENT_LIST_STACKING")) {
          auto cw = _current_client->window();
          update();
          auto result = std::find(_container.begin(), _container.end(), cw);
          if (result == _container.end()) {
            _current_client = _container.begin();
          } else {
            _current_client = result;
          }
        }
      }
    }

    void update(void)
    {
      for (auto & xc : _container) { _x_event_source.unregister_handler(&xc); }
      _container.clear();
      _windows.clear();
      _windows = _c.net_client_list_stacking();
      _container = make_x_clients(_c, _windows);
      for (auto & xc : _container) { _x_event_source.register_handler(&xc); }
    }

  private:
    container_t _container;
    iterator _current_client;
    std::vector<xcb_window_t> _windows;
    const x_connection & _c;
    x_event_source & _x_event_source;
};

#endif
