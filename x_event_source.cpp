#include "x_event_source.hpp"

x_event_source::x_event_source(x_connection & c) : _c(c)
{
  _c.update_input(_c.root_window(), XCB_EVENT_MASK_STRUCTURE_NOTIFY);
}

void
x_event_source::register_handler(unsigned int event_id, x_event_handler_t * eh)
{
  _handler[event_id].push_back(eh);
}

void
x_event_source::deregister_handler(unsigned int event_id, x_event_handler_t * eh)
{
  try {
    auto & list = _handler.at(event_id);
    auto newend = std::remove(list.begin(), list.end(), eh);
    list.erase(newend, list.end());
  } catch (...) {}
}

void
x_event_source::run_event_loop(void)
{
  xcb_generic_event_t * ge = NULL;
  while (true) {
    _c.flush();
    ge = xcb_wait_for_event(_c());

    if (! ge) {
      break;

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
