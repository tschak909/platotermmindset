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
      FONT_SIZE_X=5;
      FONT_SIZE_Y=6;
      font=&font_320x200;
      fontptr=&fontptr_6;
      scalex=&scalex_320;
      scaley=&scaley_200;
      break;
    case 4:
      width=640;
      height=200;
      FONT_SIZE_X=8;
      FONT_SIZE_Y=6;
      font=&font_640x200;
      fontptr=&fontptr_6;
      scalex=&scalex_640;
      scaley=&scaley_200;
      break;
    case 6:
      width=640;
      height=400;
      FONT_SIZE_X=8;
      FONT_SIZE_Y=12;
      font=&font_640x400;
      fontptr=&fontptr_12;
      scalex=&scalex_640;
      scaley=&scaley_400;      
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
  if (CurMode==ModeErase || CurMode==ModeInverse)
    current_foreground=current_background;
  else
    current_foreground=current_foreground;
}

/**
 * Update palette
 */
void screen_update_palette(void)
{
  mindset_gfx_set_palette(0,16,0,&palette);
}

/**
 * screen_clear - Clear the screen
 */
void screen_clear(void)
{
  highest_color_index=1;
  mindset_gfx_fill_dest_buffer(color_to_solid_fill_pattern[current_background]);
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

  mindset_gfx_blt_polyline(0,2,current_foreground,1,0,0,&params);
}

/**
 * screen_char_draw(Coord, ch, count) - Output buffer from ch* of length count as PLATO characters
 */
void screen_char_draw(padPt* Coord, unsigned char* ch, unsigned char count)
{
  PolyPointParams pp_mainColor[192]; // max size of 8x12 font for 96 total points to blit max. x2 for bold.
  unsigned char ppc_mainColor=0; // polypoint counter for above.
  PolyPointParams pp_altColor[192];
  unsigned char ppc_altColor=0;
  short offset; /* due to negative offsets */
  unsigned short x;      /* Current X and Y coordinates */
  unsigned short y;
  unsigned short* px;   /* Pointers to X and Y coordinates used for actual plotting */
  unsigned short* py;
  unsigned char i; /* current character counter */
  unsigned char a; /* current character byte */
  unsigned char j,k; /* loop counters */
  char b; /* current character row bit signed */
  unsigned char width=FONT_SIZE_X;
  unsigned char height=FONT_SIZE_Y;
  unsigned short deltaX=1;
  unsigned short deltaY=1;
  unsigned char mainColor=current_foreground;
  unsigned char altColor=current_background;
  unsigned char *p;
  unsigned char* curfont;
  
  switch(CurMem)
    {
    case M0:
      curfont=font;
      offset=-32;
      break;
    case M1:
      curfont=font;
      offset=64;
      break;
    case M2:
      curfont=fontm23;
      offset=-32;
      break;
    case M3:
      curfont=fontm23;
      offset=32;      
      break;
    }

  if (CurMode==ModeRewrite)
    {
      altColor=current_background;
    }
  else if (CurMode==ModeInverse)
    {
      altColor=current_foreground;
    }
  
  if (CurMode==ModeErase || CurMode==ModeInverse)
    mainColor=current_background;
  else
    mainColor=current_foreground;

  current_foreground=mainColor;
  
  x=scalex[(Coord->x&0x1FF)];
  y=scaley[(Coord->y)+14&0x1FF];
  
  if (FastText==padF)
    {
      goto chardraw_with_fries;
    }

  /* the diet chardraw routine - fast text output. */
  
  for (i=0;i<count;++i)
    {
      a=*ch;
      ++ch;
      a+=offset;
      p=&curfont[fontptr[a]];
      
      for (j=0;j<FONT_SIZE_Y;++j)
  	{
  	  b=*p;
	  
  	  for (k=0;k<FONT_SIZE_X;++k)
  	    {
  	      if (b&0x80) /* check sign bit. */
		{
		  pp_mainColor[ppc_mainColor].x=x;
		  pp_mainColor[ppc_mainColor].y=y;
		  ppc_mainColor++;
		}
	      ++x;
  	      b<<=1;
  	    }

	  ++y;
	  x-=width;
	  ++p;
  	}

      mindset_gfx_blt_polypoint(0,ppc_mainColor,mainColor,0,0,&pp_mainColor);
      x+=width;
      y-=height;
      ppc_mainColor=0;
    }

  return;

 chardraw_with_fries:
  if (Rotate)
    {
      deltaX=-abs(deltaX);
      width=-abs(width);
      px=&y;
      py=&x;
    }
    else
    {
      px=&x;
      py=&y;
    }
  
  if (ModeBold)
    {
      deltaX = deltaY = 2;
      width<<=1;
      height<<=1;
    }
  
  for (i=0;i<count;++i)
    {
      a=*ch;
      ++ch;
      a+=offset;
      p=&curfont[fontptr[a]];
      for (j=0;j<FONT_SIZE_Y;++j)
  	{
  	  b=*p;

	  if (Rotate)
	    {
	      px=&y;
	      py=&x;
	    }
	  else
	    {
	      px=&x;
	      py=&y;
	    }

  	  for (k=0;k<FONT_SIZE_X;++k)
  	    {
  	      if (b&0x80) /* check sign bit. */
		{
		  if (ModeBold)
		    {
		      pp_mainColor[ppc_mainColor].x=*px+1;
		      pp_mainColor[ppc_mainColor].y=*py;
		      ppc_mainColor++;
		      pp_mainColor[ppc_mainColor].x=*px;
		      pp_mainColor[ppc_mainColor].y=*py+1;
		      ppc_mainColor++;
		      pp_mainColor[ppc_mainColor].x=*px+1;
		      pp_mainColor[ppc_mainColor].y=*py+1;
		      ppc_mainColor++;
		    }
		  pp_mainColor[ppc_mainColor].x=*px;
		  pp_mainColor[ppc_mainColor].y=*py;
		  ppc_mainColor++;
		}
	      else
		{
		  if (CurMode==ModeInverse || CurMode==ModeRewrite)
		    {
		      if (ModeBold)
			{
			  pp_altColor[ppc_altColor].x=*px+1;
			  pp_altColor[ppc_altColor].y=*py;
			  ppc_altColor++;
			  pp_altColor[ppc_altColor].x=*px;
			  pp_altColor[ppc_altColor].y=*py+1;
			  ppc_altColor++;
			  pp_altColor[ppc_altColor].x=*px+1;
			  pp_altColor[ppc_altColor].y=*py+1;
			  ppc_altColor++;
			}
		      pp_altColor[ppc_altColor].x=*px;
		      pp_altColor[ppc_altColor].y=*py;
		    }
		}

	      x += deltaX;
  	      b<<=1;
  	    }

	  y+=deltaY;
	  x-=width;
	  ++p;
  	}

      Coord->x+=width;
      x+=width;
      y-=height;
      mindset_gfx_blt_polypoint(0,ppc_altColor,altColor,0,0,&pp_altColor);
      mindset_gfx_blt_polypoint(0,ppc_mainColor,mainColor,0,0,&pp_mainColor);
      ppc_altColor=ppc_mainColor=0;
    }

  return;
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
  unsigned short newTVRed=theColor->red >> 5;
  unsigned short newTVGreen=(theColor->green >> 5) << 3;
  unsigned short newTVBlue=(theColor->blue >> 5) << 6;

  // TODO: Monitor colors.
  
  unsigned char i;

  for (i=0;i<16;i++)
    {
      if (i>highest_color_index)
	{
	  palette[i]=newTVRed|newTVGreen|newTVBlue;
	  highest_color_index++;
	  return i;
	}
      else if (palette[i]==(newTVRed|newTVGreen|newTVBlue))
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
