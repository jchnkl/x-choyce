#ifndef _THUMBNAIL_MANAGER_HPP
#define _THUMBNAIL_MANAGER_HPP

#include <vector>
#include <iterator>

#include "cyclic_iterator.hpp"
#include "chooser_t.hpp"
#include "thumbnail_t.hpp"
#include "x_event_handler_t.hpp"
#include "x_connection.hpp"

class thumbnail_manager : public chooser_t
                        , public thumbnail_container_t<std::vector>
{
  public:
    thumbnail_manager(thumbnail_factory_t<std::vector> * thumbnail_factory);

    ~thumbnail_manager(void) {}

    void show(void);
    void hide(void);
    void next(void);
    void prev(void);
    void select(void);

    inline void notify(void);
    inline std::vector<thumbnail_t::thumbnail_ptr> & operator*(void);
    inline std::vector<thumbnail_t::thumbnail_ptr> * operator->(void);

  private:
    typedef std::vector<thumbnail_t::thumbnail_ptr> thumbnail_list_t;
    typedef std::back_insert_iterator<thumbnail_list_t> thumbnail_inserter_t;
    typedef const_cyclic_iterator<thumbnail_list_t> thumbnail_cyclic_iterator;

    thumbnail_factory_t<std::vector> * _thumbnail_factory;

    const unsigned int id = 0;

    bool _visible = false;
    xcb_window_t _next_window = XCB_NONE;
    xcb_window_t _current_window = XCB_NONE;

    std::vector<thumbnail_t::thumbnail_ptr> _thumbnails;
    thumbnail_cyclic_iterator _thumbnail_cyclic_iterator;

    void next_or_prev(bool next);
};

#endif
