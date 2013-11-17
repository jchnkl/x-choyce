#ifndef _X_EVENT_SOURCE_HPP
#define _X_EVENT_SOURCE_HPP

#include "x_event_handler_t.hpp"

struct x_event_source_t {
  typedef unsigned int event_id_t;

  virtual void
    attach(event_id_t i, x_event_handler_t * eh) = 0;
  virtual void
    detach(event_id_t i, x_event_handler_t * eh) = 0;
  virtual void run_event_loop(void) = 0;
  virtual void shutdown(void) = 0;
};

#endif
