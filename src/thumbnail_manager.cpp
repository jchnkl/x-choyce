#include "thumbnail_manager.hpp"

#include <algorithm> // find

#include "algorithm.hpp"

thumbnail_manager::thumbnail_manager(x_connection & c,
                                     const layout_t * layout,
                                     const thumbnail_t::factory * factory)
  : m_c(c), m_layout(layout), m_factory(factory)
{
  m_c.attach(0, XCB_PROPERTY_NOTIFY, this);
  m_c.attach(20, XCB_CONFIGURE_NOTIFY, this);
  m_c.update_input(m_c.root_window(), XCB_EVENT_MASK_PROPERTY_CHANGE);
}

thumbnail_manager::~thumbnail_manager(void)
{
  m_c.detach(XCB_PROPERTY_NOTIFY, this);
  m_c.detach(XCB_CONFIGURE_NOTIFY, this);
}

void
thumbnail_manager::show(void)
{
  m_active = true;

  update();

  m_cyclic_iterator = window_cyclic_iterator(&m_windows);
  m_next_window = *(m_cyclic_iterator + 1);
  m_current_window = *m_cyclic_iterator;

  foreach([](const thumbnail_t::ptr & t) { t->show().update(); });
}

void
thumbnail_manager::hide(void)
{
  m_active = false;

  for (auto & item : m_thumbnails) {
    item.second->hide();
  }
}

void
thumbnail_manager::next(void) { next_or_prev(true); }

void
thumbnail_manager::prev(void) { next_or_prev(false); }

void
thumbnail_manager::select(const xcb_window_t & window)
{
  if (window == XCB_NONE) {
    try {
      m_thumbnails.at(*m_cyclic_iterator)->select();
    } catch (...) {}

  } else {
    for (auto & item : m_thumbnails) {
      if (item.second->id() == window) {
        item.second->select();
        break;
      }
    }
  }
}

void
thumbnail_manager::highlight(const unsigned int & window)
{
  for (auto & item : m_thumbnails) {
    if (item.second->id() == window) {
      try {
        m_thumbnails.at(m_current_window)->highlight(false).update();
        item.second->highlight(true).update();
        while (*m_cyclic_iterator != item.first) ++m_cyclic_iterator;
        m_next_window = *(m_cyclic_iterator + 1);
        m_current_window = *m_cyclic_iterator;
      } catch (...) {}

      break;
    }
  }
}

void
thumbnail_manager::east(void)
{
  xcb_window_t tid = nearest_thumbnail(
      std::bind(&thumbnail_manager::is_east, this, std::placeholders::_1));
  if (tid != XCB_NONE) highlight(tid);
}

void
thumbnail_manager::west(void)
{
  xcb_window_t tid = nearest_thumbnail(
      std::bind(&thumbnail_manager::is_west, this, std::placeholders::_1));
  if (tid != XCB_NONE) highlight(tid);
}

void
thumbnail_manager::north(void)
{
  xcb_window_t tid = nearest_thumbnail(
      std::bind(&thumbnail_manager::is_north, this, std::placeholders::_1));
  if (tid != XCB_NONE) highlight(tid);
}

void
thumbnail_manager::south(void)
{
  xcb_window_t tid = nearest_thumbnail(
      std::bind(&thumbnail_manager::is_south, this, std::placeholders::_1));
  if (tid != XCB_NONE) highlight(tid);
}

bool
thumbnail_manager::handle(xcb_generic_event_t * ge)
{
  if (XCB_PROPERTY_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_property_notify_event_t * e = (xcb_property_notify_event_t *)ge;
    if (m_active
        && e->window == m_c.root_window()
        && e->atom == m_c.intern_atom("_NET_CLIENT_LIST_STACKING"))
    {
      update();
      reset();
      foreach([](const thumbnail_t::ptr & t) { t->show().update(); });
    }

    return true;

  } else if (XCB_CONFIGURE_NOTIFY == (ge->response_type & ~0x80)) {
    if (m_active) {
      auto rects = m_layout->arrange(query_current_screen(), m_windows.size());
      for (size_t i = 0; i < m_windows.size(); ++i) {
        m_thumbnails.at(m_windows[i])->update(rects[i]).update();
      }
    }

    return true;
  }

  return false;
}

inline
thumbnail_manager &
thumbnail_manager::reset(void)
{
  bool found = false;
  m_cyclic_iterator = window_cyclic_iterator(&m_windows);

  // search for current thumbnail
  for (std::size_t i = 0; i < m_windows.size(); ++i) {
    if (*m_cyclic_iterator == m_current_window) {
      found = true;
      break;
    } else {
      ++m_cyclic_iterator;
    }
  }

  // search for next thumbnail if current was not found
  if (! found) {
    m_cyclic_iterator = window_cyclic_iterator(&m_windows);

    for (std::size_t i = 0; i < m_windows.size(); ++i) {
      if (*m_cyclic_iterator == m_next_window) {
        break;
      } else {
        ++m_cyclic_iterator;
      }
    }
  }

  m_next_window = *(m_cyclic_iterator + 1);
  m_current_window = *m_cyclic_iterator;

  return *this;
}

inline
thumbnail_manager &
thumbnail_manager::update(void)
{
  auto windows = m_c.net_client_list_stacking();
  m_windows = window_list_t(windows.begin(), windows.end());
  auto rects = m_layout->arrange(query_current_screen(), m_windows.size());

  for (auto item = m_thumbnails.begin(); item != m_thumbnails.end(); ) {
    auto result = std::find(m_windows.begin(), m_windows.end(), item->first);
    if (result == m_windows.end()) {
      item = m_thumbnails.erase(item);
    } else {
      ++item;
    }
  }

  for (size_t i = 0; i < m_windows.size(); ++i) {
    auto result = m_thumbnails.find(m_windows[i]);

    if (result == m_thumbnails.end()) {
      m_thumbnails[m_windows[i]] = m_factory->make(m_windows[i], rects[i]);
    } else {
      result->second->update(rects[i]);
    }
  }

  return *this;
}

inline
thumbnail_manager &
thumbnail_manager::next_or_prev(bool next)
{
  try {
    m_thumbnails.at(*m_cyclic_iterator)->highlight(false).update();
    next ? ++m_cyclic_iterator : --m_cyclic_iterator;
    m_thumbnails.at(*m_cyclic_iterator)->highlight(true).update();
  } catch (...) {}

  m_next_window = *(m_cyclic_iterator + 1);
  m_current_window = *m_cyclic_iterator;

  return *this;
}

inline xcb_window_t
thumbnail_manager::
nearest_thumbnail(const std::function<bool(double)> & direction)
{
  xcb_window_t thumbnail_id = XCB_NONE;

  try {
    auto & current = m_thumbnails.at(m_current_window);
    auto & r1 = current->rect();

    // in X (x,y) coordinates are actually flipped on the x axis
    // this means that (0,0) is in the top left corner, not in bottom left
    auto p1 = std::make_tuple(  r1.x() + (r1.width()  / 2),
                              -(r1.y() + (r1.height() / 2)));

    double min_distance = 0xffffffff;

    for (auto & item : m_thumbnails) {
      if (item.second->id() == current->id()) {
        continue;
      } else {
        auto & r2 = item.second->rect();
        // in X (x,y) coordinates are actually flipped on the x axis
        // this means that (0,0) is in the top left corner, not in bottom left
        auto p2 = std::make_tuple(  r2.x() + (r2.width()  / 2),
                                  -(r2.y() + (r2.height() / 2)));

        if (direction(algorithm::angle()(p1, p2))) {
          double distance = algorithm::distance()(p1, p2);
          if (distance < min_distance) {
            min_distance = distance;
            thumbnail_id = item.second->id();
          }
        }
      }
    }

  } catch (...) {}

  return thumbnail_id;
}

// 2*M_PI ^= 360°
// 2*M_PI - M_PI/4 ^= 315°
// 2*M_PI - M_PI/2 ^= 270°
// M_PI + M_PI/4 ^= 225°
// M_PI ^= 180°
// M_PI - M_PI/4 ^= 135°
// M_PI/2 ^= 90°
// M_PI/4 ^= 45°

inline bool
thumbnail_manager::is_east(double angle)
{
  // (>=0° && <=45°) || >= 315°
  return (angle >= 0 && angle <= M_PI/4) || angle >= 7*M_PI/4;
}

inline bool
thumbnail_manager::is_west(double angle)
{
  // >=135° && <=225°
  return angle >= 3*M_PI/4 && angle <= 5*M_PI/4;
}

inline bool
thumbnail_manager::is_north(double angle)
{
  // >=45° && <=135°
  return angle >= M_PI/4 && angle <= 3*M_PI/4;
}

inline bool
thumbnail_manager::is_south(double angle)
{
  // >=225° && <= 315°
  return angle >= 5*M_PI/4 && angle <= 7*M_PI/4;
}

rectangle
thumbnail_manager::query_current_screen(void)
{
  rectangle screen = { 0, 0, 800, 600 };

  try {
    auto pos = m_c.query_pointer();
    screen = m_c.current_screen(pos.first);
  } catch (...) {}

  return screen;
}
