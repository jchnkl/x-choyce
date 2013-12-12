#ifndef CONFIG_T_HPP
#define CONFIG_T_HPP

#include <string>

#include "observer.hpp"

namespace generic {

class config_t : public observable<config_t>
{
  public:
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

    virtual const option &
      operator[](const std::string & name) = 0;

}; // class config_t

}; // namespace generic

#endif // CONFIG_T_HPP
