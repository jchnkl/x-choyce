#include <X11/keysym.h>

#include "grid.hpp"
#include "thumbnail_manager.hpp"
#include "x_connection.hpp"
#include "x_client_chooser.hpp"
#include "x_client_thumbnail_gl.hpp"

int main(int argc, char ** argv)
{
  xcb_keysym_t quit_key = XK_Escape;
  xcb_keysym_t action_key = XK_Tab;
  xcb_mod_mask_t mod = XCB_MOD_MASK_1;

  x_connection c;

  grid_t grid;

  x_client_thumbnail::factory factory;

  thumbnail_manager tm(c, &grid, &factory);
  x_client_chooser cp(c, &tm, quit_key, action_key, mod);

  c.run_event_loop();

  return 0;
}
