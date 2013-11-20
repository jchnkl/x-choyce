#ifndef X_XRM_HPP
#define X_XRM_HPP

#include <unordered_map>
#include <unordered_set>
#include <X11/Xresource.h>

#include "observer.hpp"
#include "x_connection.hpp"

namespace x {

class xrm : public observable<xrm> {
{
  public:
    struct option;
    typedef std::unordered_map<std::string, option> options;

    enum { str, num, dbl };

    union value {
      int num;
      double dbl;
      std::string * str;
    };

    struct option {
      int type;
      value v;
    };

    xrm(x_connection & c,
        const std::string & name,
        const std::string & _class,
        const options & options);

    ~xrm(void);

    const option & operator[](const std::string & name)
    {
      return m_options.at(name);
    }

  private:
    x_connection & m_c;
    XrmDatabase m_database;
    std::string m_name;
    std::string m_class;
    options m_options;

    void update_db(void);
    void release_db(void);
    std::string resource_manager_string(void);
};

}; // namespace x

#endif // X_XRM_HPP
