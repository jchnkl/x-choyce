#ifndef _X_EVENT_SOURCE
#define _X_EVENT_SOURCE

#include <list>

#include "x_connection.hpp"
#include "x_event_source_t.hpp"
#include "x_event_handler_t.hpp"

class x_event_source : public x_event_source_t {
  public:
    x_event_source(const x_connection & c) : _c(c) {}

    void register_handler(x_event_handler_t * eh)
    {
      _handler_list.push_back(eh);
    }

    void unregister_handler(x_event_handler_t * eh)
    {
      _handler_list.remove(eh);
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
          for (auto eh : _handler_list) { eh->handle(ge); }
          delete ge;
        }
      }
    }

  private:
    const x_connection & _c;
    std::list<x_event_handler_t *> _handler_list;
};

#endif
