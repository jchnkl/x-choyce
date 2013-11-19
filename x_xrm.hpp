#ifndef X_XRM_HPP
#define X_XRM_HPP

#include <unordered_map>
#include <unordered_set>
#include <X11/Xresource.h>

namespace x {

class xrm {
  public:

    enum { string, number };

    union value_t {
      int number;
      char * string;
    };

    struct option {
      int type;
      value_t value;
    };

    typedef std::unordered_map<std::string, option> options;

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
    std::unordered_set<std::string> m_dynamic;
};

}; // namespace x

#endif // X_XRM_HPP
