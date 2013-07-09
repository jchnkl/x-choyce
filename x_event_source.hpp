#ifndef _X_EVENT_SOURCE
#define _X_EVENT_SOURCE

#include <algorithm>
#include <list>
#include <unordered_map>

#include <X11/Xlibint.h>

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
        auto & list = _handler.at(event_id);
        auto newend = std::remove(list.begin(), list.end(), eh);
        list.erase(newend, list.end());
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
            taken = true;
          } catch (...) {}

          if (! taken) {
            // Check if a custom XEvent constructor was registered in xlib for this
            // event type, and call it discarding the constructed XEvent if any.
            // XESetWireToEvent might be used by libraries to intercept messages from
            // the X server e.g. the OpenGL lib waiting for DRI2 events.

            Bool (*proc)(Display*, XEvent*, xEvent*) =
              XESetWireToEvent(_c.dpy(), response_type, 0);
            if (proc) {
              XESetWireToEvent(_c.dpy(), response_type, proc);
              XEvent dummy;
              ge->sequence = LastKnownRequestProcessed(_c.dpy());
              proc(_c.dpy(), &dummy, (xEvent*)ge);
            }
          }

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
