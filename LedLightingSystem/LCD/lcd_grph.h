/*
    This module implements a linux character device driver for the XXX chip.
    
      Copyright (C) 2006  Embedded Artists AB (www.embeddedartists.com)

    
      This program is free software; you can redistribute it and/or modify
    
      it under the terms of the GNU General Public License as published by
    
      the Free Software Foundation; either version 2 of the License, or
    
      (at your option) any later version.

    
      This program is distributed in the hope that it will be useful,
    
      but WITHOUT ANY WARRANTY; without even the implied warranty of
    
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    
      GNU General Public License for more details.

    
      You should have received a copy of the GNU General Public License
    
      along with this program; if not, write to the Free Software
    
      Foundation, Inc., 
      59 Temple Place, Suite 330, 
      Boston, MA  02111-1307  USA
 */



#ifndef _LCD_GRPH_

#define _LCD_GRPH_

#define DISPLAY_WIDTH  240

#define DISPLAY_HEIGHT 320

typedef unsigned short lcd_color_t;


#define   BLACK			0x0000		/*   0,   0,   0 */
#define   NAVY			0x000F      /*   0,   0, 128 */
#define   DARK_GREEN	0x03E0      /*   0, 128,   0 */
#define   DARK_CYAN		0x03EF      /*   0, 128, 128 */
#define   MAROON		0x7800      /* 128,   0,   0 */
#define   PURPLE		0x780F      /* 128,   0, 128 */
#define   OLIVE			0x7BE0      /* 128, 128,   0 */
#define   LIGHT_GRAY	0xC618      /* 192, 192, 192 */
#define   DARK_GRAY		0x7BEF      /* 128, 128, 128 */
#define   BLUE			0x001F      /*   0,   0, 255 */
#define   GREEN			0x07E0      /*   0, 255,   0 */
#define   CYAN          0x07FF      /*   0, 255, 255 */
#define   RED           0xF800      /* 255,   0,   0 */
#define   MAGENTA		0xF81F      /* 255,   0, 255 */
#define   YELLOW		0xFFE0      /* 255, 255, 0   */
#define   WHITE			0xFFFF      /* 255, 255, 255 */

void lcd_movePen(unsigned short x, unsigned short y);

void lcd_fillScreen(lcd_color_t color);

void lcd_point(unsigned short x, unsigned short y, lcd_color_t color);

void lcd_drawRect(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, lcd_color_t color);

void lcd_fillRect(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, lcd_color_t color);

void lcd_line(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, lcd_color_t color);

void lcd_circle(unsigned short x0, unsigned short y0, unsigned short r, lcd_color_t color);

unsigned long lcd_putChar(unsigned short x, unsigned short y, unsigned char ch);

void lcd_putString(unsigned short x, unsigned short y, unsigned char *pStr);

void lcd_fontColor(lcd_color_t foreground, lcd_color_t background);

void lcd_picture(unsigned short x, unsigned short y, unsigned short width, unsigned short height, unsigned short *pPicture);


void lcd_pictureBegin(unsigned short x, unsigned short y, unsigned short width, unsigned short height);

void lcd_pictureData(unsigned short *pPicture, unsigned short len);

void lcd_pictureEnd(void);



#endif /* _LCD_GRPH_ */
