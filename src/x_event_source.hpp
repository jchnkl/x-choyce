#ifndef X_EVENT_SOURCE
#define X_EVENT_SOURCE

#include <map>
#include <unordered_map>

#include "x_connection.hpp"
#include "x_event_source_t.hpp"
#include "x_event_handler_t.hpp"

class x_connection;

class x_event_source : public x_event_source_t {
  public:
    x_event_source(x_connection & c);
    void attach(priority_t p, event_id_t i, x_event_handler_t * eh);
    void detach(event_id_t i, x_event_handler_t * eh);
    void run_event_loop(void);
    void shutdown(void);

  private:
    typedef std::multimap<priority_t, x_event_handler_t *> priority_map_t;
    typedef std::unordered_map<event_id_t, priority_map_t> event_map_t;

    const uint32_t id = 0 | (unsigned long)this;

    x_connection & m_c;
    bool m_running = true;
    event_map_t m_handler;
};

#endif
