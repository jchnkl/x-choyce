#ifndef _X_CLIENT_THUMBNAIL_FACTORY_HPP
#define _X_CLIENT_THUMBNAIL_FACTORY_HPP

#include "layout_t.hpp"
#include "thumbnail_t.hpp"
#include "x_connection.hpp"
#include "x_client_thumbnail.hpp"

template<template<class t = thumbnail_t::thumbnail_ptr,
                  class = std::allocator<t>>
         class container_t>
class x_client_thumbnail_factory : public thumbnail_factory_t<container_t> {
  public:
    typedef std::back_insert_iterator<container_t<thumbnail_t::thumbnail_ptr>>
      back_inserter;

    x_client_thumbnail_factory(x_connection & c, const layout_t * layout);

    void make(back_inserter insert) const;

  private:
    x_connection & _c;
    const layout_t * _layout;
};

#include "x_client_thumbnail_factory.cpp"

#endif
