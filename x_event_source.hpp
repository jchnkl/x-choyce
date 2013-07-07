#ifndef _X_EVENT_SOURCE
#define _X_EVENT_SOURCE

#include <algorithm>
#include <list>
#include <unordered_map>

#include "x_connection.hpp"
#include "x_event_source_t.hpp"
#include "x_event_handler_t.hpp"

class x_event_source : public x_event_source_t {
  public:
    x_event_source(x_connection & c) : _c(c) {}

    void register_handler(unsigned int event_id, x_event_handler_t * eh)
    {
      _handler[event_id].push_back(eh);
    }

    void deregister_handler(unsigned int event_id, x_event_handler_t * eh)
    {
      try {
        std::remove(_handler.at(event_id).begin(), _handler.at(event_id).end(), eh);
      } catch (...) {}
    }

    void run_event_loop(void)
    {
      xcb_generic_event_t * ge = NULL;
      while (true) {
        _c.flush();
        ge = xcb_wait_for_event(_c());

        if (! ge) {
          continue;

        } else {
          bool taken = false;
          unsigned int response_type = ge->response_type & ~0x80;

          try {
            for (auto * eh : _handler.at(response_type)) {
              eh->handle(ge);
            }
          } catch (...) {}

          delete ge;
        }
      }
    }

  private:
    typedef std::list<x_event_handler_t *> handler_list_t;

    x_connection & _c;
    std::unordered_map<unsigned int, handler_list_t> _handler;
};

#endif
