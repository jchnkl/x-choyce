#ifndef CYCLIC_ITERATOR_CPP
#define CYCLIC_ITERATOR_CPP

#include "cyclic_iterator.hpp"

template<typename T>
cyclic_iterator<T>::cyclic_iterator(void) : m_valid(false) {}

template<typename T>
cyclic_iterator<T>::cyclic_iterator(T * container)
  : m_valid(true), m_container(container)
{
  m_iterator = m_container->begin();
}

template<typename T>
cyclic_iterator<T>::cyclic_iterator(const self_type & rhs)
  : m_valid(rhs.m_valid), m_container(rhs.m_container), m_iterator(rhs.m_iterator) {}

  template<typename T> typename cyclic_iterator<T>::self_type
cyclic_iterator<T>::operator=(const self_type & rhs)
{
  m_valid = rhs.m_valid;
  m_container = rhs.m_container;
  m_iterator = rhs.m_iterator;
  return *this;
}

  template<typename T> typename cyclic_iterator<T>::reference
cyclic_iterator<T>::operator*(void)
{
  return *m_iterator;
}

  template<typename T> typename cyclic_iterator<T>::pointer
cyclic_iterator<T>::operator->(void)
{
  return &(*m_iterator);
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
  if (n != 0 && ! m_container->empty()) {

    int distance = (m_container->end() - m_iterator) - 1;
    n %= m_container->size();

    if (n <= distance) {
      m_iterator += n;
    } else {
      m_iterator = m_container->begin() + (n - 1 - distance);
    }

  }

  return *this;
}

  template<typename T> typename cyclic_iterator<T>::self_type &
cyclic_iterator<T>::operator-=(int n)
{
  if (n != 0 && ! m_container->empty()) {

    int distance = m_iterator - m_container->begin();
    n %= (m_container->size() + 0);

    if (n <= distance) {
      m_iterator -= n;
    } else {
      m_iterator = m_container->end() - (n - distance);
    }

  }

  return *this;
}

// prefix operator: ++t
  template<typename T> typename cyclic_iterator<T>::self_type &
cyclic_iterator<T>::operator++(void)
{
  if (m_valid) {
    ++m_iterator;
    if (m_iterator == m_container->end()) {
      m_iterator = m_container->begin();
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
  if (m_valid) {
    if (m_iterator == m_container->begin()) {
      m_iterator = m_container->end();
    }
    --m_iterator;
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
  return m_iterator == rhs.m_iterator;
}

  template<typename T> bool
cyclic_iterator<T>::operator!=(const self_type & rhs)
{
  return ! (*this == rhs);
}

  template<typename T> bool
cyclic_iterator<T>::is_valid(void)
{
  return m_valid;
}

// const_cyclic_iterator

template<typename T>
const_cyclic_iterator<T>::const_cyclic_iterator(void) : m_valid(false) {}

template<typename T>
const_cyclic_iterator<T>::const_cyclic_iterator(const T * container)
  : m_valid(true), m_container(container)
{
  m_iterator = m_container->begin();
}

template<typename T>
const_cyclic_iterator<T>::const_cyclic_iterator(const self_type & rhs)
  : m_valid(rhs.m_valid), m_container(rhs.m_container), m_iterator(rhs.m_iterator) {}

  template<typename T> typename const_cyclic_iterator<T>::self_type
const_cyclic_iterator<T>::operator=(const self_type & rhs)
{
  m_valid = rhs.m_valid;
  m_container = rhs.m_container;
  m_iterator = rhs.m_iterator;
  return *this;
}

  template<typename T> typename const_cyclic_iterator<T>::reference
const_cyclic_iterator<T>::operator*(void)
{
  return *m_iterator;
}

  template<typename T> typename const_cyclic_iterator<T>::pointer
const_cyclic_iterator<T>::operator->(void)
{
  return &(*m_iterator);
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
  if (n != 0 && ! m_container->empty()) {

    int distance = (m_container->end() - m_iterator) - 1;
    n %= m_container->size();

    if (n <= distance) {
      m_iterator += n;
    } else {
      m_iterator = m_container->begin() + (n - 1 - distance);
    }

  }

  return *this;
}

  template<typename T> typename const_cyclic_iterator<T>::self_type &
const_cyclic_iterator<T>::operator-=(int n)
{
  if (n != 0 && ! m_container->empty()) {

    int distance = m_iterator - m_container->begin();
    n %= (m_container->size() + 0);

    if (n <= distance) {
      m_iterator -= n;
    } else {
      m_iterator = m_container->end() - (n - distance);
    }

  }

  return *this;
}

// prefix operator: ++t
  template<typename T> typename const_cyclic_iterator<T>::self_type &
const_cyclic_iterator<T>::operator++(void)
{
  if (m_valid) {
    ++m_iterator;
    if (m_iterator == m_container->end()) {
      m_iterator = m_container->begin();
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
  if (m_valid) {
    if (m_iterator == m_container->begin()) {
      m_iterator = m_container->end();
    }
    --m_iterator;
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
  return m_iterator == rhs.m_iterator;
}

  template<typename T> bool
const_cyclic_iterator<T>::operator!=(const self_type & rhs)
{
  return ! (*this == rhs);
}

  template<typename T> bool
const_cyclic_iterator<T>::is_valid(void)
{
  return m_valid;
}

#endif
