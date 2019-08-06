#include <i86.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include "io.h"
#include "protocol.h"
#include "log.h"

#define SERIAL 0x14
#define PORT 0

unsigned short* ibuf;
unsigned short* obuf;
unsigned char rts; // RTS enabled/disabled?

#define IBUF_SIZE 8192
#define OBUF_SIZE 128

union REGPACK regs;
union REGS iregs;

void io_init(void)
{
  ibuf=malloc(IBUF_SIZE*sizeof(unsigned short));
  obuf=malloc(IBUF_SIZE*sizeof(unsigned short));
  
  // Initialize port.
  regs.x.dx = PORT;
  regs.h.ah = 0x04;
  intr(SERIAL, &regs);

  // Set line characteristics. 
  regs.h.al = 0x07; // 9600bps
  regs.h.al = (regs.h.al << 5) | 0x03;   /* 8/N/1 */
  regs.x.dx = PORT;
  regs.h.ah = 0x00;
  intr(SERIAL,&regs);

  // Set input/output buffers
  regs.h.ah=0x2C;
  regs.w.cx=IBUF_SIZE;
  regs.w.dx=0;
  regs.w.es=FP_SEG(ibuf);
  regs.w.bx=FP_OFF(ibuf);
  intr(0xEE,&regs);

  regs.h.ah=0x2D;
  regs.w.cx=OBUF_SIZE;
  regs.w.dx=0;
  regs.w.es=FP_SEG(obuf);
  regs.w.bx=FP_OFF(obuf);
  intr(0xEE,&regs);
  
  // Set Interrupt driven mode
  regs.h.ah=0x2E;
  regs.h.al=0x3; // Set DTR/RTS to enabled.
  regs.h.bl=0x3; // Enable both interrupts.
  intr(0xEE,&regs);
  
}

void io_send_byte(unsigned char b)
{
  regs.x.dx = PORT;
  regs.h.al = b;
  regs.h.ah = 0x28;
  intr(0xEE, &regs);
}

void io_lower_dtr(void)
{
  /* // Lower DTR */
  regs.h.ah = 0x06;
  regs.h.al = 0x00;
  regs.x.dx = PORT;
  intr(SERIAL,&regs);
}

void io_raise_dtr(void)
{
  /* // Raise DTR */
  regs.h.ah = 0x06;
  regs.h.al = 0x01;
  regs.x.dx = PORT;
  intr(SERIAL,&regs);
}

void io_hang_up(void)
{
  io_lower_dtr();
  delay(500);
  io_raise_dtr();
}

void io_rts(unsigned char t)
{
  regs.h.ah=0x2E;
  regs.h.al=(t==1 ? 3 : 1);
  regs.x.dx=PORT;
  intr(0xEE,&regs);
  rts=t;
}

void io_main(void)
{
  unsigned char ch;
  // Get port status
  iregs.x.dx = PORT;
  iregs.h.ah = 0x2B;
  int86(0xEE, &iregs, &iregs);

  if (iregs.w.bx<IBUF_SIZE-1)
    {
      // Data is waiting

      // Toggle RTS if our buffer is hitting a threshold.
      if ((iregs.w.bx<128) && (rts==1))
	io_rts(0);
      else if ((iregs.w.bx>7168) && (rts==0))
	io_rts(1);
      
      iregs.x.dx = PORT;
      iregs.h.ah = 0x29;
      int86(0xEE, &iregs, &iregs);
      ch=iregs.h.al;
      ShowPLATO(&ch,1);
    }
}

void io_done(void)
{
  io_hang_up();

  // FIXME: do we need to disable the interrupts?
  free(ibuf);
  free(obuf);
  
}
