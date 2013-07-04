#ifndef _X_EVENT_SOURCE_HPP
#define _X_EVENT_SOURCE_HPP

#include "x_event_handler.hpp"

struct x_event_source_t {
  virtual void register_handler(x_event_handler * eh) = 0;
  virtual void unregister_handler(x_event_handler * eh) = 0;
  virtual void run_event_loop(void) = 0;
};

#endif
