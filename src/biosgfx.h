#ifndef BIOSGFX_H
#define BIOSGFX_H

#define BLACK        0
#define BLUE         1
#define GREEN        2
#define CYAN         3
#define RED          4
#define MAGENTA      5
#define BROWN        6
#define LIGHTGREY    7
#define DARKGREY     8
#define LIGHTBLUE    9
#define LIGHTGREEN   10
#define LIGHTCYAN    11
#define LIGHTRED     12
#define LIGHTMAGENTA 13
#define YELLOW       14
#define WHITE        15

/* SET PIXEL MODE VALUES */
#define PIXEL_MODE_OPAQUE          0
#define PIXEL_MODE_TRANSPARENT     1
#define PIXEL_MODE_COPY            0
#define PIXEL_MODE_AND             1
#define PIXEL_MODE_OR              2
#define PIXEL_MODE_XOR             3
#define PIXEL_MODE_NOT             4
#define PIXEL_MODE_AND_NOT         5
#define PIXEL_MODE_OR_NOT          6
#define PIXEL_MODE_XOR_NOT         7

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
void mindset_mode(unsigned char mode);

/**
 * Set pixel transfer mode
 */
void mindset_set_transfer_mode(unsigned char transparent, unsigned char mode);

/**
 * Draw line
 *
 * short x1 - initial X position
 * short y1 - initial Y position
 * short x2 - final X position
 * short y2 - final Y position
 * unsigned char c - color
 */
void mindset_line(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned char c);

/**
 * Draw bar (with a bunch of lines)
 *
 * short x1 - initial left corner of box
 * short y1 - initial top corner of box
 * short x2 - final right corner of box
 * short y2 - final bottom corner of box
 * unsigned char c - Color
 */
void mindset_bar(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned char c);

#endif /* BIOSGFX_H */
