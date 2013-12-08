#ifndef THUMBNAIL_MANAGER_HPP
#define THUMBNAIL_MANAGER_HPP

#include <deque>
#include <unordered_map>

#include "cyclic_iterator.hpp"
#include "layout_t.hpp"
#include "chooser_t.hpp"
#include "thumbnail_t.hpp"
#include "x_event_handler_t.hpp"
#include "x_connection.hpp"

class thumbnail_manager : public chooser_t
                        , public x_event_handler_t
{
  public:
    thumbnail_manager(x_connection &,
                      const layout_t *,
                      const thumbnail_t::factory *);

    ~thumbnail_manager(void);

    void show(void);
    void hide(void);
    void next(void);
    void prev(void);
    void east(void);
    void west(void);
    void north(void);
    void south(void);
    void select(const xcb_window_t & window = XCB_NONE);
    void highlight(const unsigned int & window = XCB_NONE);

    bool handle(xcb_generic_event_t *);

  private:
    typedef std::deque<xcb_window_t> window_list_t;
    typedef const_cyclic_iterator<window_list_t> window_cyclic_iterator;

    x_connection & m_c;
    const layout_t * m_layout;
    const thumbnail_t::factory * m_factory;

    window_cyclic_iterator m_cyclic_iterator;

    window_list_t m_windows;
    std::unordered_map<xcb_window_t, thumbnail_t::ptr> m_thumbnails;

    bool m_active = false;
    xcb_window_t m_next_window = XCB_NONE;
    xcb_window_t m_current_window = XCB_NONE;

    thumbnail_manager & reset(void);
    thumbnail_manager & update(void);
    thumbnail_manager & next_or_prev(bool next);
    rectangle query_current_screen(void);

    bool is_east(double);
    bool is_west(double);
    bool is_north(double);
    bool is_south(double);
    xcb_window_t nearest_thumbnail(const std::function<bool(double)> &);

    thumbnail_manager &
      foreach(std::function<void(const thumbnail_t::ptr &)> f)
    {
      for (auto & item : m_thumbnails) {
        f(item.second);
      }
      return *this;
    }

};

#endif
