#ifndef THUMBNAIL_T_HPP
#define THUMBNAIL_T_HPP

#include <memory>
#include <iterator>

#include "data_types.hpp"
#include "x_connection.hpp"

class thumbnail_t {
  public:
    typedef std::shared_ptr<thumbnail_t> ptr;
    virtual thumbnail_t & show(void) = 0;
    virtual thumbnail_t & hide(void) = 0;
    virtual thumbnail_t & update(void) = 0;
    virtual thumbnail_t & update(const rectangle &) = 0;
    virtual const rectangle & rect(void) = 0;
    virtual thumbnail_t & raise(void) = 0;
    virtual thumbnail_t & select(void) = 0;
    virtual const unsigned int & id(void) = 0;
    virtual const unsigned int & window(void) = 0;
    virtual thumbnail_t & highlight(bool want_highlight) = 0;

    class factory {
      public:
        virtual ptr
          make(const xcb_window_t &, const rectangle &) const = 0;
    };
};

template<template<class t = thumbnail_t::ptr,
                  class = std::allocator<t>>
         class container_t>
class thumbnail_container_t {
  public:
    virtual void notify(void) = 0;
    virtual container_t<thumbnail_t::ptr> & operator*(void) = 0;
    virtual container_t<thumbnail_t::ptr> * operator->(void) = 0;
};

template<template<class t = thumbnail_t::ptr,
                  class = std::allocator<t>>
         class container_t>
class thumbnail_factory_t {
  public:
    typedef std::back_insert_iterator<container_t<thumbnail_t::ptr>>
      back_insert_iterator;

    virtual void make(back_insert_iterator insert) = 0;
    virtual void manage(unsigned int id, thumbnail_container_t<container_t> * container) = 0;
    virtual void giveup(unsigned int id) = 0;
    virtual void update(unsigned int id) = 0;
};

#endif
