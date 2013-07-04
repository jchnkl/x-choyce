#ifndef _X_CLIENT_THUMBNAIL_FACTORY_CPP
#define _X_CLIENT_THUMBNAIL_FACTORY_CPP

#include "x_client_thumbnail_factory.hpp"

template<template<class t = thumbnail_t::thumbnail_ptr,
                  class = std::allocator<t>>
         class container_t>
x_client_thumbnail_factory<container_t>::x_client_thumbnail_factory(
    x_connection & c, const layout_t * layout)
  : _c(c), _layout(layout) {}

template<template<class t = thumbnail_t::thumbnail_ptr,
                  class = std::allocator<t>>
         class container_t>
void
x_client_thumbnail_factory<container_t>::make(back_inserter insert) const
{
  auto windows = _c.net_client_list_stacking();
  auto rects = _layout->arrange(_c.current_screen(), windows.size());

  for (size_t i = 0; i < windows.size(); ++i) {
    *insert = thumbnail_t::thumbnail_ptr(
          new x_client_thumbnail(_c, rects[i], windows[i]));
  }
}

#endif
