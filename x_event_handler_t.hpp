#ifndef _X_EVENT_HANDLER
#define _X_EVENT_HANDLER

#include <xcb/xcb.h>

class x_event_handler_t {
  public:
    virtual bool handle(xcb_generic_event_t *) = 0;
};

#endif
