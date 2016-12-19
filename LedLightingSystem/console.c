/* Scheduler include files. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "serial.h"
#include "console.h"

#define consoleSTACK_SIZE			( ( unsigned portBASE_TYPE ) 256 )
#define consoleBUFFER_LEN			( ( unsigned portBASE_TYPE ) 256 )
#define consoleMAX_DELAY			( ( portTickType ) 1000 )

/* Handle to the com port used by the console. */
static xComPortHandle xPort;

/* The console task. */
static void vConsoleTask( void *pvParameters );

/* Console prompt */
const char *pcPrompt = "Command> ";

void vStartConsole( unsigned portBASE_TYPE uxPriority, unsigned long ulBaudRate)
{
	/* Initialise the com port. */
	xPort = xSerialPortInitMinimal( ulBaudRate, consoleBUFFER_LEN );

	/* Spawn the console task . */
	xTaskCreate( vConsoleTask, "Console", consoleSTACK_SIZE, NULL, uxPriority, ( xTaskHandle * ) NULL );

	printf("Console task started ...\r\n");
}

xComPortHandle xConsolePortHandle(void)
{
	return xPort;
}

static portTASK_FUNCTION( vConsoleTask, pvParameters )
{
	char cRxChar;

	/* Just to stop compiler warnings. */
	( void ) pvParameters;

	for( ;; )
	{
		/* Display prompt */
		vSerialPutString(xPort, pcPrompt, strlen((const char *)pcPrompt));

		cRxChar = 0;

		while (cRxChar != '\r')
		{
			/* Read input */
			xSerialGetChar(xPort, &cRxChar, portMAX_DELAY);
	
			/* Echo input */
			xSerialPutChar(xPort, cRxChar, consoleMAX_DELAY);

			if (cRxChar == '\r')
			{
				xSerialPutChar(xPort, '\n', consoleMAX_DELAY);
			}
		}
	}
}
