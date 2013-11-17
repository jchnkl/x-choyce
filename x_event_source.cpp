#include "x_event_source.hpp"

x_event_source::x_event_source(x_connection & c) : _c(c)
{
  _c.update_input(_c.root_window(), XCB_EVENT_MASK_STRUCTURE_NOTIFY);
}

void
x_event_source::attach(event_id_t i, x_event_handler_t * eh)
{
  _handler[i].push_back(eh);
}

void
x_event_source::detach(event_id_t i, x_event_handler_t * eh)
{
  try {
    auto & list = _handler.at(i);
    auto newend = std::remove(list.begin(), list.end(), eh);
    list.erase(newend, list.end());
  } catch (...) {}
}

void
x_event_source::run_event_loop(void)
{
  xcb_generic_event_t * ge = NULL;
  while (_running) {
    _c.flush();
    ge = xcb_wait_for_event(_c());

    if (! ge) {
      break;

    } else if (XCB_CLIENT_MESSAGE == (ge->response_type & ~0x80)
        && id == ((xcb_client_message_event_t *)ge)->data.data32[0]) {
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

void
x_event_source::shutdown(void)
{
  _running = false;

  xcb_client_message_event_t e;
  memset(&e, 0, sizeof(xcb_client_message_event_t));

  e.response_type = XCB_CLIENT_MESSAGE;
  e.format = 32;
  e.type = XCB_ATOM_CARDINAL;
  e.window = _c.root_window();
  e.data.data32[0] = id;

  xcb_send_event(_c(), false, _c.root_window(),
                 XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char *)&e);

  _c.flush();
}
