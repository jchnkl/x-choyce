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
    typedef std::back_insert_iterator<container_t<thumbnail_t::thumbnail_ptr>>
      back_inserter;

    virtual void make(back_inserter insert) const = 0;
};

#endif
