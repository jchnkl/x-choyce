#ifndef _X_EVENT_SOURCE
#define _X_EVENT_SOURCE

#include <iostream>
#include <algorithm>
#include <list>
#include <unordered_map>

#include <X11/Xlibint.h>

#include "x_connection.hpp"
#include "x_event_source_t.hpp"
#include "x_event_handler_t.hpp"

class x_event_source : public x_event_source_t {
  public:
    x_event_source(x_connection & c);
    void register_handler(unsigned int event_id, x_event_handler_t * eh);
    void deregister_handler(unsigned int event_id, x_event_handler_t * eh);
    void run_event_loop(void);

  private:
    typedef std::list<x_event_handler_t *> handler_list_t;

    x_connection & _c;
    std::unordered_map<unsigned int, handler_list_t> _handler;
};

#endif
