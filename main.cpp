#include <X11/keysym.h>

#include "grid.hpp"
#include "x_connection.hpp"
#include "x_client_chooser.hpp"
#include "x_client_thumbnail_manager.hpp"

int main(int argc, char ** argv)
{
  xcb_keysym_t key = XK_Tab;
  xcb_mod_mask_t mod = XCB_MOD_MASK_4;

  x_connection c;

  grid_t grid;
  x_client_thumbnail_manager tm(c, &grid);

  x_client_chooser cp(c, &tm, key, mod);

  c.run_event_loop();

  return 0;
}
