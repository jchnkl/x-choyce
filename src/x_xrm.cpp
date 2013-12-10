#include "x_xrm.hpp"

#include <climits>
#include <algorithm>

using namespace x;

xrm::xrm(x_connection & c,
         const std::string & name,
         const std::string & m_class,
         const options & options)
  : m_c(c), m_name(name), m_class(m_class), m_options(options)
{
  m_c.attach(0, XCB_PROPERTY_NOTIFY, this);
  XrmInitialize();
  update_db();
}

xrm::~xrm(void)
{
  m_c.detach(XCB_PROPERTY_NOTIFY, this);
  release_db();
}

bool
xrm::handle(xcb_generic_event_t * ge)
{
  if (XCB_PROPERTY_NOTIFY == (ge->response_type & ~0x80)) {
    xcb_property_notify_event_t * e = (xcb_property_notify_event_t *)ge;

    if (e->atom == m_c.intern_atom("RESOURCE_MANAGER")) {
      update_db();
      observable::notify();
    }

    return true;
  }

  return false;
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
          {
          if (item.second.v.str != NULL) delete item.second.v.str;
          std::string * s = new std::string(value.addr, value.size);
          s->erase(std::find_if_not(s->begin(), s->end(), ::isprint), s->end());
          item.second.v.str = s;
          break;
          }

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

void
xrm::release_db(void)
{
  XrmDestroyDatabase(m_database);
  for (auto & item : m_options) {
    if (item.second.type == str && item.second.v.str != NULL) {
      delete item.second.v.str;
    }
  }
}

std::string
xrm::resource_manager_string(void)
{
  xcb_atom_t atom = m_c.intern_atom("RESOURCE_MANAGER");

  xcb_get_property_cookie_t c = xcb_get_property(
      m_c(), false, m_c.root_window(), atom, XCB_ATOM_STRING, 0, UINT_MAX);

  xcb_generic_error_t * error = NULL;
  xcb_get_property_reply_t * r = xcb_get_property_reply(m_c(), c, &error);

  std::string result;

  if (error) {
    delete error;

  } else {
    result = std::string((char *)xcb_get_property_value(r),
                         xcb_get_property_value_length(r));
  }

  if (r) delete r;
  return result;
}
