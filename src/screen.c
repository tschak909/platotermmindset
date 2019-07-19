#include "screen.h"
#include "protocol.h"
#include "biosgfx.h"
#include "font.h"
#include "scale.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <i86.h>

unsigned char CharWide=8;
unsigned char CharHigh=16;
unsigned char screen_mode;
extern padBool FastText; /* protocol.c */
padPt TTYLoc;
uint8_t FONT_SIZE_X;
uint8_t FONT_SIZE_Y;
uint16_t* scalex;
uint16_t* scaley;
uint8_t* font;
uint16_t* fontptr;
uint16_t width;
uint16_t height;
unsigned char current_foreground=WHITE;
unsigned char current_background=BLACK;
unsigned char fontm23[768];
unsigned char is_mono=0;

/**
 * screen_init() - Set up the screen
 */
void screen_init(void)
{
  screen_mode=2;
  width=320;
  height=200;
  FONT_SIZE_X=5;
  FONT_SIZE_Y=6;
  font=&font_320x200;
  scalex=&scalex_320;
  scaley=&scaley_200;
  fontptr=&fontptr_6;
  mindset_init();
  mindset_mode(screen_mode);
}

/**
 * screen_beep(void) - Beep the terminal
 */
void screen_beep(void)
{
}

/**
 * screen_clear - Clear the screen
 */
void screen_clear(void)
{
  mindset_clear_screen();
}

/**
 * screen_wait(void) - Sleep for approx 16.67ms
 */
void screen_wait(void)
{
}

/**
 * screen_block_draw(Coord1, Coord2) - Perform a block fill from Coord1 to Coord2
 */
void screen_block_draw(padPt* Coord1, padPt* Coord2)
{
  // initial naive implementation, draw a bunch of horizontal lines the size of bounding box.
  if (CurMode==ModeErase || CurMode==ModeInverse)
    current_foreground=current_background;
  else
    current_foreground=current_foreground;

  mindset_bar(scalex[Coord1->x],scaley[Coord1->y],scalex[Coord2->x],scaley[Coord2->y],current_foreground);
}

/**
 * screen_dot_draw(Coord) - Plot a mode 0 pixel
 */
void screen_dot_draw(padPt* Coord)
{
  if (CurMode==ModeErase || CurMode==ModeInverse)
    current_foreground=current_background;
  else
    current_foreground=current_foreground;
  mindset_dot(scalex[Coord->x],scaley[Coord->y],current_foreground);
}

/**
 * screen_line_draw(Coord1, Coord2) - Draw a mode 1 line
 */
void screen_line_draw(padPt* Coord1, padPt* Coord2)
{
  if (CurMode==ModeErase || CurMode==ModeInverse)
    current_foreground=current_background;
  else
    current_foreground=current_foreground;
  mindset_line(scalex[Coord1->x],scaley[Coord1->y],scalex[Coord2->x],scaley[Coord2->y],current_foreground);
}

/**
 * screen_char_draw(Coord, ch, count) - Output buffer from ch* of length count as PLATO characters
 */
void screen_char_draw(padPt* Coord, unsigned char* ch, unsigned char count)
{
  mindset_text(scalex[Coord->x],scaley[Coord->y],ch,count);
}

/**
 * screen_tty_char - Called to plot chars when in tty mode
 */
void screen_tty_char(padByte theChar)
{
  if ((theChar >= 0x20) && (theChar < 0x7F)) {
    screen_char_draw(&TTYLoc, &theChar, 1);
    TTYLoc.x += CharWide;
  }
  else if ((theChar == 0x0b)) /* Vertical Tab */
    {
      TTYLoc.y += CharHigh;
    }
  else if ((theChar == 0x08) && (TTYLoc.x > 7))	/* backspace */
    {
      TTYLoc.x -= CharWide;
      // screen_block_draw(&scalex[TTYLoc.x],&scaley[TTYLoc.y],&scalex[TTYLoc.x+CharWide],&scaley[TTYLoc.y+CharHigh]);
    }
  else if (theChar == 0x0A)			/* line feed */
    TTYLoc.y -= CharHigh;
  else if (theChar == 0x0D)			/* carriage return */
    TTYLoc.x = 0;
  
  if (TTYLoc.x + CharWide > 511) {	/* wrap at right side */
    TTYLoc.x = 0;
    TTYLoc.y -= CharHigh;
  }
  
  if (TTYLoc.y < 0) {
    screen_clear();
    TTYLoc.y=495;
  }
}

/**
 * screen_color - return closest match to requested color.
 */
unsigned char screen_color(padRGB* theColor)
{
  return 0;
/*   unsigned char newRed; */
/*   unsigned char newGreen; */
/*   unsigned char newBlue; */

/*   newRed=(theColor->red*0.008); */
/*   newGreen=(theColor->green*0.008); */
/*   newBlue=(theColor->blue*0.008); */

/*   if (is_mono==1) */
/*     { */
/*       if ((theColor->red==0) && (theColor->green==0) && (theColor->blue==0)) */
/* 	{ */
/* 	  return 0; */
/* 	} */
/*       else */
/*         { */
/* #ifdef SANYO_MBC550 */
/* 	  return 2; // emulating CGA I guess... */
/* #else */
/* 	  return 1; */
/* #endif */
/* 	} */
/*     } */
/*   else */
/*     { */
/*       if ((newRed==0) && (newGreen==0) && (newBlue==0)) */
/* 	return BLACK; */
/*       else if ((newRed==1) && (newGreen==1) && (newBlue==1)) */
/* 	return LIGHTGREY; */
/*       else if ((newRed==0) && (newGreen==0) && (newBlue==1)) */
/* 	return BLUE; */
/*       else if ((newRed==0) && (newGreen==0) && (newBlue==2)) */
/* 	return LIGHTBLUE; */
/*       else if ((newRed==0) && (newGreen==1) && (newBlue==0)) */
/* 	return GREEN; */
/*       else if ((newRed==0) && (newGreen==2) && (newBlue==0)) */
/* 	return LIGHTGREEN; */
/*       else if ((newRed==0) && (newGreen==1) && (newBlue==1)) */
/* 	return CYAN; */
/*       else if ((newRed==0) && (newGreen==2) && (newBlue==2)) */
/* 	return LIGHTCYAN; */
/*       else if ((newRed==1) && (newGreen==0) && (newBlue==0)) */
/* 	return RED; */
/*       else if ((newRed==2) && (newGreen==0) && (newBlue==0)) */
/* 	return LIGHTRED; */
/*       else if ((newRed==1) && (newGreen==0) && (newBlue==1)) */
/* 	return MAGENTA; */
/*       else if ((newRed==2) && (newGreen==0) && (newBlue==2)) */
/* 	return LIGHTMAGENTA; */
/*       else if ((newRed==1) && (newGreen==1) && (newBlue==0)) */
/* 	return BROWN; */
/*       else if ((newRed==2) && (newGreen==2) && (newBlue==0)) */
/* 	return YELLOW; */
/*       else if ((newRed==2) && (newGreen==2) && (newBlue==2)) */
/* 	return WHITE; */
/*     } */
/*   return WHITE; */
}

/**
 * screen_foreground - Called to set foreground color.
 */
void screen_foreground(padRGB* theColor)
{
  /* current_foreground=screen_color(theColor); */
}

/**
 * screen_background - Called to set foreground color.
 */
void screen_background(padRGB* theColor)
{
  /* current_background=screen_color(theColor); */
}

/**
 * screen_paint - Paint screen scanline_fill
 */
void _screen_paint(unsigned short x, unsigned short y)
{
}

/**
 * screen_paint - Called to paint at location.
 */
void screen_paint(padPt* Coord)
{
}

/**
 * touch_allow - Set whether touchpanel is active or not.
 */
void touch_allow(padBool allow)
{
}
