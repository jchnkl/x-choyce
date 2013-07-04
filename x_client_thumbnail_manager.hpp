#ifndef _X_CLIENT_THUMBNAIL_MANAGER_HPP
#define _X_CLIENT_THUMBNAIL_MANAGER_HPP

#include <unordered_map>

#include "cyclic_iterator.hpp"
#include "layout_t.hpp"
#include "chooser_t.hpp"
#include "x_event_handler_t.hpp"
#include "x_client_thumbnail.hpp"

class x_client_thumbnail_manager : public chooser_t
                                 , public x_event_handler_t {
  public:
    x_client_thumbnail_manager(x_connection & c,
                               const layout_t * layout);

    ~x_client_thumbnail_manager(void)
    {
      _c.unregister_handler(this);
    }

    void show(void);
    void hide(void);
    void next(void);
    void prev(void);
    void select(void);
    void update(void);

    void handle(xcb_generic_event_t * ge);

  private:
    typedef std::shared_ptr<thumbnail_t> thumbnail_ptr;
    typedef std::vector<xcb_window_t> window_list_t;
    typedef const_cyclic_iterator<window_list_t> cyclic_window_iterator_t;

    x_connection & _c;
    const layout_t * _layout;

    window_list_t _windows;
    cyclic_window_iterator_t _cyclic_window_iterator;

    std::unordered_map<xcb_window_t, thumbnail_ptr> _x_client_thumbnail_manager;

    void next_or_prev(bool next);
};

#endif
