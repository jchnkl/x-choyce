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
    void register_handler(event_id_t i, x_event_handler_t * eh);
    void deregister_handler(event_id_t i, x_event_handler_t * eh);
    void run_event_loop(void);
    void shutdown(void);

  private:
    typedef std::list<x_event_handler_t *> handler_list_t;

    const uint32_t id = 0 | (unsigned long)this;

    x_connection & _c;
    bool _running = true;
    std::unordered_map<unsigned int, handler_list_t> _handler;
};

#endif
