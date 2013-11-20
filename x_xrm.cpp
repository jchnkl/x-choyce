#include "x_xrm.hpp"

using namespace x;

xrm::xrm(Display * dpy,
         const std::string & name,
         const std::string & _class,
         const options & options)
  : m_dpy(dpy), m_name(name), m_class(_class), m_options(options)
{
  XrmInitialize();
  update_db();
}


// private

void
xrm::update_db(void)
{
  std::string db = resource_manager_string();

  if (! db.empty()) {
    m_database = XrmGetStringDatabase(db.c_str());
  } else {
    return;
  }

  for (auto & item : m_options) {
    char * type;
    XrmValue value;
    std::string n = m_name + "." + item.first;
    std::string c = m_class + "." + item.first;

    if (XrmGetResource(m_database, n.c_str(), c.c_str(), &type, &value)) {
      switch (item.second.type) {
        case str:
          if (item.second.v.str != NULL) delete item.second.v.str;
          item.second.v.str = new std::string(value.addr, value.size);
          break;

        case num:
          item.second.v.num = std::stoi(std::string(value.addr, value.size));
          break;

        case dbl:
          item.second.v.dbl = std::stod(std::string(value.addr, value.size));
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
    if (item.second.type == str && item.second.v.str != NULL) {
      delete item.second.v.str;
    }
  }
}
