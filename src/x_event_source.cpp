#include "x_event_source.hpp"

#include <algorithm>
#include <X11/Xlibint.h>

x_event_source::x_event_source(x_connection & c) : m_c(c)
{
  m_c.update_input(m_c.root_window(), XCB_EVENT_MASK_STRUCTURE_NOTIFY);
}

void
x_event_source::attach(priority_t p, event_id_t i, x_event_handler_t * eh)
{
  _handler[i].insert({p, eh});
}

void
x_event_source::detach(event_id_t i, x_event_handler_t * eh)
{
  try {
    auto & ps = _handler.at(i);
      for (auto it = ps.begin(); it != ps.end(); ) {
        if (it->second == eh) {
          it = ps.erase(it);
        } else {
          ++it;
        }
    }
  } catch (...) {}
}

void
x_event_source::run_event_loop(void)
{
  xcb_generic_event_t * ge = NULL;
  while (_running) {
    m_c.flush();
    ge = xcb_wait_for_event(m_c());

    if (! ge) {
      break;

    } else if (XCB_CLIENT_MESSAGE == (ge->response_type & ~0x80)
        && id == ((xcb_client_message_event_t *)ge)->data.data32[0]) {
      break;

    } else {
      bool taken = false;
      unsigned int response_type = ge->response_type & ~0x80;

      try {
        for (auto & item : _handler.at(response_type)) {
          item.second->handle(ge);
        }
        taken = true;
      } catch (...) {}

      if (! taken) {
        // Check if a custom XEvent constructor was registered in xlib for this
        // event type, and call it discarding the constructed XEvent if any.
        // XESetWireToEvent might be used by libraries to intercept messages from
        // the X server e.g. the OpenGL lib waiting for DRI2 events.

        Bool (*proc)(Display*, XEvent*, xEvent*) =
          XESetWireToEvent(m_c.dpy(), response_type, 0);
        if (proc) {
          XESetWireToEvent(m_c.dpy(), response_type, proc);
          XEvent dummy;
          ge->sequence = LastKnownRequestProcessed(m_c.dpy());
          proc(m_c.dpy(), &dummy, (xEvent*)ge);
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
  e.window = m_c.root_window();
  e.data.data32[0] = id;

  xcb_send_event(m_c(), false, m_c.root_window(),
                 XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char *)&e);

  m_c.flush();
}
