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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _LCD_HW_
#define _LCD_HW_

#define V1_CONTROLLER			1
#define V2_CONTROLLER			2

extern void mdelay( unsigned int delay );
extern void lcd_hw_init( void );
extern unsigned int lcd_init(void);

extern void           writeToDisp(unsigned short data);
extern unsigned short readFromDisp(void);
extern void           writeToReg(unsigned short data, unsigned short addr);
extern unsigned short readFromReg(unsigned char addr);
extern void           writeLcdCommand(unsigned short command);
extern void           getTouch(unsigned int* xPos, unsigned int* yPos, unsigned int* pressure);
extern unsigned char  activeController;

#endif /* _LCD_HW_ */
