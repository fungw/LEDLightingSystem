/*******************************************************************************
    This module implements a linux character device driver for the QVGA chip.
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
*******************************************************************************/
#include <LPC24xx.H>				/* LPC23xx/24xx definitions */
#include "lcd_hw.h"
#include <general.h>
#include <config.h>
#include "FreeRTOS.h"
#include "task.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/
#define LCD_COMMAND_16  (*((volatile unsigned short *) 0x82000000)) 
#define LCD_DATA_16     (*((volatile unsigned short *) 0x82000002))

#define BACKLIGHT_PIN 0x10000000  //P3.28

#define QVGA_TOUCH_ENABLE		1

#if (QVGA_TOUCH_ENABLE == 1)

	#define CS_PIN        0x00010000        //P0.16
	#define ACTIVATE_CS   IOCLR0 = CS_PIN
	#define DEACTIVATE_CS IOSET0 = CS_PIN

#endif


unsigned char activeController;

/******************************************************************************
** Function name:		mdelay
**
** Descriptions:		
**
** parameters:			delay length
** Returned value:		None
** 
******************************************************************************/
void mdelay( unsigned int delayInMs )
{
	// Wait for required delay.
	vTaskDelay( delayInMs / portTICK_RATE_MS );
}


#if (QVGA_TOUCH_ENABLE == 1)

static unsigned char
sendSpi(unsigned char indata)
{
	unsigned int failsafe;
	
  S0SPDR = indata;
  failsafe = 0;
  while(((S0SPSR & 0x80) == 0) && (failsafe < 5000))
    failsafe++;
  
  if (failsafe >= 5000)
  {
    S0SPCCR = 57;
    S0SPCR  = 0x38;
  }
  return S0SPDR;
}
#endif


/******************************************************************************
** Function name:		lcd_hw_init
**
** Descriptions:		Initialize hardware for LCD controller, it includes
**						both EMC and I/O initialization. External CS2 is used 
**						for LCD controller.		
**
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
void lcd_hw_init(void)
{
  unsigned int regVal;

  /****************************************************************
   * Initialize EMC for CS2
   ****************************************************************/
  regVal = PINSEL4;
  regVal &= 0x0FFFFFFF;
  regVal |= 0x50000000;
  PINSEL4 = regVal;
 
  regVal = PINSEL5;
  regVal &= 0xF0F0F000;
  regVal |= 0x05050555;
  PINSEL5 = regVal;

  PINSEL6 = 0x55555555;
  PINSEL8 = 0x55555555;

  regVal = PINSEL9;
  regVal &= 0x0F000000;
  regVal |= 0x50555555;
  PINSEL9 = regVal;

  // Initialize EMC for CS2
  EMC_STA_CFG2 = 0x00000081;	/* 16 bit, byte lane state, BLSn[3:0] are low. */ 
  EMC_STA_WAITWEN2  = 0x1;		/* WE delay 2(n+1)CCLK */
  EMC_STA_WAITOEN2  = 0x2;		/* OE delay, 2(n)CCLK */
  EMC_STA_WAITRD2   = 0x10;		/* RD delay, 17(n+1)CCLK */ 
  EMC_STA_WAITPAGE2 = 0x1F;		/* Page mode read delay, 32CCLK(default) */
  EMC_STA_WAITWR2   = 0x8;		/* Write delay, 10(n+2)CCLK */
  EMC_STA_WAITTURN2 = 0x5;		/* Turn arounc delay, 5(n+1)CCLK */
  EMC_STA_EXT_WAIT  = 0x0;		/* Extended wait time, 16CCLK */


#if (QVGA_TOUCH_ENABLE == 1)

  //init SPI #0 for touch screen i/f
  PINSEL0 |= 0xc0000000;
  PINSEL1 |= 0x0000003c;
  IODIR0  |= CS_PIN;
  IOSET0   = CS_PIN;
  
  S0SPCCR = 57;    
  S0SPCR  = 0x38;

#endif


  /****************************************************************
   * Setup control of backlight
   ****************************************************************/
  FIO3DIR = BACKLIGHT_PIN;		/* Set P3.28 as output */
  FIO3CLR = BACKLIGHT_PIN;		/* Set P3.28 low (= turn light on for >= V1.1 controller */

  //set P3.28 as PWM output (PWM1.5, third alternative function)
  PINSEL7 |= 0x03000000;
  PWM1PR  = 0x00;	  //no prescaling
  PWM1MCR = 0x02;	  //reset counter if MR0 match
  PWM1MR0 = 0x3000;   //period time equal about 5 ms
  PWM1MR5 = 0x0000;
  PWM1LER = 0x21;	  //latch MR0 and MR5
  PWM1PCR = 0x2000;   //enable PWMENA5
  PWM1TCR = 0x09;	  //enable counter and PWM


	/* Touch Screen Interrupts */
#if (QVGA_TOUCH_ENABLE == 1)

	ACTIVATE_CS;
	sendSpi(0x80 | 0x00 | 0x00);
	DEACTIVATE_CS;
#endif
 
  return;
}


void
writeToDisp(unsigned short data)
{
  LCD_DATA_16 = data;
}


unsigned short
readFromDisp(void)
{
  volatile unsigned short value;

  value = LCD_DATA_16;  //dummy data
  value = LCD_DATA_16;

  return value;
}


void
writeToReg(unsigned short addr, unsigned short data)
{
  if (activeController == V2_CONTROLLER)
  {
    LCD_COMMAND_16 = addr;
    LCD_DATA_16    = data;
    LCD_COMMAND_16 = 0x22;  //restore index register to GRAM
  }
  else
  {
    LCD_COMMAND_16 = data;
  }
}


unsigned short
readFromReg(unsigned char addr)
{
  if (activeController == V2_CONTROLLER)
  {
    LCD_COMMAND_16 = addr;
    return LCD_DATA_16;
  }
  else
  {
    LCD_COMMAND_16 = (unsigned short)addr << 8;
    return (unsigned short)LCD_COMMAND_16;
  }
}


/******************************************************************************
** Function name:		writeLcdCommand
**
** Descriptions:		
**
** parameters:			Command (16-bits)
** Returned value:	None
** 
******************************************************************************/
void
writeLcdCommand(unsigned short command)
{
  writeToReg(command, 0);
  switch ((command >> 8) & 0xff)
  {
    case 3:
    case 24:
    case 25:
    case 26:
    case 27:
    case 29:
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 40:
    case 55:
      mdelay(1);
    default:
      break;
  }
}


/******************************************************************************
** Function name:		lcd_init
**
** Descriptions:		Read LCD controller R49 and R50 to make sure
**						the controller has been initialized correctly,
**						then, turn on system, set display, adjust GAMMA,
**						finally, display.		
**
** parameters:			None
** Returned value:		TRUE or FALSE
** 
******************************************************************************/
unsigned int lcd_init(void)
{
  unsigned int result;
  
  /****************************************************************
   * Check if contact with V2 Lcd controller (read register R00 = 0x8989)
   ****************************************************************/
  if (readFromReg(0) == 0x8989)
  {
    activeController = V2_CONTROLLER;
    
    /****************************************************************
     * Initialize Lcd controller (long sequence) 
     ****************************************************************/
    writeToReg(0x07, 0x0021);
    writeToReg(0x00, 0x0001);
    writeToReg(0x07, 0x0723);
    writeToReg(0x10, 0x0000);
    mdelay(200);
    writeToReg(0x07, 0x0033);
    writeToReg(0x11, 0x6830);
    writeToReg(0x02, 0x0600);
    writeToReg(0x0f, 0x0000);
    writeToReg(0x01, 0x2b3f);
    writeToReg(0x0b, 0x5308);
    writeToReg(0x25, 0xa000);
  
    return( TRUE );
  }


  /****************************************************************
   * Check if contact with V1 Lcd controller (read register R49 & R50)
   ****************************************************************/
  /* read register R49=0x31, should be 0x10 */
  result = readFromReg(0x31);
  if (result != 0x10)
  {
	return( FALSE );
  }

  /* read register R50=0x32, should be 0x02 */
  result = readFromReg(0x32);
  if (result != 0x02)
  {
  	return( FALSE );
  }
  
  /****************************************************************
   * Initialize Lcd controller (long sequence) 
   ****************************************************************/
  activeController = V1_CONTROLLER;

  /* system power on */
  writeLcdCommand(0x0301);
  mdelay(10);
  writeLcdCommand(0x0111);
  mdelay(10);
  writeLcdCommand(0x0301);
  mdelay(10);
  writeLcdCommand(0x0028);
  mdelay(10);
  writeLcdCommand(0x2201);
  mdelay(10);
  writeLcdCommand(0x0020);
  mdelay(10);

  /* set Display Window */
  writeLcdCommand(0x0110);
  writeLcdCommand(0x0500);
  writeLcdCommand(0x4200);
  writeLcdCommand(0x4300);
  writeLcdCommand(0x4400);
  writeLcdCommand(0x4500);
  writeLcdCommand(0x46EF);
  writeLcdCommand(0x4700);
  writeLcdCommand(0x4800);
  writeLcdCommand(0x4901);
  writeLcdCommand(0x4A3F);
  writeLcdCommand(0x0200);
  writeLcdCommand(0x0D00);
  writeLcdCommand(0x0E00);
  writeLcdCommand(0x0F00);
  writeLcdCommand(0x1000);
  writeLcdCommand(0x1100);
  writeLcdCommand(0x1200);
  writeLcdCommand(0x1300);
  writeLcdCommand(0x1400);
  writeLcdCommand(0x1500);
  writeLcdCommand(0x1600);
  writeLcdCommand(0x1700);
  writeLcdCommand(0x1D08);
  writeLcdCommand(0x2300);
  writeLcdCommand(0x2D01);
  writeLcdCommand(0x3301);
  writeLcdCommand(0x3401);
  writeLcdCommand(0x3500);
  writeLcdCommand(0x3701);
  writeLcdCommand(0x3E01);
  writeLcdCommand(0x3F3F);
  writeLcdCommand(0x4008);
  writeLcdCommand(0x410A);
  writeLcdCommand(0x4C00);
  writeLcdCommand(0x4D01);
  writeLcdCommand(0x4E3F);
  writeLcdCommand(0x4F00);
  writeLcdCommand(0x5000);
  writeLcdCommand(0x7600);
  writeLcdCommand(0x8600);
  writeLcdCommand(0x8736);
  writeLcdCommand(0x8806);
  writeLcdCommand(0x8904);
  writeLcdCommand(0x8B3F);
  writeLcdCommand(0x8D01);


  /* adjust GAMMA */
  writeLcdCommand(0x8F00);
  writeLcdCommand(0x9022);
  writeLcdCommand(0x9167);
  writeLcdCommand(0x9240);
  writeLcdCommand(0x9307);
  writeLcdCommand(0x9412);
  writeLcdCommand(0x9522);
  writeLcdCommand(0x9600);
  writeLcdCommand(0x9707);
  writeLcdCommand(0x9873);
  writeLcdCommand(0x9901);
  writeLcdCommand(0x9A21);
  writeLcdCommand(0x9B24);
  writeLcdCommand(0x9C42);
  writeLcdCommand(0x9D01);

  mdelay(100);
  writeLcdCommand(0x2494);
  writeLcdCommand(0x256F);
  mdelay(100);

  writeLcdCommand(0x2812);
  mdelay(10);
  writeLcdCommand(0x1900);
  mdelay(10);
  writeLcdCommand(0x2110);
  mdelay(10);
  writeLcdCommand(0x1e00);
  mdelay(50);
  writeLcdCommand(0x18f7);
  mdelay(100);
  writeLcdCommand(0x2100);
  mdelay(10);
  writeLcdCommand(0x2812);
  mdelay(10);
  writeLcdCommand(0x1a00);
  mdelay(10);
  writeLcdCommand(0x197c);
  mdelay(10);
  writeLcdCommand(0x1f51);
  writeLcdCommand(0x2060);

  mdelay(10);
  writeLcdCommand(0x1e80);
  mdelay(10);
  writeLcdCommand(0x1b0b);
  mdelay(10);

  //Display Start
  writeLcdCommand(0x0020);
  mdelay(10);
  writeLcdCommand(0x3b01);

  return( TRUE );
}

#if (QVGA_TOUCH_ENABLE == 1)
void
getTouch(unsigned int* xPos, unsigned int* yPos, unsigned int* pressure)
{
    tU32 Xposition;
    tU32 Yposition;
    tU32 Z1position;
    tU32 Z2position;
	tU32 RTouch;
	
	//Read X-position
    ACTIVATE_CS;
//    sendSpi(0x83 | 0x50 | 0x00);
    sendSpi(0x80 | 0x50 | 0x00);
    Xposition  = ((tU32)sendSpi(0x00)) << 8;
    Xposition |= (tU32)sendSpi(0x00);
    Xposition >>= 3;
    DEACTIVATE_CS;

    //Read Y-position
    ACTIVATE_CS;
//    sendSpi(0x83 | 0x10 | 0x00);
	sendSpi(0x80 | 0x10 | 0x00);
	Yposition  = ((tU32)sendSpi(0x00)) << 8;
    Yposition |= (tU32)sendSpi(0x00);
    Yposition >>= 3;
    DEACTIVATE_CS;

    //Read Z1-position
    ACTIVATE_CS;
//    sendSpi(0x83 | 0x30 | 0x00);
    sendSpi(0x80 | 0x30 | 0x00);
    Z1position  = ((tU32)sendSpi(0x00)) << 8;
    Z1position |= (tU32)sendSpi(0x00);
    Z1position >>= 3;
    DEACTIVATE_CS;

    //Read Z2-position
    ACTIVATE_CS;
//    sendSpi(0x83 | 0x40 | 0x00);
    sendSpi(0x80 | 0x40 | 0x00);
    Z2position  = ((tU32)sendSpi(0x00)) << 8;
    Z2position |= (tU32)sendSpi(0x00);
    Z2position >>= 3;
    DEACTIVATE_CS;

    //Calculate pressure (with Rx-plate = 4096)
	RTouch = ((Xposition * Z2position) / Z1position) - Xposition;

	if (!RTouch || RTouch > 65535)
	{
		*pressure = 0;
	}
	else
	{
	 	*pressure = 65535 - RTouch;
	}
	
	// Calculate yPos
	*xPos = 240 - ((Yposition * 240) >> 12);
		
	// Calculate xPos
	*yPos = 320 - ((Xposition * 320) >> 12);
}

#endif

