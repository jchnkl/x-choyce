#ifndef DATA_TYPES
#define DATA_TYPES

#include <iostream>

struct dimension {
  dimension(void);
  dimension(unsigned int w, unsigned int h);
  unsigned int width, height;
};

struct position {
  position(void);
  position(int x, int y);
  int x, y;
};

struct rectangle {
  rectangle(void);
  rectangle(position pos, dimension dim);
  rectangle(int x, int y, unsigned int width, unsigned int height);

  int & x(void);
  int const & x(void) const;
  int & y(void);
  int const & y(void) const;
  unsigned int & width(void);
  unsigned int const & width(void) const;
  unsigned int & height(void);
  unsigned int const & height(void) const;

  position m_position;
  dimension m_dimension;
};

std::ostream & operator<<(std::ostream & os, const rectangle & rect);

namespace x {

namespace type {

namespace generic {

class name {
  public:
    name(void) {}
    name(const std::string & name) : m_name(name) {}
    std::string & operator*(void) { return m_name; }
    const std::string & operator*(void) const { return m_name; }
    std::string * const operator->(void) { return &m_name; }
    const std::string * const operator->(void) const { return &m_name; }
  private:
    std::string m_name;
}; // class name

}; // namespace generic

class colorname : public generic::name {
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 8)
  using generic::name::name;
#else
  public:
    colorname(void) {}
    colorname(const std::string & name) : generic::name(name) {}
#endif
};

class fontname : public generic::name {
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 8)
  using generic::name::name;
#else
  public:
    fontname(void) {}
    fontname(const std::string & name) : generic::name(name) {}
#endif
};

}; // namespace type

}; // namespace x

#endif
