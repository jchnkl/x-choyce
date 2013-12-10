#include <signal.h>
#include <X11/keysym.h>

#include "x_xrm.hpp"
#include "grid.hpp"
#include "thumbnail_manager.hpp"
#include "x_connection.hpp"
#include "x_event_source.hpp"
#include "x_ewmh.hpp"
#include "x_client_chooser.hpp"
#include "x_client_thumbnail_gl.hpp"

x_event_source_t * g_event_source = NULL;

void sig_handler(int signum)
{
  if (g_event_source) g_event_source->shutdown();
}

int main(int argc, char ** argv)
{
  x_connection c;
  x_event_source event_source(c);
  c.set(&event_source);
  g_event_source = &c;

  x_ewmh ewmh(c);
  c.set(&ewmh);

  signal(SIGINT,  sig_handler);
  signal(SIGTERM, sig_handler);

  x::xrm::option options[] =
    // focusedalpha
    { { .type = x::xrm::dbl, .v = { .dbl = 0.75                              } }
    // focusedcolor: goldenrod
    , { .type = x::xrm::str, .v = { .str = new std::string("#daa520")        } }
    // unfocusedalpha
    , { .type = x::xrm::dbl, .v = { .dbl = 0.5                               } }
    // unfocusedcolor
    , { .type = x::xrm::str, .v = { .str = new std::string("#404040")        } }
    // iconsize
    , { .type = x::xrm::num, .v = { .num = 64                                } }
    // borderwidth
    , { .type = x::xrm::num, .v = { .num =  4                                } }
    // titlefont
    , { .type = x::xrm::str,
      .v = { .str = new std::string("Sans:bold:pixelsize=26:antialias=true") } }
    // subtitlefont
    , { .type = x::xrm::str,
      .v = { .str = new std::string("Sans:bold:pixelsize=16:antialias=true") } }
    // titlefgcolor
    , { .type = x::xrm::str, .v = { .str = new std::string("#616161")        } }
    // titlebgalpha
    , { .type = x::xrm::dbl, .v = { .dbl = 0.25                              } }
    // titlebgcolor
    , { .type = x::xrm::str, .v = { .str = new std::string("#292929")        } }
    // north
    , { .type = x::xrm::str, .v = { .str = new std::string("k")              } }
    // south
    , { .type = x::xrm::str, .v = { .str = new std::string("j")              } }
    // east
    , { .type = x::xrm::str, .v = { .str = new std::string("l")              } }
    // west
    , { .type = x::xrm::str, .v = { .str = new std::string("h")              } }
    // raise
    , { .type = x::xrm::str, .v = { .str = new std::string("space")          } }
    // action
    , { .type = x::xrm::str, .v = { .str = new std::string("Tab")            } }
    // escape
    , { .type = x::xrm::str, .v = { .str = new std::string("Escape")         } }
    // mod
    , { .type = x::xrm::str, .v = { .str = new std::string("mod4")           } }
    // screen
    , { .type = x::xrm::str, .v = { .str = new std::string("pointer")        } }
    };

  int o = 0;
  x::xrm xrm(c, "xchoyce", "XChoyce",
      { { "focusedalpha",   options[o++] }
      , { "focusedcolor",   options[o++] }
      , { "unfocusedalpha", options[o++] }
      , { "unfocusedcolor", options[o++] }
      , { "iconsize",       options[o++] }
      , { "borderwidth",    options[o++] }
      , { "titlefont",      options[o++] }
      , { "subtitlefont",   options[o++] }
      , { "titlefgcolor",   options[o++] }
      , { "titlebgalpha",   options[o++] }
      , { "titlebgcolor",   options[o++] }
      , { "north",          options[o++] }
      , { "south",          options[o++] }
      , { "east",           options[o++] }
      , { "west",           options[o++] }
      , { "raise",          options[o++] }
      , { "action",         options[o++] }
      , { "escape",         options[o++] }
      , { "mod",            options[o++] }
      , { "screen",         options[o++] }
      });

  grid_t grid;

  x_client_thumbnail::factory factory(c, xrm);

  thumbnail_manager tm(c, xrm, &grid, &factory);
  x_client_chooser cp(c, xrm, &tm);

  c.run_event_loop();

  return 0;
}
