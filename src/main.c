#include <stdbool.h>
#include <string.h>
#include "screen.h"
#include "io.h"
#include "terminal.h"
#include "splash.h"
#include "keyboard.h"
#include "log.h"

extern unsigned char screen_mode;

void main(int argc, char* argv[])
{
  if (!strcasecmp(argv[1],"/320x200"))
    screen_mode=2;
  else if (!strcasecmp(argv[1],"/640x200"))
    screen_mode=4;
  else if (!strcasecmp(argv[1],"/640x400"))
    screen_mode=6;
  else
    screen_mode=2; // 320x200
  
  screen_init();
  io_init();
  terminal_init();
  ShowPLATO(splash,sizeof(splash));
  terminal_initial_position();
  for (;;)
    {
      io_main();
      keyboard_main();
    }
}
