#include <i86.h>
#include <stdlib.h>
#include "biosgfx.h"

#define INT_VIDEO 0xEF

unsigned short* dot;
unsigned short* line;
unsigned short* bar;
unsigned short* text;
/* unsigned short* font; */

static const char* hello="Hello";

/**
 * Mindset init
 */
void mindset_init(void)
{
  dot=malloc(2);
  line=malloc(4);
  bar=malloc(10);
  text=malloc(10);
  /* font=malloc(14); */
}

/**
 * Set video mode
 * 
 * unsigned char m - video mode
 *
 * 0 - 320x200x1
 * 1 - 320x200x2
 * 2 - 320x200x4
 * 3 - 640x200x1
 * 4 - 640x200x2
 * 5 - 320x400x2
 * 6 - 640x400x1
 */
void mindset_mode(unsigned char mode)
{
  union REGS regs;
  regs.h.ah=0x00;
  regs.h.al=mode;
  int86(INT_VIDEO,&regs,&regs);
}

/**
 * Set font
 */
void mindset_set_font(void)
{
  /* union REGS regs; */

  /* font[0]=0x0000;    // Fixed font, no excess space. */
  /* font[1]=0x0005;    // 5 pixels wide */
  /* font[2]=0x0006;    // 6 pixels high */
  /* font[3]=0x0002;    // one word per bitmap. */
  /* font[4]=FP_SEG(&fontdata[0]);  // Font address segment */
  /* font[5]=FP_OFF(&fontdata[0]);  // Font address offset */
  /* font[6]=0x20AA;    // first and last ASCII value */
  
  /* regs.h.ah=0x1F; // Set font pointer */
  /* regs.w.bx=FP_OFF(&font[0]); */
  /* int86(INT_VIDEO,&regs,&regs); */
}

/**
 * Set pixel transfer mode
 */
void mindset_set_transfer_mode(unsigned char transparent, unsigned char mode)
{
  union REGS regs;
  regs.h.ah=0x02;
  regs.h.bh=transparent;
  regs.h.bl=mode;
  int86(INT_VIDEO,&regs,&regs);
}

/**
 * Draw line
 *
 * short x1 - initial X position
 * short y1 - initial Y position
 * short x2 - final X position
 * short y2 - final Y position
 * unsigned char c - color
 */
void mindset_line(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned char c)
{
  union REGS regs;

  line[0]=x1;
  line[1]=y1;
  line[2]=x2;
  line[3]=y2;
  
  regs.h.ah=0x0D;   // Draw polyline
  regs.h.al=0x00;   // BLT ID, not really used.
  regs.w.cx=2;      // 2 pairs of coordinates.
  regs.h.dh=c;      // color
  regs.h.dl=1;      // paired mode
  regs.w.si=0;      // X origin
  regs.w.di=0;      // Y origin
  regs.w.bx=FP_OFF(&line[0]);  // Line
  int86(INT_VIDEO,&regs,&regs);
}

/**
 * Draw dot
 *
 * short x1 - initial X position
 * short y1 - initial Y position
 * short x2 - final X position
 * short y2 - final Y position
 * unsigned char c - color
 */
void mindset_dot(unsigned short x, unsigned short y, unsigned char c)
{
  union REGS regs;

  dot[0]=x;
  dot[1]=y;
  
  regs.h.ah=0x0C;   // Draw polypoint
  regs.h.al=0x00;   // BLT ID, not really used.
  regs.w.cx=1;      // one coordinate pair
  regs.h.dh=c;      // color
  regs.h.dl=1;      // paired mode
  regs.w.si=0;      // X origin
  regs.w.di=0;      // Y origin
  regs.w.bx=FP_OFF(&dot[0]);  // dot
  int86(INT_VIDEO,&regs,&regs);
}

/**
 * Draw bar (with a bunch of lines)
 *
 * short x1 - initial left corner of box
 * short y1 - initial top corner of box
 * short x2 - final right corner of box
 * short y2 - final bottom corner of box
 * unsigned char c - Color
 */
void mindset_bar(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned char c)
{
  union REGS regs;
  
  bar[0]=x1;
  bar[1]=y1;
  bar[2]=x2;
  bar[3]=y1;
  bar[4]=x2;
  bar[5]=y2;
  bar[6]=x1;
  bar[7]=y2;
  bar[8]=x1;
  bar[9]=y1;
  
  regs.h.ah=0x19;   // draw filled polygon
  regs.h.al=0x00;   // BLT ID (not really used)
  regs.w.cx=5;      // 5 pairs of coordinates
  regs.h.dh=c;      // even color index.
  regs.h.dl=c;      // odd color index.
  regs.w.si=0;      // X origin
  regs.w.di=0;      // Y origin
  regs.w.bx=FP_OFF(&bar[0]);   // bar
  int86(INT_VIDEO,&regs,&regs);
}

/**
 * Mindset text output
 */
/* void mindset_text(short x, short y, char* ch, unsigned short count) */
/* { */
/*   union REGS regs; */

/*   text[0]=x; */
/*   text[1]=y; */
/*   text[2]=5; */
/*   text[3]=FP_OFF(&hello); */
/*   text[4]=FP_SEG(&hello); */
  
/*   regs.h.ah=0x21; // BLT string */
/*   regs.h.al=0x00; // BLT id (not really used.) */
/*   regs.h.ch=1;    // One char string */
/*   regs.h.cl=0;    // Do not ignore any chars at beginning of string */
/*   regs.h.dh=0;    // Left to right, for now. */
/*   regs.h.dl=WHITE;    // Color */
/*   regs.w.si=0;    // X origin */
/*   regs.w.di=0;    // Y origin */
/*   regs.w.bx=FP_OFF(&text[0]); // Text Parameter block */
/*   int86(INT_VIDEO,&regs,&regs); */
/* } */

/**
 * Mindset clear screen
 */
void mindset_clear_screen(void)
{
  union REGS regs;

  regs.h.ah=0x1E;   // Fill Dest Buffer
  regs.w.bx=0x0000; // Completely empty.
  int86(INT_VIDEO,&regs,&regs);
}
