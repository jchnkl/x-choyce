#include "x_xrm.hpp"

using namespace x;

xrm::xrm(Display * dpy,
         const std::string & name,
         const std::string & _class,
         const options & options)
  : m_dpy(dpy), m_name(name), m_class(_class), m_options(options)
{
  XrmInitialize();

  // The returned string is owned by Xlib and should not be freed by the client.
  char * db = XResourceManagerString(dpy);
  if (db != NULL) {
    m_database = XrmGetStringDatabase(db);
  }

  for (auto & item : m_options) {
    char * type;
    XrmValue value;
    std::string n = m_name + "." + item.first;
    std::string c = m_class + "." + item.first;

    if (XrmGetResource(m_database, n.c_str(), c.c_str(), &type, &value)) {
      switch (item.second.type) {
        case str:
          item.second.value.str = new std::string(value.addr, value.size);
          break;

        case num:
          item.second.value.num = std::stoi(std::string(value.addr, value.size));
          break;

        case dbl:
          item.second.value.dbl = std::stod(std::string(value.addr, value.size));
          break;

        default:
          break;
      }
    }

  }
}

xrm::~xrm(void)
{
  for (auto & item : m_options) {
    if (item.second.type == str && item.second.value.str != NULL) {
      delete item.second.value.str;
    }
  }
}
