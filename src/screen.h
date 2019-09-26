#ifndef SCREEN_H
#define SCREEN_H

#include "protocol.h"

/**
 * screen_init() - Set up the screen
 */
void screen_init(void);


/**
 * Screen set font pointer for BLT STRING calls
 * depending on graphics mode.
 */
void screen_set_font(unsigned char mode);

/**
 * screen_beep(void) - Beep the terminal
 */
void screen_beep(void);

/**
 * screen_clear - Clear the screen
 */
void screen_clear(void);

/**
 * Screen Pen color
 */
unsigned char screen_pen_color(void);

/**
 * screen_wait(void) - Sleep for approx 16.67ms
 */
void screen_wait(void);

/**
 * screen_block_draw(Coord1, Coord2) - Perform a block fill from Coord1 to Coord2
 */
void screen_block_draw(padPt* Coord1, padPt* Coord2);

/**
 * screen_dot_draw(Coord) - Plot a mode 0 pixel
 */
void screen_dot_draw(padPt* Coord);

/**
 * screen_line_draw(Coord1, Coord2) - Draw a mode 1 line
 */
void screen_line_draw(padPt* Coord1, padPt* Coord2);

/**
 * screen_char_draw(Coord, ch, count) - Output buffer from ch* of length count as PLATO characters
 */
void screen_char_draw(padPt* Coord, unsigned char* ch, unsigned char count);

/**
 * screen_tty_char - Called to plot chars when in tty mode
 */
void screen_tty_char(padByte theChar);

/**
 * screen_color - return closest match to requested color.
 */
unsigned char screen_color(padRGB* theColor);

/**
 * screen_monitor_color() - Quantize 8-bit RGB color component to RGBI color component.
 * c = color component, b = bit mask to set.
 *
 * If color value is greater than 128, then intensity bit is set.
 *
 */
unsigned short screen_monitor_color(unsigned char c, unsigned short b);

/**
 * screen_foreground - Called to set foreground color.
 */
void screen_foreground(padRGB* theColor);

/**
 * screen_background - Called to set foreground color.
 */
void screen_background(padRGB* theColor);

/**
 * screen_paint - Called to paint at location.
 */
void screen_paint(padPt* Coord);

/**
 * touch_allow - Set whether touchpanel is active or not.
 */
void touch_allow(padBool allow);

#endif
