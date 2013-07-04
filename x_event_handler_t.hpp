#ifndef _X_EVENT_HANDLER
#define _X_EVENT_HANDLER

#include <xcb/xcb.h>

class x_event_handler {
  public:
    virtual void handle(xcb_generic_event_t *) = 0;
};

#endif
