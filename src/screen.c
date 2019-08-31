#include "screen.h"
#include "protocol.h"
#include "font.h"
#include "scale.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <i86.h>
#include "../mindset-c/libmindset_gfx/libmindset_gfx.h"

/* Quickly get a solid fill pattern for a color */
const unsigned short color_to_solid_fill_pattern[]=
  {
   0x0000,
   0x1111,
   0x2222,
   0x3333,
   0x4444,
   0x5555,
   0x6666,
   0x7777,
   0x8888,
   0x9999,
   0xAAAA,
   0xBBBB,
   0xCCCC,
   0xDDDD,
   0xEEEE,
   0xFFFF
  };

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
unsigned char current_foreground=0;
unsigned char current_background=1;
padRGB current_background_rgb={0,0,0};
padRGB current_foreground_rgb={255,255,255};
unsigned char fontm23[768];
unsigned char is_mono=0;
unsigned char highest_color_index;
unsigned short palette[16];
unsigned char maxcolors=16;

/* Palette bits for Monitor RGBI colors */
#define MONITOR_I 0x8000
#define MONITOR_R 0x4000
#define MONITOR_G 0x2000
#define MONITOR_B 0x1000

/**
 * screen_init() - Set up the screen
 */
void screen_init(void)
{
  switch (screen_mode)
    {
    case 2:
      width=320;
      height=200;
      /* FONT_SIZE_X=5; */
      /* FONT_SIZE_Y=6; */
      /* font=&font_320x200; */
      /* fontptr=&fontptr_6; */
      scalex=&scalex_320;
      scaley=&scaley_200;
      maxcolors=16;
      break;
    case 4:
      width=640;
      height=200;
      /* FONT_SIZE_X=8; */
      /* FONT_SIZE_Y=6; */
      /* font=&font_640x200; */
      /* fontptr=&fontptr_6; */
      scalex=&scalex_640;
      scaley=&scaley_200;
      maxcolors=4;
      break;
    case 6:
      width=640;
      height=400;
      /* FONT_SIZE_X=8; */
      /* FONT_SIZE_Y=12; */
      /* font=&font_640x400; */
      /* fontptr=&fontptr_12; */
      scalex=&scalex_640;
      scaley=&scaley_400;
      maxcolors=2;
      break;
    }
  mindset_gfx_set_mode(screen_mode);
} 

/**
 * screen_beep(void) - Beep the terminal
 */
void screen_beep(void)
{
}

/**
 * screen_pen_mode() - Set pen mode
 */
void screen_pen_mode(void)
{
  unsigned char t;
  if (CurMode==ModeErase || CurMode==ModeInverse)
    {
      t=current_foreground;
      current_foreground=current_background;
      current_background=t;
    }
  else
    {
      t=current_background;
      current_foreground=current_foreground;
      current_background=t;
    }
}

/**
 * Dump palette to screen
 */
void screen_dump_palette(void)
{
  CopyWordParams p[16];
  int i;
  
  // Set color patterns for each palette entry.
  p[0].pattern=0x0000;
  p[1].pattern=0x1111;
  p[2].pattern=0x2222;
  p[3].pattern=0x3333;
  p[4].pattern=0x4444;
  p[5].pattern=0x5555;
  p[6].pattern=0x6666;
  p[7].pattern=0x7777;
  p[8].pattern=0x8888;
  p[9].pattern=0x9999;
  p[10].pattern=0xAAAA;
  p[11].pattern=0xBBBB;
  p[12].pattern=0xCCCC;
  p[13].pattern=0xDDDD;
  p[14].pattern=0xEEEE;
  p[15].pattern=0xFFFF;

  for (i=0;i<16;i++)
    {
      p[i].x=i*8;
      p[i].y=192;
      p[i].width=8;
      p[i].height=8;
    }

  mindset_gfx_blt_copy_word(0,16,0,0,&p);  
}

/**
 * Update palette
 */
void screen_update_palette(void)
{
  mindset_gfx_set_palette(0,16,0,&palette);
  screen_dump_palette();
}

/**
 * screen_clear - Clear the screen
 */
void screen_clear(void)
{
  highest_color_index=0;
  palette[0]=palette[current_background];

  if (palette[0]!=palette[current_foreground])
    {
      palette[1]=palette[current_foreground];
      highest_color_index++;
    }

  if ((palette[0]==0x0000) && (palette[1]==0x0000))
    {
      palette[0]=0x0000;
      palette[1]=0xF1FF;
    }
  
  mindset_gfx_fill_dest_buffer(0x0000);
  screen_update_palette();
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
  CopyWordParams params;
  
  screen_pen_mode();

  params.pattern=color_to_solid_fill_pattern[current_foreground];
  params.x=scalex[Coord1->x];
  params.y=scaley[Coord1->y];
  params.width=scalex[(Coord2->x)-(Coord1->x)];
  params.height=scaley[(Coord2->y)-(Coord1->y)];

  mindset_gfx_blt_copy_word(0,1,0,0,&params);
}

/**
 * screen_dot_draw(Coord) - Plot a mode 0 pixel
 */
void screen_dot_draw(padPt* Coord)
{
  PolyPointParams params;

  params.x=scalex[Coord->x];
  params.y=scaley[Coord->y];

  screen_pen_mode();
  mindset_gfx_blt_polypoint(0,0,current_foreground,0,0,&params);
}

/**
 * screen_line_draw(Coord1, Coord2) - Draw a mode 1 line
 */
void screen_line_draw(padPt* Coord1, padPt* Coord2)
{
  PolyLineParams params[2];

  params[0].x=scalex[Coord1->x];
  params[0].y=scaley[Coord1->y];
  params[1].x=scalex[Coord2->x];
  params[1].y=scaley[Coord2->y];

  screen_pen_mode();
  mindset_gfx_blt_polyline(0,2,current_foreground,1,0,0,&params);
}

/**
 * screen_char_draw(Coord, ch, count) - Output buffer from ch* of length count as PLATO characters
 */
void screen_char_draw(padPt* Coord, unsigned char* ch, unsigned char count)
{
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
 * screen_monitor_color() - Quantize 8-bit RGB color component to RGBI color component.
 * c = color component, b = bit mask to set.
 *
 * If color value is greater than 128, then intensity bit is set.
 *
 */
unsigned short screen_monitor_color(unsigned char c, unsigned short b)
{
  unsigned short ret=0;

  // Quantize color bit.
  c>>=6; // (1 = 64, 2 = 128, we don't care about 192.)
    
  if (c>1)
    ret|=0x8000; // Set intensity bit.

  if (c>0)
    ret|=b; // Set color bit.

  return ret;
}

/**
 * screen_color - return closest match to requested color.
 */
unsigned char screen_color(padRGB* theColor)
{
  unsigned short newTVRed=theColor->red >> 5;
  unsigned short newTVGreen=(theColor->green >> 5) << 3;
  unsigned short newTVBlue=(theColor->blue >> 5) << 6;
  unsigned short newMonitorRed=screen_monitor_color(theColor->red, MONITOR_R);
  unsigned short newMonitorGreen=screen_monitor_color(theColor->green, MONITOR_G);
  unsigned short newMonitorBlue=screen_monitor_color(theColor->blue, MONITOR_B);
  unsigned char i; 

  for (i=0;i<16;i++)
    {
      if (i>highest_color_index)
	{
	  if (highest_color_index<maxcolors)
	    {
	      palette[i]=newMonitorRed|newMonitorGreen|newMonitorBlue|newTVRed|newTVGreen|newTVBlue;
	      highest_color_index++;
	      return i;
	    }
	  else
	    {
	      return palette[maxcolors-1];
	    }
	}
      else if (palette[i]==(newMonitorRed|newMonitorGreen|newMonitorBlue|newTVRed|newTVGreen|newTVBlue))
	return i;
    }
  return 0;
}

/**
 * screen_foreground - Called to set foreground color.
 */
void screen_foreground(padRGB* theColor)
{
  current_foreground=screen_color(theColor);
  screen_update_palette();
}

/**
 * screen_background - Called to set foreground color.
 */
void screen_background(padRGB* theColor)
{
  current_background=screen_color(theColor);
  screen_update_palette();
}

unsigned char GetPixel(unsigned short x, unsigned short y)
{
  union REGS regs;
  regs.h.ah=0x0d;
  regs.w.cx=x;
  regs.w.dx=y;
  int86(0x10,&regs,&regs);
  return regs.h.al;
}

void Line(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned char c)
{
  union REGPACK regs;
  unsigned short l[4];

  l[0]=x1;
  l[1]=y1;
  l[2]=x2;
  l[3]=y2;
  
  regs.h.ah=0x0d;
  regs.h.al=1;
  regs.w.cx=2;
  regs.h.dh=c;
  regs.h.dl=1;
  regs.w.si=0;
  regs.w.di=0;
  regs.w.es=FP_SEG(l);
  regs.w.bx=FP_OFF(l);
  intr(0xEF,&regs);
}

/**
 * screen_paint - Called to paint at location.
 */
void screen_paint(padPt* Coord)
{
  static unsigned short xStack[640];
  static unsigned short yStack[400];
  unsigned char stackentry = 1;
  unsigned char spanAbove, spanBelow;
  unsigned short x=scalex[Coord->x];
  unsigned short y=scaley[Coord->y];
  unsigned char oldColor = GetPixel(x,y);

  if (oldColor == current_foreground)
    return;

  do
    {
      unsigned short startx;
      while (x > 0 && GetPixel(x-1,y) == oldColor)
        --x;

      spanAbove = spanBelow = false;
      startx=x;

      while(GetPixel(x,y) == oldColor)
        {
          if (y < (199))
            {
              unsigned char belowColor = GetPixel(x, y+1);
              if (!spanBelow  && belowColor == oldColor)
                {
                  xStack[stackentry]  = x;
                  yStack[stackentry]  = y+1;
                  ++stackentry;
                  spanBelow = true;
                }
              else if (spanBelow && belowColor != oldColor)
                spanBelow = false;
            }

          if (y > 0)
            {
              unsigned char aboveColor = GetPixel(x, y-1);
              if (!spanAbove  && aboveColor == oldColor)
                {
                  xStack[stackentry]  = x;
                  yStack[stackentry]  = y-1;
                  ++stackentry;
                  spanAbove = true;
                }
              else if (spanAbove && aboveColor != oldColor)
                spanAbove = false;
            }

          ++x;
        }
      Line(startx,y,x-1,y,current_foreground);
      --stackentry;
      x = xStack[stackentry];
      y = yStack[stackentry];
    }
  while (stackentry);
}

/**
 * touch_allow - Set whether touchpanel is active or not.
 */
void touch_allow(padBool allow)
{
}
