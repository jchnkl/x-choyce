#include "thumbnail_manager.hpp"

thumbnail_manager::thumbnail_manager(
    thumbnail_factory_t<std::vector> * thumbnail_factory)
  : _thumbnail_factory(thumbnail_factory)
{
  _thumbnail_factory->manage(id, this);
}

void
thumbnail_manager::show(void)
{
  _thumbnail_factory->update(id);
  _thumbnail_cyclic_iterator = thumbnail_cyclic_iterator(&_thumbnails);
  for (auto & thumbnail : _thumbnails) { thumbnail->show(); }
}

void
thumbnail_manager::hide(void)
{
  for (auto & thumbnail : _thumbnails) {
    thumbnail->hide();
    thumbnail->highlight(false);
  }
}

void
thumbnail_manager::next(void) { next_or_prev(true); }

void
thumbnail_manager::prev(void) { next_or_prev(false); }

void
thumbnail_manager::select(void)
{
  (*_thumbnail_cyclic_iterator)->select();
}

inline void
thumbnail_manager::notify(void)
{
}

inline std::vector<thumbnail_t::thumbnail_ptr> &
thumbnail_manager::operator*(void)
{
  return _thumbnails;
}

inline std::vector<thumbnail_t::thumbnail_ptr> *
thumbnail_manager::operator->(void)
{
  return &_thumbnails;
}

void
thumbnail_manager::next_or_prev(bool next)
{
  (*_thumbnail_cyclic_iterator)->highlight(false);
  next ? ++_thumbnail_cyclic_iterator : --_thumbnail_cyclic_iterator;
  (*_thumbnail_cyclic_iterator)->highlight(true);
}
