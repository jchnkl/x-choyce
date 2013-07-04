#include "x_client_thumbnail_manager.hpp"

x_client_thumbnail_manager::x_client_thumbnail_manager(
    x_connection & c,
    const thumbnail_factory_t<std::vector> * thumbnail_factory)
  : _c(c), _thumbnail_factory(thumbnail_factory)
{
  _c.register_handler(this);
  update();
  _c.update_input(_c.root_window(), XCB_EVENT_MASK_PROPERTY_CHANGE);
}

void
x_client_thumbnail_manager::handle(xcb_generic_event_t * ge)
{
  if (XCB_PROPERTY_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_property_notify_event_t * e = (xcb_property_notify_event_t *)ge;
    if (e->window == _c.root_window()
        && e->atom == _c.intern_atom("_NET_CLIENT_LIST_STACKING")) {
      update();
    }
  }
}

void
x_client_thumbnail_manager::show(void)
{
  _thumbnail_cyclic_iterator = thumbnail_cyclic_iterator(&_thumbnails);
  for (auto & thumbnail : _thumbnails) { thumbnail->show(); }
}

void
x_client_thumbnail_manager::hide(void)
{
  for (auto & thumbnail : _thumbnails) { thumbnail->hide(); }
}

void
x_client_thumbnail_manager::next(void) { next_or_prev(true); }

void
x_client_thumbnail_manager::prev(void) { next_or_prev(false); }

void
x_client_thumbnail_manager::select(void)
{
  (*_thumbnail_cyclic_iterator)->select();
}

// private

void
x_client_thumbnail_manager::update(void)
{
  _thumbnails.clear();
  _thumbnail_factory->make(thumbnail_inserter_t(_thumbnails));
}

void
x_client_thumbnail_manager::next_or_prev(bool next)
{
  (*_thumbnail_cyclic_iterator)->highlight(false);
  next ? ++_thumbnail_cyclic_iterator : --_thumbnail_cyclic_iterator;
  (*_thumbnail_cyclic_iterator)->highlight(true);
}
