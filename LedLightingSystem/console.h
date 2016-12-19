#ifndef CONSOLE_H
#define CONSOLE_H

#include "serial.h"

void vStartConsole( unsigned portBASE_TYPE uxPriority, unsigned long ulBaudRate );
xComPortHandle xConsolePortHandle(void);

#endif
