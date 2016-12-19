/******************************************************************************/
/* RETARGET.C: 'Retarget' layer for target-dependent low level functions      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stdio.h>
#include <rt_misc.h>
#include "serial.h"
#include "console.h"

#pragma import(__use_no_semihosting_swi)


struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;


int fputc(int ch, FILE *f) {
  /* return (sendchar(ch)); */
  xSerialPutChar(xConsolePortHandle(), ch, portMAX_DELAY);
  return ch;
}


int fgetc(FILE *f) {
  char cRxChar = NULL;
  /* return (sendchar(_getkey())); */
  xSerialGetChar(xConsolePortHandle(), &cRxChar, portMAX_DELAY);
  xSerialPutChar(xConsolePortHandle(), cRxChar, portMAX_DELAY);
  return cRxChar;
}


int ferror(FILE *f) {
  /* Your implementation of ferror */
  return EOF;
}


void _ttywrch(int ch) {
  /*sendchar(ch);*/
  xSerialPutChar(xConsolePortHandle(), ch, portMAX_DELAY);

}


void _sys_exit(int return_code) {
label:  goto label;  /* endless loop */
}
