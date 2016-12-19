/*****************************************************************************
 *   lcd_graph.c:  Graphic C file for NXP LPC24xx Family Microprocessors
 *
 *   Copyright (c) 2006 Embedded Artists AB
 *   All rights reserved.
 *
 *   History
 *   2007.01.11  ver 1.00    Prelimnary version, first Release
 *   2007.01.11  ver 1.00    Prelimnary version, first Release
 *
*****************************************************************************/

#include <LPC24xx.H>
#include <general.h>
#include "lcd_hw.h"
#include "lcd_grph.h"
#include "font5x7.h"


static lcd_color_t  foregroundColor = WHITE;
static lcd_color_t  backgroundColor = BLACK;

static unsigned char const  font_mask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};


/******************************************************************************
** Function name:		hLine		
**
** Descriptions:		Draw a horizontal line from x0 to x1 on y0.		
**
** parameters:			x0, y0, x1, color
** Returned value:		None
** 
******************************************************************************/
static void hLine(unsigned short x0, unsigned short y0, unsigned short x1, lcd_color_t color) 
{
  unsigned short bak;

  if (x0 > x1) 						
  {
    bak = x1;
    x1 = x0;
    x0 = bak;
  }
  lcd_point(x0, y0, color);
  x0++;
   
  while(x1 >= x0)
  {
    writeToDisp(color);
    x0++;
  }
  return;
}

/******************************************************************************
** Function name:		vLine
**
** Descriptions:		Draw a vertical line from y0 to y1 on x0.
**
** parameters:			x0, y0, y1, color
** Returned value:		None
** 
******************************************************************************/
static void vLine(unsigned short x0, unsigned short y0, unsigned short y1, lcd_color_t color)
{
  unsigned short bak;

  if(y0 > y1) 						
  {
    bak = y1;
    y1 = y0;
    y0 = bak;
  }

  while(y1 >= y0)
  {
    lcd_point(x0, y0, color);
    y0++;
  }
  return;
}


/******************************************************************************
** Function name:		lcd_movePen
**
** Descriptions:		Move the pen to a particular location.		
**
** parameters:			pixel x and y
** Returned value:		None
** 
******************************************************************************/
void lcd_movePen(unsigned short x, unsigned short y)
{
  if (activeController == V2_CONTROLLER)
  {
    writeToReg(0x4e, x & 0xff);
    writeToReg(0x4f, y & 0x1ff);
  }
  else
  {
    writeLcdCommand(0x4200 | (x & 0xff));	  	  /* x start address */
    writeLcdCommand(0x4300 | ((y>>8) & 0xff));  /* y start address MSB */
    writeLcdCommand(0x4400 | (y & 0xff));       /* y start address LSB */
  }
  return;
}

/******************************************************************************
** Function name:		lcd_setWindow
**
** Descriptions:		Set the window area without filling the color		
**
** parameters:			x0, y0, x1, y1
** Returned value:		If the range is not set correctly, e.g. x1 <= x0
**				y1 <= y0, return false, the window will not be set.
** 
******************************************************************************/
unsigned long lcd_setWindow(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1)  
{
  if (x1 > DISPLAY_WIDTH-1) 
  {  
    x1 = DISPLAY_WIDTH-1;
  }
  
  if (y1 > DISPLAY_HEIGHT-1)
  {
    y1 = DISPLAY_HEIGHT-1;
  }
  
  if ((x1 <= x0) || (y1 <= y0))
  {
    return( FALSE );
  }

  if (activeController == V1_CONTROLLER)
  {
    /* TODO x and y values aren't used below. 240x320 (239x319) is always used */
    writeLcdCommand(0x4500); /* X-start address */
    writeLcdCommand(0x46EF); /* X-end address */
    writeLcdCommand(0x4700); /* Y-start address MSB */
    writeLcdCommand(0x4800); /* Y-start address LSB */
    writeLcdCommand(0x4901); /* Y-end address MSB */
    writeLcdCommand(0x4A3F); /* Y-end address LSB */
  }
  return( TRUE );
}

/******************************************************************************
** Function name:		lcd_fillScreen
**
** Descriptions:		Fill the LCD screen with color		
**
** parameters:			Color
** Returned value:		None
** 
******************************************************************************/
void lcd_fillScreen(lcd_color_t color)
{
  unsigned short i = 0;
  unsigned short j = 0;

  
  lcd_setWindow(0, 0, DISPLAY_WIDTH-1, DISPLAY_HEIGHT-1); 
  lcd_movePen(0, 0);

  for(i=0; i < DISPLAY_HEIGHT; i++)
  {
    for(j=0; j<DISPLAY_WIDTH; j++)
    {
      writeToDisp(color);
    }
  }
  return;
}


/******************************************************************************
** Function name:		lcd_point
**
** Descriptions:		Draw a point at {x0, y0} on the LCD		
**				if {x0,y0} is out of range, display nothing.
** parameters:			x0, y0, color
** Returned value:		None
** 
******************************************************************************/
void lcd_point(unsigned short x, unsigned short y, lcd_color_t color)
{
  if( x >= DISPLAY_WIDTH )  
  {
	return;
  }
  
  if(y >= DISPLAY_HEIGHT)
  {
	return;
  }
  lcd_movePen(x, y);
  writeToDisp(color);
  return;
}


/******************************************************************************
** Function name:		lcd_movePen
**
** Descriptions:		
**
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
void lcd_drawRect(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, lcd_color_t color)
{  
  hLine(x0, y0, x1, color);
  hLine(x0, y1, x1, color);
  vLine(x0, y0, y1, color);
  vLine(x1, y0, y1, color);
  return;
}

/******************************************************************************
** Function name:		lcd_fillRect
**
** Descriptions:		Fill a rectangle with color, the area is
**						{x0, y0, x1, y1 }
**
** parameters:			x0, y0, x1, y1, color
** Returned value:		None
** 
******************************************************************************/
void lcd_fillRect(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, lcd_color_t color)
{  
  unsigned short i = 0;
  
  if(x0 > x1)
  {   
	i  = x0;
	x0 = x1;
	x1 = i;
  }

  if(y0 > y1)
  {   
	i  = y0;
	y0 = y1;
	y1 = i;
  }
   
  if(y0 == y1) 
  {  
	hLine(x0, y0, x1, color);
	return;
  }

  if(x0 == x1) 
  {  
	vLine(x0, y0, y1, color);
	return;
  }

  while(y0 <= y1)						
  {  
	hLine(x0, y0, x1, color);
	y0++;
  }
  return;
}

/******************************************************************************
** Function name:		lcd_line
**
** Descriptions:		draw a line between {x0,y0} and {x1,y1}
**						the last parameter is the color of the line		
**
** parameters:			x0, y0, x1, y1, color
** Returned value:		None
** 
******************************************************************************/
void lcd_line(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, lcd_color_t color)
{  
  signed short   dx = 0, dy = 0;
  signed char    dx_sym = 0, dy_sym = 0;
  signed short   dx_x2 = 0, dy_x2 = 0;
  signed short   di = 0;
 
  dx = x1-x0;
  dy = y1-y0;
  

  if(dx == 0)			/* vertical line */ 
  {
	vLine(x0, y0, y1, color);
	return;
  }

  if(dx > 0)
  {    
	dx_sym = 1;
  }
  else
  { 
	dx_sym = -1;
  }

  if(dy == 0)			/* horizontal line */  
  {
	hLine(x0, y0, x1, color);
	return;
  }

  if(dy > 0)
  {    
	dy_sym = 1;
  }
  else
  { 
	dy_sym = -1;
  }

  dx = dx_sym*dx;
  dy = dy_sym*dy;
 
  dx_x2 = dx*2;
  dy_x2 = dy*2;
   
  if(dx >= dy)
  { 
	di = dy_x2 - dx;
	while(x0 != x1)
	{   
	  lcd_point(x0, y0, color);
	  x0 += dx_sym;
	  if(di<0)
	  {
		di += dy_x2;
	  }
	  else
	  {
		di += dy_x2 - dx_x2;
		y0 += dy_sym;
	  }
	}
	lcd_point(x0, y0, color);
  }
  else
  {
	di = dx_x2 - dy;
	while(y0 != y1)
	{   
	  lcd_point(x0, y0, color);
	  y0 += dy_sym;
	  if(di < 0)
	  { 
		di += dx_x2;
	  }
	  else
	  {
		di += dx_x2 - dy_x2;
		x0 += dx_sym;
	  }
	}
	lcd_point(x0, y0, color);
  }
  return; 
}

/******************************************************************************
** Function name:		lcd_circle
**
** Descriptions:		Use x0 and y0 as the center point to draw a 
**				a cycle with radius length r, and the latest parameter
**				is the color of the circle		
**
** parameters:			x0, y0, radius, color
** Returned value:		None
** 
******************************************************************************/
void lcd_circle(unsigned short x0, unsigned short y0, unsigned short r, lcd_color_t color)
{
  signed short draw_x0, draw_y0;
  signed short draw_x1, draw_y1;	
  signed short draw_x2, draw_y2;	
  signed short draw_x3, draw_y3;	
  signed short draw_x4, draw_y4;	
  signed short draw_x5, draw_y5;	
  signed short draw_x6, draw_y6;	
  signed short draw_x7, draw_y7;	
  signed short xx, yy;
  signed short  di;
   
  if(r == 0)		  /* no radius */ 
  {
	return;
  }
   
  draw_x0 = draw_x1 = x0;
  draw_y0 = draw_y1 = y0 + r;
  if(draw_y0 < DISPLAY_HEIGHT)
  { 
	lcd_point(draw_x0, draw_y0, color);		/* 90 degree */
  }
	
  draw_x2 = draw_x3 = x0;
  draw_y2 = draw_y3 = y0 - r;
  if(draw_y2 >= 0)
  {
	lcd_point(draw_x2, draw_y2, color);    /* 270 degree */
  }
   	
  draw_x4 = draw_x6 = x0 + r;
  draw_y4 = draw_y6 = y0;
  if(draw_x4 < DISPLAY_WIDTH)
  {
	lcd_point(draw_x4, draw_y4, color);		/* 0 degree */
  }
   
  draw_x5 = draw_x7 = x0 - r;
  draw_y5 = draw_y7 = y0;
  if(draw_x5>=0)
  {
	lcd_point(draw_x5, draw_y5, color);		/* 180 degree */
  }

  if(r == 1)
  {
	return;
  }
   
  di = 3 - 2*r;
  xx = 0;
  yy = r;
  while(xx < yy)
  {  
	if(di < 0)
	{
	  di += 4*xx + 6;	      
	}
	else
	{  
	  di += 4*(xx - yy) + 10;
	  yy--;	  
	  draw_y0--;
	  draw_y1--;
	  draw_y2++;
	  draw_y3++;
	  draw_x4--;
	  draw_x5++;
	  draw_x6--;
	  draw_x7++;	 	
	}  
	xx++;   
	draw_x0++;
	draw_x1--;
	draw_x2++;
	draw_x3--;
	draw_y4++;
	draw_y5++;
	draw_y6--;
	draw_y7--;
	
	if( (draw_x0 <= DISPLAY_WIDTH) && (draw_y0>=0) )	
	{
      lcd_point(draw_x0, draw_y0, color);
	}
	    
	if( (draw_x1 >= 0) && (draw_y1 >= 0) )	
	{ 
      lcd_point(draw_x1, draw_y1, color);
	}

	if( (draw_x2 <= DISPLAY_WIDTH) && (draw_y2 <= DISPLAY_HEIGHT) )
	{
	  lcd_point(draw_x2, draw_y2, color);
	}

	if( (draw_x3 >=0 ) && (draw_y3 <= DISPLAY_HEIGHT) )	
	{
	  lcd_point(draw_x3, draw_y3, color);
	}

	if( (draw_x4 <= DISPLAY_HEIGHT) && (draw_y4 >= 0) )	
	{
	  lcd_point(draw_x4, draw_y4, color);
	}

	if( (draw_x5 >= 0) && (draw_y5 >= 0) )
	{  
	  lcd_point(draw_x5, draw_y5, color);
	}
	if( (draw_x6 <= DISPLAY_WIDTH) && (draw_y6 <= DISPLAY_HEIGHT) )	
	{
	  lcd_point(draw_x6, draw_y6, color);
	}
	if( (draw_x7 >= 0) && (draw_y7 <= DISPLAY_HEIGHT) )	
	{  
	  lcd_point(draw_x7, draw_y7, color);
	}
  }
  return;
}

/******************************************************************************
** Function name:		lcd_putChar
**
** Descriptions:		Put one chacter on the LCD for display		
**
** parameters:			pixel X and Y, and the character
** Returned value:		TRUE or FALSE, if the pixels given are out of range,
**						nothing will be written.
** 
******************************************************************************/
unsigned long lcd_putChar(unsigned short x, unsigned short y, unsigned char ch)
{  
  unsigned char data = 0;
  unsigned char i = 0, j = 0;
  
  lcd_color_t color = BLACK;

  if((x >= (DISPLAY_WIDTH - 8)) || (y >= (DISPLAY_HEIGHT - 8)) )
  {
	return( FALSE );
  }

  if( (ch < 0x20) || (ch > 0x7f) )
  {
	ch = 0x20;		/* unknown character will be set to blank */
  }
   
  ch -= 0x20;
  for(i=0; i<8; i++)
  {
    data = font5x7[ch][i];
    for(j=0; j<6; j++)
    {
	    if( (data&font_mask[j])==0 )
	    {  
		    color = backgroundColor;
	    }
	    else
	    {
		    color = foregroundColor;
	    }
	    lcd_point(x, y, color);       
	    x++;
    }   
    y++;
    x -= 6;
  }
  return( TRUE );
}

/******************************************************************************
** Function name:		lcd_putString
**
** Descriptions:		Put a string of characters for display		
**
** parameters:			x and y pixels, and the pointer to the string characters
** Returned value:		None
** 
******************************************************************************/
void lcd_putString(unsigned short x, unsigned short y, unsigned char *pStr)
{
  while(1)
  {      
	  if( (*pStr)=='\0' )
	  {
		  break;
	  }
	  if( lcd_putChar(x, y, *pStr++) == 0 )
	  {
  		break;
  	}
  	x += 6;
  }
  return;
}

/******************************************************************************
** Function name:		lcd_fontColor
**
** Descriptions:		foreground and back ground color setting		
**
** parameters:			foreground color and background color
** Returned value:		None
** 
******************************************************************************/

void lcd_fontColor(lcd_color_t foreground, lcd_color_t background)
{
  foregroundColor = foreground;
  backgroundColor = background;
  return;
}

/******************************************************************************
** Function name:		lcd_pictureBegin
**
** Descriptions:		Set where the pixels of the picture should be set, the 
**						size of the picture.		
**
** parameters:			x, y, width, and height
** Returned value:		None
** 
******************************************************************************/
void lcd_pictureBegin(unsigned short x, unsigned short y, unsigned short width, unsigned short height)
{
  /* set window */
  lcd_setWindow(x,y, x+width-1,y+height-1);
  lcd_movePen(x, y);
  return;
}

/******************************************************************************
** Function name:		lcd_pictureData
**
** Descriptions:		
**
** parameters:			pointer to the picture and total size which is normally
**				the width multiply by the height.
** Returned value:		None
** 
******************************************************************************/
void lcd_pictureData(unsigned short *pPicture, unsigned short len)
{
  unsigned short i = 0;
  
  for (i=0; i<len; i++)
  {
    writeToDisp(*pPicture++);
  }
  return;
}

/******************************************************************************
** Function name:		lcd_pictureEnd
**
** Descriptions:		Restore window		
**
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
void lcd_pictureEnd(void)
{
  /* restore window */
  lcd_setWindow(0,0, DISPLAY_WIDTH-1,DISPLAY_HEIGHT-1);
  return;
}

/******************************************************************************
** Function name:		lcd_picture
**
** Descriptions:		put a picture file on the LCD display
**						pixels, width, and height are the parameters of
**						of the location, pPicuture is the pointer to the
**						picture		
**
** parameters:			x, y, width, height, pointer to the picture
** Returned value:		None
** 
******************************************************************************/
void lcd_picture(unsigned short x, unsigned short y, unsigned short width, unsigned short height, unsigned short *pPicture)
{
  lcd_pictureBegin(x, y, width, height);
  lcd_pictureData(pPicture, width*height);
  lcd_pictureEnd();
  return;

}
