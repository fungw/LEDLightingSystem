/* 
	Sample task that initialises the EA QVGA LCD display
	with touch screen controller and processes touch screen
	interrupt events.

	Jonathan Dukes (jdukes@scss.tcd.ie)
	Wesley Fung (fungw@tcd.ie)
*/

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "lcd.h"
#include "lcd_hw.h"
#include "lcd_grph.h"
#include "timers.h"
#include "ui.h"
#include "commands.h"
#include <stdio.h>
#include <string.h>

/* Maximum task stack size */
#define lcdSTACK_SIZE			( ( unsigned portBASE_TYPE ) 256 )

/* Interrupt handlers */
extern void vLCD_ISREntry( void );
void vLCD_ISRHandler( void );

/* The LCD task. */
static void vLcdTask( void *pvParameters );

xSemaphoreHandle xLcdSemphr;
xQueueHandle xFromSensorsQ;
TimerHandle_t xTimerSavePreset;
TimerHandle_t xTimerSavedNotion;
xQueueHandle xCmdLCDQ;
Command GLOBAL_COMMAND;
int FIRE;

/*
	PresetSavedTimeout()
	- Description: Checks whether the preset buttons are held
	for default time (2s).
	- Parameters: xTimerSavePreset - Preset saving timer
*/
void PresetSavedTimeout(TimerHandle_t xTimerSavePreset) {
	xTimerReset(xTimerSavePreset, 0);
	xTimerStop(xTimerSavePreset, 0);
	drawStatusBar(1);
	xQueueSendToBack(xCmdLCDQ, &GLOBAL_COMMAND, 0);
	xTimerStart(xTimerSavedNotion, 0);
}

/*
	RemoveIndicator()
	- Description: Removes the CONFIGURATION SAVED indicator
	- Parameters: xTimerSavedNotion - Timer for how long
	CONFIGURATION SAVED message should show. Default (3s).
*/
void RemoveIndicator(TimerHandle_t xTimerSavedNotion) {
	drawStatusBar(0);
	xTimerReset(xTimerSavedNotion, 0);
	xTimerStop(xTimerSavedNotion, 0);
}

/*
	vStartLcd()
	- Description: LCD task start
	- Parameters: uxPriority - Priority
								xQueue - LCD queue
								xSensorsQueue - Sensors queue
								xLSSemphr - LCD semaphore
*/
void vStartLcd( unsigned portBASE_TYPE uxPriority, xQueueHandle xFromUIQ, xQueueHandle xToLCDQueue, xSemaphoreHandle xLSemphr ){
	static xQueueHandle xCmdQ;
	xCmdQ = xFromUIQ;
	xLcdSemphr = xLSemphr;
	xFromSensorsQ = xToLCDQueue;
	
	/* Spawn the console task. */
	xTaskCreate( vLcdTask, "Lcd", lcdSTACK_SIZE, &xCmdQ, uxPriority, ( xTaskHandle * ) NULL );
	
	xTimerSavePreset = xTimerCreate("TimerSavePreset", 2000, pdFALSE, (void *) 0, PresetSavedTimeout);
	xTimerSavedNotion = xTimerCreate("TimerSaved", 3000, pdFALSE, (void *) 0, RemoveIndicator);
}

/*
	portTASK_FUNCTION()
	- Description: Main for LCD file
	- Parameters: vLcdTask - LCD task
								pvParameters - Various parameters
*/					
static portTASK_FUNCTION( vLcdTask, pvParameters ) {
	unsigned int pressure;
	unsigned int xPos;
	unsigned int yPos;
	portTickType xLastWakeTime;
	int clapState;
	
	Command cmd;
	Command receiveCMD;
	
	xCmdLCDQ = * ( ( xQueueHandle * ) pvParameters );
	FIRE = 0;

	/* Just to stop compiler warnings. */
	( void ) pvParameters;

	/* Initialise LCD display */
	/* NOTE: We needed to delay calling lcd_init() until here because it uses
	 * xTaskDelay to implement a delay and, as a result, can only be called from
	 * a task */
	lcd_init();
	initial(xCmdLCDQ);

	/* Infinite loop blocks waiting for a touch screen interrupt event from
	the queue.*/
	for( ;; )
	{
		Command presetCommand;
		
		/* Clear TS interrupts (EINT3) */
		/* Reset and (re-)enable TS interrupts on EINT3 */
		EXTINT = 8;						/* Reset EINT3 */

		/* Enable TS interrupt vector (VIC) (vector 17) */
		VICIntEnable = 1 << 17;			/* Enable interrupts on vector 17 */
		
		/* Block on a queue waiting for an event from the TS interrupt handler */
		xSemaphoreTake(xLcdSemphr, portMAX_DELAY);
		
		// Receive commands from sensors queue
		xQueueReceive(xFromSensorsQ, &receiveCMD, 0);
		/*
			Button pressed # cases
			3 - Forces shutdown
			4 - Forces reflection of state
			5 - Alternating FIRE1 state
			6 - Alternating FIRE2 state
			7 - Turn the fire off
		*/
		switch(receiveCMD.action) {
			case 3:
				forceShutdown();
				break;
			case 4:
				reflectState(receiveCMD.state);
				break;
			case 5:
				drawButtons(1);
				FIRE = 1;
				break;
			case 6:
				drawButtons(2);
				FIRE = 1;
				break;
			case 7:
				FIRE = 0;
				break;
			case 8:
				clapState = getLEDState(receiveCMD.identifier);
				cmd.value = clapState;
				xQueueSendToBack(xCmdLCDQ, &cmd, portMAX_DELAY);
				break;
			default:
				break;
		}
		
		/* Disable TS interrupt vector (VIC) (vector 17) */
		VICIntEnClr = 1 << 17;

		/* Measure next sleep interval from this point */
		xLastWakeTime = xTaskGetTickCount();
		
		/* Start polling the touchscreen pressure and position ( getTouch(...) ) */
		/* Keep polling until pressure == 0 */
		getTouch(&xPos, &yPos, &pressure);
		
		/* 
			- Draws different status bar depending on FIRE state
			- Checks UI for user input, sends to sensor queue accordingly.
		*/
		if (FIRE == 0) {
			cmd = checkPressed(xPos, yPos);
			if ((cmd.action >= 0) && (cmd.action < 10))
				xQueueSendToBack(xCmdLCDQ, &cmd, 0);
			cmd = checkSliderButton(xPos, yPos);
			if ((cmd.action >= 0) && (cmd.action < 10))
				xQueueSendToBack(xCmdLCDQ, &cmd, 0);
			presetCommand = checkPresets(xPos, yPos);
			drawStatusBar(0);
		} else {
			drawStatusBar(2);
		}
		
		// Checking whether FIRE state, draw buttons accordingly
		if ((receiveCMD.action != 5) && (receiveCMD.action != 6) && (FIRE == 0)) {
			drawButtons(0);
		}
		drawSlider();
		while (pressure > 0)
		{
			/* Get current pressure */
			getTouch(&xPos, &yPos, &pressure);
			/* While pressure is pressed, checks whether the button is HELD for default (2s) */
			if ((presetCommand.action == 2) && (xTimerIsTimerActive(xTimerSavePreset) == pdFALSE)) {
				GLOBAL_COMMAND = presetCommand;
				xTimerStart(xTimerSavePreset, 0);
			}
			/* Delay to give us a 25ms periodic TS pressure sample */
			vTaskDelayUntil( &xLastWakeTime, 25 );
			
		}
		xTimerReset(xTimerSavePreset, 0);
		xTimerStop(xTimerSavePreset, 0);
		/* +++ This point in the code can be interpreted as a screen button release event +++ */
		presetReset();
		
		// Re-draws buttons - noticeable UX effect when preset buttons are not re-drawn
		if ((receiveCMD.action != 5) && (receiveCMD.action != 6) && (FIRE == 0)) {
			drawButtons(0);
		}
		
		cmd.action = -1;
		receiveCMD.action = -1;
		receiveCMD.state = 0x00;
		xPos = 0;
		yPos = 0;
	}
}

/*
	vLCD_ISRHandler()
	- Description: TS event handler
	- Parameters: N/A
*/
void vLCD_ISRHandler( void ) {
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	/* Process the touchscreen interrupt */
	/* We would want to indicate to the task above that an event has occurred */
	xSemaphoreGiveFromISR(xLcdSemphr, &xHigherPriorityTaskWoken);

	EXTINT = 8;					/* Reset EINT3 */
	VICVectAddr = 0;			/* Clear VIC interrupt */

	/* Exit the ISR.  If a task was woken by either a character being received
	or transmitted then a context switch will occur. */
	portEXIT_SWITCHING_ISR( xHigherPriorityTaskWoken );
}
