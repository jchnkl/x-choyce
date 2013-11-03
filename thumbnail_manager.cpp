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
  _visible = true;

  _thumbnail_factory->update(id);
  _thumbnail_cyclic_iterator = thumbnail_cyclic_iterator(&_thumbnails);
  for (auto & thumbnail : _thumbnails) { thumbnail->show(); }

  _next_window = (*(_thumbnail_cyclic_iterator + 1))->window();
  _current_window = (*_thumbnail_cyclic_iterator)->window();
}

void
thumbnail_manager::hide(void)
{
  _visible = false;

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
thumbnail_manager::select(const xcb_window_t & window)
{
  if (window == XCB_NONE) {
    (*_thumbnail_cyclic_iterator)->select();

  } else {
    for (auto & thumbnail : _thumbnails) {
      if (thumbnail->id() == window) {
        thumbnail->select();
      }
    }
  }
}

inline void
thumbnail_manager::notify(void)
{
  if (! _visible) return;

  for (auto & thumbnail : _thumbnails) {
    thumbnail->show();
    thumbnail->highlight(false);
  }

  bool found = false;
  _thumbnail_cyclic_iterator = thumbnail_cyclic_iterator(&_thumbnails);

  // search for current thumbnail
  for (std::size_t i = 0; i < _thumbnails.size(); ++i) {
    if ((*_thumbnail_cyclic_iterator)->window() == _current_window) {
      found = true;
      break;
    } else {
      ++_thumbnail_cyclic_iterator;
    }
  }

  // search for next thumbnail if current was not found
  if (! found) {
    _thumbnail_cyclic_iterator = thumbnail_cyclic_iterator(&_thumbnails);

    for (std::size_t i = 0; i < _thumbnails.size(); ++i) {
      if ((*_thumbnail_cyclic_iterator)->window() == _next_window) {
        break;
      } else {
        ++_thumbnail_cyclic_iterator;
      }
    }
  }

  _next_window = (*(_thumbnail_cyclic_iterator + 1))->window();
  _current_window = (*_thumbnail_cyclic_iterator)->window();

  (*_thumbnail_cyclic_iterator)->highlight(true);
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

  _current_window = (*_thumbnail_cyclic_iterator)->window();
  _next_window = (*(_thumbnail_cyclic_iterator + 1))->window();
}
