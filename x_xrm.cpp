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
      if (item.second.type == string) {
        std::string result(value.addr, value.size);
        item.second.value.string = new char[result.length()];
        result.copy(item.second.value.string, result.length());
        m_dynamic.insert(item.first);
      } else if (item.second.type == number) {
        item.second.value.number = std::stoi(std::string(value.addr, value.size));
      }
    }

  }
}

xrm::~xrm(void)
{
  for (auto & item : m_options) {
    if (m_dynamic.find(item.first) != m_dynamic.end()
        && item.second.type == string
        && item.second.value.string != NULL) {
      delete [] item.second.value.string;
    }
  }
}
