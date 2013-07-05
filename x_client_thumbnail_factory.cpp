#ifndef _X_CLIENT_THUMBNAIL_FACTORY_CPP
#define _X_CLIENT_THUMBNAIL_FACTORY_CPP

#include <algorithm>

#include "x_client_thumbnail_factory.hpp"

template<template<class t = thumbnail_t::thumbnail_ptr,
                  class = std::allocator<t>>
         class container_t>
x_client_thumbnail_factory<container_t>::x_client_thumbnail_factory(
    x_connection & c, const layout_t * layout)
  : _c(c), _layout(layout)
{
  _c.register_handler(this);
  _c.update_input(_c.root_window(), XCB_EVENT_MASK_PROPERTY_CHANGE);
  update(true);
}

template<template<class t = thumbnail_t::thumbnail_ptr,
                  class = std::allocator<t>>
         class container_t>
x_client_thumbnail_factory<container_t>::~x_client_thumbnail_factory(void)
{
  _c.unregister_handler(this);
}

template<template<class t = thumbnail_t::thumbnail_ptr,
                  class = std::allocator<t>>
         class container_t>
void
x_client_thumbnail_factory<container_t>::make(back_insert_iterator insert)
{
  auto windows = _c.net_client_list_stacking();
  auto rects = _layout->arrange(_c.current_screen(), windows.size());

  for (size_t i = 0; i < windows.size(); ++i) {
    *insert = thumbnail_t::thumbnail_ptr(
          new x_client_thumbnail(_c, rects[i], windows[i]));
  }
}

template<template<class t = thumbnail_t::thumbnail_ptr,
                  class = std::allocator<t>>
         class container_t>
void
x_client_thumbnail_factory<container_t>::
manage(unsigned int id, thumbnail_container_t & container)
{
  _container[id] = &container;
  update(container);
}

template<template<class t = thumbnail_t::thumbnail_ptr,
                  class = std::allocator<t>>
         class container_t>
void
x_client_thumbnail_factory<container_t>::giveup(unsigned int id)
{
  _container.erase(id);
}

template<template<class t = thumbnail_t::thumbnail_ptr,
                  class = std::allocator<t>>
         class container_t>
void
x_client_thumbnail_factory<container_t>::update(unsigned int id)
{
  update(false, id);
}

template<template<class t = thumbnail_t::thumbnail_ptr,
                  class = std::allocator<t>>
         class container_t>
void
x_client_thumbnail_factory<container_t>::handle(xcb_generic_event_t * ge)
{
  if (XCB_PROPERTY_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_property_notify_event_t * e = (xcb_property_notify_event_t *)ge;
    if (e->window == _c.root_window()
        && e->atom == _c.intern_atom("_NET_CLIENT_LIST_STACKING")) {
      update(true);
    }
  }
}

template<template<class t = thumbnail_t::thumbnail_ptr,
                  class = std::allocator<t>>
         class container_t>
void
x_client_thumbnail_factory<container_t>::update(bool all, unsigned int id)
{
  _windows = _c.net_client_list_stacking();
  auto rects = _layout->arrange(_c.current_screen(), _windows.size());

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
      _thumbnails[_windows[i]] = x_client_thumbnail_ptr(
          new x_client_thumbnail(_c, rects[i], _windows[i]));
    } else {
      result->second->update_rectangle(rects[i]);
    }
  }

  if (all) {
    for (auto & item : _container) {
      update(*item.second);
    }
  } else {
    try {
      update(*_container.at(id));
    } catch (...) {}
  }
}

template<template<class t = thumbnail_t::thumbnail_ptr,
                  class = std::allocator<t>>
         class container_t>
void
x_client_thumbnail_factory<container_t>::
update(thumbnail_container_t & container)
{
  container.clear();
  for (auto & window : _windows) {
    try {
      container.push_back(_thumbnails.at(window));
    } catch (...) {}
  }
}

#endif
