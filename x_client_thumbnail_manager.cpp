#include "x_client_thumbnail_manager.hpp"

x_client_thumbnail_manager::x_client_thumbnail_manager(x_connection & c,
                                                       const layout_t * layout)
  : _c(c), _layout(layout)
{
  _c.register_handler(this);
  update();
  _c.select_input(_c.root_window(), XCB_EVENT_MASK_PROPERTY_CHANGE);
}

void
x_client_thumbnail_manager::handle(xcb_generic_event_t * ge)
{
  if (XCB_PROPERTY_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_property_notify_event_t * e = (xcb_property_notify_event_t *)ge;
    if (e->window == _c.root_window()
        && e->atom == _c.intern_atom("_NET_CLIENT_LIST_STACKING")) {
      _windows = _c.net_client_list_stacking();
      update();
    }
  }
}

void
x_client_thumbnail_manager::update(void)
{
  _x_client_thumbnail_manager.clear();

  _windows.clear();
  _windows = _c.net_client_list_stacking();

  if (_windows.empty()) {
    _cyclic_window_iterator = cyclic_window_iterator_t();

  } else {
    _cyclic_window_iterator = cyclic_window_iterator_t(&_windows);

    auto rectangles = _layout->arrange(_c.current_screen(), _windows.size());

    for (size_t i = 0; i < _windows.size(); ++i) {
      _x_client_thumbnail_manager.emplace(_windows[i],
          thumbnail_ptr(new x_client_thumbnail(_c, rectangles[i], _windows[i])));
    }
  }
}

void
x_client_thumbnail_manager::show(void)
{
  xcb_window_t active_window = _c.net_active_window();
  _cyclic_window_iterator = cyclic_window_iterator_t(&_windows);

  while (*_cyclic_window_iterator != *_windows.begin()) {
    if (*_cyclic_window_iterator == active_window) { break; }
    ++_cyclic_window_iterator;
  }

  for (auto & item : _x_client_thumbnail_manager) {
    item.second->show();
  }
}

void
x_client_thumbnail_manager::hide(void)
{
  for (auto & item : _x_client_thumbnail_manager) {
    item.second->hide();
  }
}

void
x_client_thumbnail_manager::next(void) { next_or_prev(true); }

void
x_client_thumbnail_manager::prev(void) { next_or_prev(false); }

void
x_client_thumbnail_manager::select(void)
{
  try {
    _x_client_thumbnail_manager.at(*_cyclic_window_iterator)->select();
  } catch (...) {}
}

// private

void
x_client_thumbnail_manager::next_or_prev(bool next)
{
  try {
    _x_client_thumbnail_manager.at(*_cyclic_window_iterator)->highlight(false);
  } catch (...) {}
  next ? ++_cyclic_window_iterator : --_cyclic_window_iterator;
  try {
    _x_client_thumbnail_manager.at(*_cyclic_window_iterator)->highlight(true);
  } catch (...) {}
}
