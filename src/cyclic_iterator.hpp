#ifndef _CYCLIC_ITERATOR_HPP
#define _CYCLIC_ITERATOR_HPP

#include <iostream>
template<typename T>
class cyclic_iterator {
  public:
    typedef typename T::value_type value_type;
    typedef value_type * pointer;
    typedef value_type & reference;
    typedef cyclic_iterator self_type;
    cyclic_iterator(void);
    cyclic_iterator(T * container);
    cyclic_iterator(const self_type & rhs);
    self_type operator=(const self_type & rhs);
    reference operator*(void);
    pointer operator->(void);
    self_type operator+(int);
    self_type operator-(int);
    self_type & operator+=(int);
    self_type & operator-=(int);
    self_type & operator++(void);
    self_type operator++(int);
    self_type & operator--(void);
    self_type operator--(int);
    bool operator==(const self_type & rhs);
    bool operator!=(const self_type & rhs);
    bool is_valid(void);
  private:
    bool _valid;
    T * _container;
    typename T::iterator _iterator;
};

template<typename T>
class const_cyclic_iterator {
  public:
    typedef typename T::value_type value_type;
    typedef const value_type * pointer;
    typedef const value_type & reference;
    typedef const_cyclic_iterator self_type;
    const_cyclic_iterator(void);
    const_cyclic_iterator(const T * container);
    const_cyclic_iterator(const self_type & rhs);
    self_type operator=(const self_type & rhs);
    reference operator*(void);
    pointer operator->(void);
    self_type operator+(int);
    self_type operator-(int);
    self_type & operator+=(int);
    self_type & operator-=(int);
    self_type & operator++(void);
    self_type operator++(int);
    self_type & operator--(void);
    self_type operator--(int);
    bool operator==(const self_type & rhs);
    bool operator!=(const self_type & rhs);
    bool is_valid(void);
  private:
    bool _valid;
    const T * _container;
    typename T::const_iterator _iterator;
};

#include "cyclic_iterator.cpp"

#endif
