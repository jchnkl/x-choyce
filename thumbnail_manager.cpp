#include "thumbnail_manager.hpp"

#include <algorithm> // find

#include "algorithm.hpp"

thumbnail_manager::thumbnail_manager(x_connection & c,
                                     const layout_t * layout,
                                     const thumbnail_t::factory * factory)
  : _c(c), _layout(layout), _factory(factory)
{
  _c.attach(0, XCB_PROPERTY_NOTIFY, this);
  _c.attach(20, XCB_CONFIGURE_NOTIFY, this);
  _c.update_input(_c.root_window(), XCB_EVENT_MASK_PROPERTY_CHANGE);
}

thumbnail_manager::~thumbnail_manager(void)
{
  _c.detach(XCB_PROPERTY_NOTIFY, this);
  _c.detach(XCB_CONFIGURE_NOTIFY, this);
}

void
thumbnail_manager::show(void)
{
  _active = true;

  update();

  _cyclic_iterator = window_cyclic_iterator(&_windows);
  _next_window = *(_cyclic_iterator + 1);
  _current_window = *_cyclic_iterator;

  for (auto & item : _thumbnails) {
    item.second->show();
  }
}

void
thumbnail_manager::hide(void)
{
  _active = false;

  for (auto & item : _thumbnails) {
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
      _thumbnails.at(*_cyclic_iterator)->select();
    } catch (...) {}

  } else {
    for (auto & item : _thumbnails) {
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
  for (auto & item : _thumbnails) {
    if (item.second->id() == window) {
      try {
        _thumbnails.at(_current_window)->highlight(false).update();
        item.second->highlight(true).update();
        while (*_cyclic_iterator != item.first) ++_cyclic_iterator;
        _next_window = *(_cyclic_iterator + 1);
        _current_window = *_cyclic_iterator;
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
    if (_active
        && e->window == _c.root_window()
        && e->atom == _c.intern_atom("_NET_CLIENT_LIST_STACKING"))
    {
      update();
      reset();
    }
    return true;

  } else if (XCB_CONFIGURE_NOTIFY == (ge->response_type & ~0x80)) {
    if (_active) {
      auto rects = _layout->arrange(query_current_screen(), _windows.size());
      for (size_t i = 0; i < _windows.size(); ++i) {
        try {
          _thumbnails.at(_windows[i])->update(rects[i]).update();
        } catch (...) {}
      }
    }

    return true;
  }

  return false;
}

inline void
thumbnail_manager::reset(void)
{
  for (auto & item : _thumbnails) {
    item.second->highlight(false).show().update();
  }

  bool found = false;
  _cyclic_iterator = window_cyclic_iterator(&_windows);

  // search for current thumbnail
  for (std::size_t i = 0; i < _windows.size(); ++i) {
    if (*_cyclic_iterator == _current_window) {
      found = true;
      break;
    } else {
      ++_cyclic_iterator;
    }
  }

  // search for next thumbnail if current was not found
  if (! found) {
    _cyclic_iterator = window_cyclic_iterator(&_windows);

    for (std::size_t i = 0; i < _windows.size(); ++i) {
      if (*_cyclic_iterator == _next_window) {
        break;
      } else {
        ++_cyclic_iterator;
      }
    }
  }

  _next_window = *(_cyclic_iterator + 1);
  _current_window = *_cyclic_iterator;

  try {
    _thumbnails.at(*_cyclic_iterator)->highlight(true).update();
  } catch (...) {}
}

inline void
thumbnail_manager::update(void)
{
  _windows = _c.net_client_list_stacking();
  auto rects = _layout->arrange(query_current_screen(), _windows.size());

  for (auto item = _thumbnails.begin(); item != _thumbnails.end(); ) {
    auto result = std::find(_windows.begin(), _windows.end(), item->first);
    if (result == _windows.end()) {
      item = _thumbnails.erase(item);
    } else {
      ++item;
    }
  }

  for (size_t i = 0; i < _windows.size(); ++i) {
    auto result = _thumbnails.find(_windows[i]);

    if (result == _thumbnails.end()) {
      _thumbnails[_windows[i]] = _factory->make(_windows[i], rects[i]);
    } else {
      result->second->update(rects[i]);
    }
  }
}

void
thumbnail_manager::next_or_prev(bool next)
{
  try {
    _thumbnails.at(*_cyclic_iterator)->highlight(false).update();
    next ? ++_cyclic_iterator : --_cyclic_iterator;
    _thumbnails.at(*_cyclic_iterator)->highlight(true).update();
  } catch (...) {}

  _next_window = *(_cyclic_iterator + 1);
  _current_window = *_cyclic_iterator;
}

inline xcb_window_t
thumbnail_manager::
nearest_thumbnail(const std::function<bool(double)> & direction)
{
  xcb_window_t thumbnail_id = XCB_NONE;

  try {
    auto & current = _thumbnails.at(_current_window);
    auto & r1 = current->rect();

    // in X (x,y) coordinates are actually flipped on the x axis
    // this means that (0,0) is in the top left corner, not in bottom left
    auto p1 = std::make_tuple(  r1.x() + (r1.width()  / 2),
                              -(r1.y() + (r1.height() / 2)));

    double min_distance = 0xffffffff;

    for (auto & item : _thumbnails) {
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
    auto pos = _c.query_pointer();
    screen = _c.current_screen(pos.first);
  } catch (...) {}

  return screen;
}
