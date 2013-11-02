#ifndef _X_CLIENT_THUMBNAIL_FACTORY_HPP
#define _X_CLIENT_THUMBNAIL_FACTORY_HPP

#include <unordered_map>

#include "layout_t.hpp"
#include "thumbnail_t.hpp"
#include "x_connection.hpp"
#include "x_client_thumbnail_gl.hpp"

template<template<class t = thumbnail_t::thumbnail_ptr,
                  class = std::allocator<t>>
         class container_t>
class x_client_thumbnail_factory : public x_event_handler_t
                                 , public thumbnail_factory_t<container_t> {
  public:
    typedef container_t<thumbnail_t::thumbnail_ptr> thumbnail_container_t;
    typedef std::back_insert_iterator<container_t<thumbnail_t::thumbnail_ptr>>
      back_insert_iterator;

    x_client_thumbnail_factory(x_connection & c, const layout_t * layout);
    ~x_client_thumbnail_factory(void);

    void make(back_insert_iterator insert);
    void manage(unsigned int id, thumbnail_container_t & container);
    void giveup(unsigned int id);
    void update(unsigned int id);

    bool handle(xcb_generic_event_t * ge);

  private:
    typedef std::shared_ptr<x_client_thumbnail> x_client_thumbnail_ptr;
    x_connection & _c;
    const layout_t * _layout;

    std::vector<xcb_window_t> _windows;
    std::unordered_map<xcb_window_t, x_client_thumbnail_ptr> _thumbnails;

    std::unordered_map<unsigned int, thumbnail_container_t *> _container;

    void update(bool all, unsigned int id = 0);
    void update(thumbnail_container_t & container);

    rectangle query_current_screen(void);
};

#include "x_client_thumbnail_factory.cpp"

#endif
