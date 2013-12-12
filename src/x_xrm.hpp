#ifndef X_XRM_HPP
#define X_XRM_HPP

#include <unordered_map>
#include <unordered_set>
#include <X11/Xresource.h>

#include "config_t.hpp"
#include "observer.hpp"
#include "x_connection.hpp"

namespace x {

class xrm : public generic::config_t
          , public x_event_handler_t
{
  public:
    typedef std::unordered_map<std::string, option> options;

    xrm(x_connection & c,
        const std::string & name,
        const std::string & m_class,
        const options & options);

    ~xrm(void);

    const option &
      operator[](const std::string & name)
    {
      return m_options.at(name);
    }

    bool handle(xcb_generic_event_t *);

  private:
    x_connection & m_c;
    XrmDatabase m_database;
    std::string m_name;
    std::string m_class;
    options m_options;

    void update_db(void);
    void release_db(void);
    std::string resource_manager_string(void);

}; // class xrm

}; // namespace x

#endif // X_XRM_HPP
