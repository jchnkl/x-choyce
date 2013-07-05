#ifndef _THUMBNAIL_T_HPP
#define _THUMBNAIL_T_HPP

#include <memory>
#include <iterator>

class thumbnail_t {
  public:
    typedef std::shared_ptr<thumbnail_t> thumbnail_ptr;
    virtual void show(void) const = 0;
    virtual void hide(void) const = 0;
    virtual void update(void) const = 0;
    virtual void select(void) const = 0;
    virtual void highlight(bool want_highlight) const = 0;
};

template<template<class t = thumbnail_t::thumbnail_ptr,
                  class = std::allocator<t>>
         class container_t>
class thumbnail_factory_t {
  public:
    typedef container_t<thumbnail_t::thumbnail_ptr> thumbnail_container_t;
    typedef std::back_insert_iterator<container_t<thumbnail_t::thumbnail_ptr>>
      back_insert_iterator;

    virtual void make(back_insert_iterator insert) = 0;
    virtual void manage(unsigned int id, thumbnail_container_t & container) = 0;
    virtual void giveup(unsigned int id) = 0;
    virtual void update(unsigned int id) = 0;
};

#endif
