#include "thumbnail_manager.hpp"

#include <algorithm> // find

thumbnail_manager::thumbnail_manager(x_connection & c,
                                     const layout_t * layout,
                                     const thumbnail_t::factory * factory)
  : _c(c), _layout(layout), _factory(factory)
{
  _c.register_handler(XCB_PROPERTY_NOTIFY, this);
  _c.update_input(_c.root_window(), XCB_EVENT_MASK_PROPERTY_CHANGE);
  update();
}

thumbnail_manager::~thumbnail_manager(void)
{
  _c.deregister_handler(XCB_PROPERTY_NOTIFY, this);
}

void
thumbnail_manager::show(void)
{
  _visible = true;

  _cyclic_iterator = window_cyclic_iterator(&_windows);

  for (auto & item : _thumbnails) {
    item.second->show();
  }

  _next_window = *(_cyclic_iterator + 1);
  _current_window = *_cyclic_iterator;
}

void
thumbnail_manager::hide(void)
{
  _visible = false;

  for (auto & item : _thumbnails) {
    item.second->hide();
    item.second->highlight(false);
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

bool
thumbnail_manager::handle(xcb_generic_event_t * ge)
{
  if (XCB_PROPERTY_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_property_notify_event_t * e = (xcb_property_notify_event_t *)ge;
    if (e->window == _c.root_window()
        && e->atom == _c.intern_atom("_NET_CLIENT_LIST_STACKING")) {
      update();
    }
    return true;
  }

  return false;
}

inline void
thumbnail_manager::reset(void)
{
  if (! _visible) return;

  for (auto & item : _thumbnails) {
    item.second->show();
    item.second->highlight(false);
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
    _thumbnails.at(*_cyclic_iterator)->highlight(true);
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
      _thumbnails[_windows[i]] = _factory->make(_c, _windows[i], rects[i]);
    } else {
      result->second->update(rects[i]);
    }
  }

  reset();
}

void
thumbnail_manager::next_or_prev(bool next)
{
  try {
    _thumbnails.at(*_cyclic_iterator)->highlight(false);
    next ? ++_cyclic_iterator : --_cyclic_iterator;
    _thumbnails.at(*_cyclic_iterator)->highlight(true);
  } catch (...) {}

  _next_window = *(_cyclic_iterator + 1);
  _current_window = *_cyclic_iterator;
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
