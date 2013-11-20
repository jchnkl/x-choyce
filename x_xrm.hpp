#ifndef X_XRM_HPP
#define X_XRM_HPP

#include <unordered_map>
#include <unordered_set>
#include <X11/Xresource.h>

namespace x {

class xrm {
  public:
    struct option;
    typedef std::unordered_map<std::string, option> options;

    enum { str, num, dbl };

    union value_t {
      int num;
      double dbl;
      std::string * str;
    };

    struct option {
      int type;
      value_t value;
    };

    xrm(Display * dpy, const std::string & name,
                       const std::string & _class,
                       const options & options);

    ~xrm(void);

    const option & operator[](const std::string & name)
    {
      return m_options.at(name);
    }

  private:
    Display * m_dpy;
    XrmDatabase m_database;
    std::string m_name;
    std::string m_class;
    options m_options;
};

}; // namespace x

#endif // X_XRM_HPP
