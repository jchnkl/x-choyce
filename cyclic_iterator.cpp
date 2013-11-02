#ifndef _CYCLIC_ITERATOR_CPP
#define _CYCLIC_ITERATOR_CPP

#include "cyclic_iterator.hpp"

template<typename T>
cyclic_iterator<T>::cyclic_iterator(void) : _valid(false) {}

template<typename T>
cyclic_iterator<T>::cyclic_iterator(T * container)
  : _valid(true), _container(container)
{
  _iterator = _container->begin();
}

template<typename T>
cyclic_iterator<T>::cyclic_iterator(const self_type & rhs)
  : _valid(rhs._valid), _container(rhs._container), _iterator(rhs._iterator) {}

  template<typename T> typename cyclic_iterator<T>::self_type
cyclic_iterator<T>::operator=(const self_type & rhs)
{
  _valid = rhs._valid;
  _container = rhs._container;
  _iterator = rhs._iterator;
  return *this;
}

  template<typename T> typename cyclic_iterator<T>::reference
cyclic_iterator<T>::operator*(void)
{
  return *_iterator;
}

  template<typename T> typename cyclic_iterator<T>::pointer
cyclic_iterator<T>::operator->(void)
{
  return &(*_iterator);
}

  template<typename T> typename cyclic_iterator<T>::self_type
cyclic_iterator<T>::operator+(int n)
{
  cyclic_iterator<T> clone(*this);
  clone += n;
  return clone;
}

  template<typename T> typename cyclic_iterator<T>::self_type
cyclic_iterator<T>::operator-(int n)
{
  cyclic_iterator<T> clone(*this);
  clone -= n;
  return clone;
}

  template<typename T> typename cyclic_iterator<T>::self_type &
cyclic_iterator<T>::operator+=(int n)
{
  if (n != 0) {

    int distance = (_container->end() - _iterator) - 1;
    n %= _container->size();

    if (n <= distance) {
      _iterator += n;
    } else {
      _iterator = _container->begin() + (n - 1 - distance);
    }

  }

  return *this;
}

  template<typename T> typename cyclic_iterator<T>::self_type &
cyclic_iterator<T>::operator-=(int n)
{
  if (n != 0) {

    int distance = _iterator - _container->begin();
    n %= (_container->size() + 0);

    if (n <= distance) {
      _iterator -= n;
    } else {
      _iterator = _container->end() - (n - distance);
    }

  }

  return *this;
}

// prefix operator: ++t
  template<typename T> typename cyclic_iterator<T>::self_type &
cyclic_iterator<T>::operator++(void)
{
  if (_valid) {
    ++_iterator;
    if (_iterator == _container->end()) {
      _iterator = _container->begin();
    }
  }
  return *this;
}

// suffix operator: t++
  template<typename T> typename cyclic_iterator<T>::self_type
cyclic_iterator<T>::operator++(int)
{
  cyclic_iterator<T> clone(*this);
  ++(*this);
  return clone;
}

// prefix operator: --t
  template<typename T> typename cyclic_iterator<T>::self_type &
cyclic_iterator<T>::operator--(void)
{
  if (_valid) {
    if (_iterator == _container->begin()) {
      _iterator = _container->end();
    }
    --_iterator;
  }
  return *this;
}

// suffix operator: t--
  template<typename T> typename cyclic_iterator<T>::self_type
cyclic_iterator<T>::operator--(int)
{
  cyclic_iterator<T> clone(*this);
  --(*this);
  return clone;
}

  template<typename T> bool
cyclic_iterator<T>::operator==(const self_type & rhs)
{
  return _iterator == rhs._iterator;
}

  template<typename T> bool
cyclic_iterator<T>::operator!=(const self_type & rhs)
{
  return ! (*this == rhs);
}

  template<typename T> bool
cyclic_iterator<T>::is_valid(void)
{
  return _valid;
}

// const_cyclic_iterator

template<typename T>
const_cyclic_iterator<T>::const_cyclic_iterator(void) : _valid(false) {}

template<typename T>
const_cyclic_iterator<T>::const_cyclic_iterator(const T * container)
  : _valid(true), _container(container)
{
  _iterator = _container->begin();
}

template<typename T>
const_cyclic_iterator<T>::const_cyclic_iterator(const self_type & rhs)
  : _valid(rhs._valid), _container(rhs._container), _iterator(rhs._iterator) {}

  template<typename T> typename const_cyclic_iterator<T>::self_type
const_cyclic_iterator<T>::operator=(const self_type & rhs)
{
  _valid = rhs._valid;
  _container = rhs._container;
  _iterator = rhs._iterator;
  return *this;
}

  template<typename T> typename const_cyclic_iterator<T>::reference
const_cyclic_iterator<T>::operator*(void)
{
  return *_iterator;
}

  template<typename T> typename const_cyclic_iterator<T>::pointer
const_cyclic_iterator<T>::operator->(void)
{
  return &(*_iterator);
}

  template<typename T> typename const_cyclic_iterator<T>::self_type
const_cyclic_iterator<T>::operator+(int n)
{
  const_cyclic_iterator<T> clone(*this);
  clone += n;
  return clone;
}

  template<typename T> typename const_cyclic_iterator<T>::self_type
const_cyclic_iterator<T>::operator-(int n)
{
  const_cyclic_iterator<T> clone(*this);
  clone -= n;
  return clone;
}

  template<typename T> typename const_cyclic_iterator<T>::self_type &
const_cyclic_iterator<T>::operator+=(int n)
{
  if (n != 0) {

    int distance = (_container->end() - _iterator) - 1;
    n %= _container->size();

    if (n <= distance) {
      _iterator += n;
    } else {
      _iterator = _container->begin() + (n - 1 - distance);
    }

  }

  return *this;
}

  template<typename T> typename const_cyclic_iterator<T>::self_type &
const_cyclic_iterator<T>::operator-=(int n)
{
  if (n != 0) {

    int distance = _iterator - _container->begin();
    n %= (_container->size() + 0);

    if (n <= distance) {
      _iterator -= n;
    } else {
      _iterator = _container->end() - (n - distance);
    }

  }

  return *this;
}

// prefix operator: ++t
  template<typename T> typename const_cyclic_iterator<T>::self_type &
const_cyclic_iterator<T>::operator++(void)
{
  if (_valid) {
    ++_iterator;
    if (_iterator == _container->end()) {
      _iterator = _container->begin();
    }
  }
  return *this;
}

// suffix operator: t++
  template<typename T> typename const_cyclic_iterator<T>::self_type
const_cyclic_iterator<T>::operator++(int)
{
  cyclic_iterator<T> clone(*this);
  ++(*this);
  return clone;
}

// prefix operator: --t
  template<typename T> typename const_cyclic_iterator<T>::self_type &
const_cyclic_iterator<T>::operator--(void)
{
  if (_valid) {
    if (_iterator == _container->begin()) {
      _iterator = _container->end();
    }
    --_iterator;
  }
  return *this;
}

// suffix operator: t--
  template<typename T> typename const_cyclic_iterator<T>::self_type
const_cyclic_iterator<T>::operator--(int)
{
  cyclic_iterator<T> clone(*this);
  --(*this);
  return clone;
}

  template<typename T> bool
const_cyclic_iterator<T>::operator==(const self_type & rhs)
{
  return _iterator == rhs._iterator;
}

  template<typename T> bool
const_cyclic_iterator<T>::operator!=(const self_type & rhs)
{
  return ! (*this == rhs);
}

  template<typename T> bool
const_cyclic_iterator<T>::is_valid(void)
{
  return _valid;
}

#endif
