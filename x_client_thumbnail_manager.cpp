#include "x_client_thumbnail_manager.hpp"

x_client_thumbnail_manager::x_client_thumbnail_manager(
    x_connection & c,
    thumbnail_factory_t<std::vector> * thumbnail_factory)
  : _c(c), _thumbnail_factory(thumbnail_factory)
{
  _thumbnail_factory->manage(id, _thumbnails);
}

void
x_client_thumbnail_manager::show(void)
{
  _thumbnail_factory->update(id);
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

void
x_client_thumbnail_manager::next_or_prev(bool next)
{
  (*_thumbnail_cyclic_iterator)->highlight(false);
  next ? ++_thumbnail_cyclic_iterator : --_thumbnail_cyclic_iterator;
  (*_thumbnail_cyclic_iterator)->highlight(true);
}
