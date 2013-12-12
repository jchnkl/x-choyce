#ifndef GETOPT_HPP
#define GETOPT_HPP

#include <algorithm> // find_if_not
#include <unordered_map>

#include <getopt.h>

#include "config_t.hpp"

static struct option g_options[] =
  { { "focusedalpha",   required_argument, 0, 0 }
  , { "focusedcolor",   required_argument, 0, 0 }
  , { "unfocusedalpha", required_argument, 0, 0 }
  , { "unfocusedcolor", required_argument, 0, 0 }
  , { "iconsize",       required_argument, 0, 0 }
  , { "borderwidth",    required_argument, 0, 0 }
  , { "titlefont",      required_argument, 0, 0 }
  , { "subtitlefont",   required_argument, 0, 0 }
  , { "titlefgcolor",   required_argument, 0, 0 }
  , { "titlebgalpha",   required_argument, 0, 0 }
  , { "titlebgcolor",   required_argument, 0, 0 }
  , { "north",          required_argument, 0, 0 }
  , { "south",          required_argument, 0, 0 }
  , { "east",           required_argument, 0, 0 }
  , { "west",           required_argument, 0, 0 }
  , { "raise",          required_argument, 0, 0 }
  , { "action",         required_argument, 0, 0 }
  , { "escape",         required_argument, 0, 0 }
  , { "mod",            required_argument, 0, 0 }
  , { "screen",         required_argument, 0, 0 }
  , { 0,                0,                 0, 0 }
  };

namespace generic {

class getopt : public config_t {
  public:
    getopt(int argc, char ** argv,
           const std::unordered_map<std::string, option> & options)
    {
      int i = 0;
      int c = 0;

      while (c >= 0) {
        c = getopt_long(argc, argv, "", g_options, &i);

        switch (c) {
          case 0:

            // extern * char optarg
            if (optarg == NULL) continue;

            try {
              // copy default option
              m_options[g_options[i].name] = options.at(g_options[i].name);
              // get reference to override default value
              auto & option = m_options[g_options[i].name];

              switch (option.type) {
                case str:
                  {
                  if (option.v.str != NULL) delete option.v.str;
                  std::string * s = new std::string(optarg);
                  s->erase(std::find_if_not(s->begin(), s->end(), ::isprint), s->end());
                  option.v.str = s;
                  break;
                  }

                case num:
                  option.v.num = std::stoi(std::string(optarg));
                  break;

                case dbl:
                  option.v.dbl = std::stod(std::string(optarg));
                  break;

                default:
                  break;
              }
            } catch (...) {}

            break;

          default:
            break;
        }
      }
    }

    const option &
      operator[](const std::string & name)
    {
      return m_options.at(name);
    }

  private:
    std::unordered_map<std::string, option> m_options;

}; // class getopt

}; // namespace generic

#endif // GETOPT_HPP
