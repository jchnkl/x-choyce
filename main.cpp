#include <signal.h>
#include <X11/keysym.h>

#include "x_xrm.hpp"
#include "grid.hpp"
#include "thumbnail_manager.hpp"
#include "x_connection.hpp"
#include "x_client_chooser.hpp"
#include "x_client_thumbnail_gl.hpp"

x_event_source_t * g_event_source = NULL;

void sig_handler(int signum)
{
  if (g_event_source) g_event_source->shutdown();
}

int main(int argc, char ** argv)
{
  xcb_keysym_t east_key = XK_l;
  xcb_keysym_t west_key = XK_h;
  xcb_keysym_t north_key = XK_k;
  xcb_keysym_t south_key = XK_j;
  xcb_keysym_t quit_key = XK_Escape;
  xcb_keysym_t action_key = XK_Tab;
  xcb_mod_mask_t mod = XCB_MOD_MASK_4;

  x_connection c;
  g_event_source = &c;

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
    // borderwidth
    , { .type = x::xrm::num, .v = { .num =  4                                } }
    // iconsize
    , { .type = x::xrm::num, .v = { .num = 64                                } }
    // titlefont
    , { .type = x::xrm::str,
      .v = { .str = new std::string("Sans:bold:pixelsize=26:antialias=true") } }
    // subtitlefont
    , { .type = x::xrm::str,
      .v = { .str = new std::string("Sans:bold:pixelsize=16:antialias=true") } }
    // titlefgcolor
    , { .type = x::xrm::str, .v = { .str = new std::string("#303030")        } }
    // titlebgalpha
    , { .type = x::xrm::dbl, .v = { .dbl = 0.375                             } }
    // titlebgcolor
    , { .type = x::xrm::str, .v = { .str = new std::string("#484848")        } }
    };

  int o = 0;
  x::xrm xrm(c.dpy(), "xchoyce", "XChoyce",
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
      });

  grid_t grid;

  x_client_thumbnail::factory factory(c, xrm);

  thumbnail_manager tm(c, &grid, &factory);
  x_client_chooser cp(c, &tm,
                      east_key, west_key, north_key, south_key,
                      quit_key, action_key, mod);

  c.run_event_loop();

  return 0;
}
