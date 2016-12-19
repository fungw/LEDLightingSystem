/* 
	Sample task that initialises the EA QVGA LCD display
	with touch screen controller and processes touch screen
	interrupt events.

	Jonathan Dukes (jdukes@scss.tcd.ie)
	Wesley Fung (fungw@tcd.ie)
*/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "lpc24xx.h"
#include <stdio.h>
#include <string.h>
#include "commands.h"
#include "sensors.h"

#define I2C_AA      0x00000004
#define I2C_SI      0x00000008
#define I2C_STO     0x00000010
#define I2C_STA     0x00000020
#define I2C_I2EN    0x00000040

#define P210BIT ( ( unsigned long ) 0x4 )

/* Maximum task stack size */
#define sensorsSTACK_SIZE			( ( unsigned portBASE_TYPE ) 256 )
	
TimerHandle_t xTimerMotion;
TimerHandle_t xTimerFire;
TimerHandle_t xTimerClapClap;
TimerHandle_t xTimerStartMotion;

xQueueHandle xToLCDQ;
xQueueHandle xCmdQ;
xSemaphoreHandle xSensorSemphr;
unsigned char STATE_P1 = 0x8F; // Default state for preset 1
unsigned char STATE_P2 = 0x0B; // Default state for preset 2
unsigned char STATE_FIRE1 = 0x44; // Alternating LED state 1
unsigned char STATE_FIRE2 = 0x11; // Alternating LED state 2
unsigned char PWM0 = 0xBF; // Set PWM0 duty cycle to 75%	(255 * 75%)
unsigned char PWM1 = 0x40; // Set PWM1 duty cycle to 25%	(255 * 25%)
int FIRE_STATE = 0;
int ON_FIRE = 0;
unsigned char SHUTDOWN_STATE = 0x55;
int clapClap = 0;
int clapID;
int PRESSED;
int HELD = 1;
unsigned char TIMEOUT_STATE_CALLBACK = 0xFF;
int BUTTON_PRESSED;
unsigned char CLAP_STATE;
// Do not have same key codes for both.
// Repeated individual keys OK.
// Key codes correspond to button LED numbers, starting offset 0
int FIRE_ACTIVATE_CODE[4] = { 3, 2, 1, 1 };
int FIRE_DEACTIVATE_CODE[4] = { 2, 1, 4, 1};
int FIRE_SAME_CODE = 1;

/* The LCD task. */
static void vSensorsTask( void *pvParameters );

/* 
	PIRTimeout()
	- Description: Called when default time (30s) expires.
	Timer resets. Pushes command onto corresponding 
	queues of the LCD and I2C LEDs to signify a shutdown.
	- Parameters: xTimerMotion - Motion Timer
*/
void PIRTimeout( TimerHandle_t xTimerMotion ) {
	Command cmdTimeout, cmd;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
	xTimerReset(xTimerMotion, 0);
	xTimerStop(xTimerMotion, 0);
	
	cmdTimeout.action = 3;
	cmd.action = 0;
	cmd.value = 0;
	cmd.identifier = 6;
	
	xQueueSendToBack(xCmdQ, &cmd, 0);
	xQueueSendToBack(xToLCDQ, &cmdTimeout, 0);

	// Semphore blocks for TS event otherwise
	xSemaphoreGiveFromISR(xSensorSemphr, &xHigherPriorityTaskWoken);
}

/*
	forceState()
	- Description: Forces the state onto the sensors and pushes
	command on queue to LCD to reflect the changes on the UI.
	Function made for simplicity of fire alarm implmentation.
	Also used for force shutdown stages and clap feature. 
	Also able to retain the state as callbacks from timers
  do not allow parameters or returns.
	- Parameters: state - state to force the system into
								fire - boolean to indicate whether fire alarm
*/
unsigned char forceState(unsigned char state, int fire) {
	Command forceCMD;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
	I20CONCLR = I2C_AA | I2C_SI | I2C_STA | I2C_STO;
	I20CONSET = I2C_STA;
	
	while (!(I20CONSET & I2C_SI));
	
	I20DAT = 0xC0;
	I20CONCLR = I2C_SI | I2C_STA;
	
	while (!(I20CONSET & I2C_SI));
	
	I20DAT = 0x08;
	I20CONCLR = I2C_SI;
	
	while(!(I20CONSET & I2C_SI));
	
	I20DAT    =  state;
	I20CONCLR =  I2C_SI | I2C_STA;

	while (!(I20CONSET & I2C_SI));

	if (fire == 0) {
		forceCMD.action = 4;
		forceCMD.state = state;
		xQueueSendToBack(xToLCDQ, &forceCMD, 0);
		xSemaphoreGiveFromISR(xSensorSemphr, &xHigherPriorityTaskWoken);
	}
	TIMEOUT_STATE_CALLBACK = state;
	return state;
}

/*
	StartMotion()
	- Description: Callback for xTimerStartMotion, checks
	whether a button has been held or not. Needs to be held for
	at least default time (1s).
	- Parameters: xTimerStartMotion - Timer to check whether
	buttons are held or not
*/
void StartMotion (TimerHandle_t xTimerStartMotion) {
	if (HELD != -1) {
		TIMEOUT_STATE_CALLBACK = forceState(SHUTDOWN_STATE, 0); // HELD
	}
	HELD = 1;
	PRESSED = 0;
}

/*
	FireTimeout()
	- Description: Timeout evoked when fire alarm alternating
	state expires. Expires on default time (1s). Also forces
	alternating state to be reflected on UI. Resets timer
	on itself until fire alarm is turned off.
	- Parameters: xTimerFire - Timer for alternating state
	of fire alarm.
*/
void FireTimeout ( TimerHandle_t xTimerFire ) {
	Command fireCMD;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	fireCMD.action = (FIRE_STATE == 0)?5:6;
	xQueueSendToBack(xToLCDQ, &fireCMD, 0);
	xSemaphoreGiveFromISR(xSensorSemphr, &xHigherPriorityTaskWoken);

	forceState(FIRE_STATE?STATE_FIRE1:STATE_FIRE2, 1);
	FIRE_STATE = (FIRE_STATE == 0)?1:0;
	xTimerReset(xTimerFire, 0);
}

/*
	vStartSensors()
	- Description: Configurations settings for file. Queues
	are passed into here from main.c
	- Parameters: uxPriority - Priorty status
								xQueue - Queue for sensors.c
								xLCDQueue - Queue for LCD.c
								xSemphr - Sempahore for LCD
*/
void vStartSensors ( unsigned portBASE_TYPE uxPriority, xQueueHandle xFromUIQ, xQueueHandle xToLCDQueue, xSemaphoreHandle xSemphr){
	xCmdQ = xFromUIQ;
	xSensorSemphr = xSemphr;
	xToLCDQ = xToLCDQueue;

	/* Enable and configure I2C0 */
	PCONP    |=  (1 << 7);                /* Enable power for I2C0              */

	/* Initialize pins for SDA (P0.27) and SCL (P0.28) functions                */
	PINSEL1  &= ~0x03C00000;
	PINSEL1  |=  0x01400000;

	/* Clear I2C state machine                                                  */
	I20CONCLR =  I2C_AA | I2C_SI | I2C_STA | I2C_I2EN;
	
	/* Setup I2C clock speed                                                    */
	I20SCLL   =  0x80;
	I20SCLH   =  0x80;
	
	I20CONSET =  I2C_I2EN;

	/* Spawn the console task . */
	xTaskCreate( vSensorsTask, "Sensors", sensorsSTACK_SIZE, &xCmdQ, uxPriority, ( xTaskHandle * ) NULL );

	printf("Sensor task started ...\r\n");
}

/*
	getButtons()
	- Description: Gets the state of I2C buttons
	- Parameters: N/A
*/
unsigned char getButtons( ) {
	unsigned char ledData;

	/* Initialise */
	I20CONCLR =  I2C_AA | I2C_SI | I2C_STA | I2C_STO;
	
	/* Request send START */
	I20CONSET =  I2C_STA;

	/* Wait for START to be sent */
	while (!(I20CONSET & I2C_SI));

	/* Request send PCA9532 ADDRESS and R/W bit and clear SI */		
	I20DAT    =  0xC0;
	I20CONCLR =  I2C_SI | I2C_STA;

	/* Wait for ADDRESS and R/W to be sent */
	while (!(I20CONSET & I2C_SI));

	/* Send control word to read PCA9532 INPUT0 register */
	I20DAT = 0x00;
	I20CONCLR =  I2C_SI;

	/* Wait for DATA with control word to be sent */
	while (!(I20CONSET & I2C_SI));

	/* Request send repeated START */
	I20CONSET =  I2C_STA;
	I20CONCLR =  I2C_SI;

	/* Wait for START to be sent */
	while (!(I20CONSET & I2C_SI));

	/* Request send PCA9532 ADDRESS and R/W bit and clear SI */		
	I20DAT    =  0xC1;
	I20CONCLR =  I2C_SI | I2C_STA;

	/* Wait for ADDRESS and R/W to be sent */
	while (!(I20CONSET & I2C_SI));

	I20CONCLR = I2C_SI;

	/* Wait for DATA to be received */
	while (!(I20CONSET & I2C_SI));

	ledData = I20DAT;

	/* Request send NAQ and STOP */
	I20CONSET =  I2C_STO;
	I20CONCLR =  I2C_SI | I2C_AA;

	/* Wait for STOP to be sent */
	while (I20CONSET & I2C_STO);

	return ledData ^ 0xf;
}

/*
	ledBinaryChange()
	- Description: Changes the I2C LEDs ON/OFF
	- Parameters: value - ON/OFF (0/1)
								id - ID of I2D LED
								state - Previous state
*/
unsigned char ledBinaryChange (int value, int id, int state) {
	SHUTDOWN_STATE = state;
	
	/* CLEAR bit */
	state &= ~(3 << id * 2);
	
	switch (value) {
		case 0:
			state &=  ~(3  << id * 2);
			state = ((id == 5) || (id == 6)) ? 0x00 : state;
			break;
		case 1:
			state |= value << id * 2;
			state = ((id == 5) || (id == 6)) ? 0x55 : state;
			break;
	}
	
	I20CONCLR = I2C_AA | I2C_SI | I2C_STA | I2C_STO;
	I20CONSET = I2C_STA;
	
	while (!(I20CONSET & I2C_SI));
	
	I20DAT = 0xC0;
	I20CONCLR = I2C_SI | I2C_STA;
	
	while (!(I20CONSET & I2C_SI));
	
	I20DAT = 0x08;
	I20CONCLR = I2C_SI;
	
	while(!(I20CONSET & I2C_SI));
	
	I20DAT    =  state;
	I20CONCLR =  I2C_SI | I2C_STA;

	while (!(I20CONSET & I2C_SI));
	
	if (state == 0x00) {
		xTimerReset(xTimerMotion, 0);
		xTimerStop(xTimerMotion, 0);
	} else {
		xTimerStart(xTimerMotion, 0);
	}
	
	CLAP_STATE = state;
	return state;
}

/*
	ledDim()
	- Description: Turns I2C LED to one of two dim
	states.
	- Parameters: value - PWM0 / PWM1 (2/4)
							  id - ID of I2C LED
								state - Previous state
*/
unsigned char ledDim (int value, int id, int state) {
	int pwm_picker;
	
	/* CLEAR bit */
	state &= ~(3 << id * 2);
	
	switch (value) {
		case 2:
			pwm_picker = 3;
			state |= (pwm_picker << id * 2);
			break;
		case 4:
			pwm_picker = 2;
			state |= (pwm_picker << id * 2);
			break;
	}
	
	I20CONCLR = I2C_AA | I2C_SI | I2C_STA | I2C_STO;
	I20CONSET = I2C_STA;
	
	while (!(I20CONSET & I2C_SI));
	
	// Selects the address of I2C device; write implied
	I20DAT = 0xC0;
	I20CONCLR = I2C_SI | I2C_STA;
	
	while (!(I20CONSET & I2C_SI));
	
	// Selects the LED register
	I20DAT = 0x08;
	I20CONCLR = I2C_SI;
	
	while(!(I20CONSET & I2C_SI));
	
	// Set LED registers to whatever
	I20DAT = state;
	I20CONCLR = I2C_SI | I2C_STA;
	
	while (!(I20CONSET & I2C_SI));
	
	xTimerStart(xTimerMotion, 0);
	
	return state;
}

/*
	setDefaultPWM()
	- Description: Sets variables PWM0 and PWM1 variables to
	corresponding registers.
	- Parameters: N/A
*/
void setDefaultPWM (void) {
	/* Initialise */
	I20CONCLR = I2C_AA | I2C_SI | I2C_STA | I2C_STO;
	
	/* Request send START */
	I20CONSET = I2C_STA;
	
	/* Wait for START to be sent */
	while (!(I20CONSET & I2C_SI));
	
	// Selects the address of I2C device; write implied
	I20DAT = 0xC0;
	I20CONCLR = I2C_SI | I2C_STA;
	
	while (!(I20CONSET & I2C_SI));
	
	// Selects the PWM0 register
	I20DAT = 0x03;
	I20CONCLR = I2C_SI;
	
	while(!(I20CONSET & I2C_SI));
	
	I20DAT    =  PWM0;
	I20CONCLR =  I2C_SI | I2C_STA;

	while (!(I20CONSET & I2C_SI));
	
	/* Request send repeated START */
	I20CONSET =  I2C_STA;
	I20CONCLR =  I2C_SI;

	/* Wait for START to be sent */
	while (!(I20CONSET & I2C_SI));
	
	// Selects the PWM1 register
	I20DAT = 0x05;
	I20CONCLR = I2C_SI;
	
	while(!(I20CONSET & I2C_SI));
	
	I20DAT    =  PWM1;
	I20CONCLR =  I2C_SI | I2C_STA;

	while (!(I20CONSET & I2C_SI));
}

/*
	ClapTimeout()
	- Description: When double click is detected on I2C
	buttons, turns the corresponding light the I2C LED
	is on to ON, OFF otherwise
	- Parameters: xTimerClapClap - Clap clap timer
*/
void ClapTimeout (TimerHandle_t xTimerClapClap) {
	Command cmdUI, cmd;
	int switchState;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
	xTimerReset(xTimerClapClap, 0);
	xTimerStop(xTimerClapClap, 0);
	
	if ((clapClap == 4) && (ON_FIRE == 0)) {
		// Get state of individual LED from LCD
		cmdUI.action = 8;
		cmdUI.identifier = BUTTON_PRESSED;
		xQueueSendToBack(xToLCDQ, &cmdUI, 0);
		xSemaphoreGiveFromISR(xSensorSemphr, &xHigherPriorityTaskWoken);
		xQueueReceive(xCmdQ, &cmdUI, portMAX_DELAY);
		switchState = cmdUI.value;
		switchState = (switchState == 0)?1:0; // Switch state
		
		// Send event to LCD Queue
		cmdUI.action = 4;
		cmdUI.identifier = BUTTON_PRESSED;
		cmdUI.state = CLAP_STATE;
		cmdUI.state &= ~(3 << BUTTON_PRESSED * 2);
		if (switchState == 1)
			cmdUI.state |= 1 << BUTTON_PRESSED * 2;
		else
			cmdUI.state &=  ~(3  << BUTTON_PRESSED * 2);
		xQueueSendToBack(xToLCDQ, &cmdUI, 0);
	  xSemaphoreGiveFromISR(xSensorSemphr, &xHigherPriorityTaskWoken);
		
		// Send event to Sensors Queue
		cmd.action = 0;
		cmd.value = switchState;
		cmd.identifier = BUTTON_PRESSED;
		xQueueSendToBack(xCmdQ, &cmd, 0);
	}
	TIMEOUT_STATE_CALLBACK = CLAP_STATE;
	clapClap = 0;
	clapID = 0;
}

/*
	fireAlarmInteraction()
	- Description: Fire alarm actuator
	- Parameters: fire - to turn FIRE alarm ON/OFF
*/
unsigned char fireAlarmInteraction (int fire) {
	unsigned char state;
	Command cmd;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	ON_FIRE = fire;
	if (fire == 1) {
		TIMEOUT_STATE_CALLBACK = forceState(FIRE_STATE?STATE_FIRE1:STATE_FIRE2, 1);
		FIRE_STATE = (FIRE_STATE == 0)?1:0;
		xTimerStart(xTimerFire, 0);
		cmd.action = 3;
		xQueueSendToBack(xToLCDQ, &cmd, 0);
		xSemaphoreGiveFromISR(xSensorSemphr, &xHigherPriorityTaskWoken);
		xTimerReset(xTimerMotion, 0);
		xTimerStop(xTimerMotion, 0);
	} else if (fire == 0) {
		cmd.action = 7;
		xQueueReset(xToLCDQ); // Fixes unresponsive UI bug
		xQueueSendToBack(xToLCDQ, &cmd, 0);
		xSemaphoreGiveFromISR(xSensorSemphr, &xHigherPriorityTaskWoken);
		xTimerReset(xTimerFire, 0);
		xTimerStop(xTimerFire, 0);
		TIMEOUT_STATE_CALLBACK = forceState(0x00, 0);
	}
	return state;
}

/*
	portTASK_FUNCTION()
	- Description: main sensors.c
	- Parameters: vSensorsTask - Task
								pvParameters - Misc parameters
*/
static portTASK_FUNCTION( vSensorsTask, pvParameters ){
	portTickType xLastWakeTime;
	unsigned char buttonState;
	unsigned char lastButtonState;
	unsigned char changedState;
	unsigned int i;
	unsigned char mask;
	unsigned char state = 0x00;
	xQueueHandle xCmdQ;
	Command cmd;
	int combination_activate = 0;
	int combination_deactivate = 0;
	int alternatingBit = 0;
	
	/* Start all the timers */
	xTimerMotion = xTimerCreate("TimerMotion", 30000, pdFALSE, (void *) 0, PIRTimeout);
	xTimerFire = xTimerCreate("TimerFire", 1000, pdFALSE, (void *) 0, FireTimeout);
	xTimerClapClap = xTimerCreate("TimerClap", 500, pdFALSE, (void *) 0, ClapTimeout);
	xTimerStartMotion = xTimerCreate("TimerStartMotion", 1000, pdFALSE, (void *) 0, StartMotion);
	
	/* Sensors queue */
	xCmdQ = * ( ( xQueueHandle * ) pvParameters );

	/* Just to stop compiler warnings. */
	( void ) pvParameters;

	/* initialise lastState with all buttons off */
	lastButtonState = 0;

	/* initial xLastWakeTime for accurate polling interval */
	xLastWakeTime = xTaskGetTickCount();
	
	/* Set default values for the PWM's */
	setDefaultPWM();
	
	while( 1 )
	{
		/* 
			State before time out retained through every iteration.
			0xFF is used as no timeout retainable state will force it to 0xFF
		*/
		if (TIMEOUT_STATE_CALLBACK != 0xFF) {
			state = TIMEOUT_STATE_CALLBACK;
			CLAP_STATE = TIMEOUT_STATE_CALLBACK;
			TIMEOUT_STATE_CALLBACK = 0xFF;
		}
		xQueueReceive(xCmdQ, &cmd, 0);
		/*
			0 - LED TURN ON/OFF
			1 - LED TURN DIM1/DIM2
			2 - Preset save
			3 - Alternating fire state
		*/
		switch (cmd.action) {
			case 0: 
				state = ledBinaryChange(cmd.value, cmd.identifier, state);
				break;
			case 1:
				state = ledDim(cmd.dimValue, cmd.identifier, state);
				break;
			case 2:
				if (cmd.value == 0) {
					STATE_P1 = state;
				}
				else {
					STATE_P2 = state;
				}
				break;
			case 3:
				if (ON_FIRE == 0)
					state = forceState((cmd.value == 0)?STATE_P1:STATE_P2, 0);
				break;
			default:
				break;
		}
		cmd.action = -1;
		
		/* Read buttons */
		buttonState = getButtons();
		changedState = buttonState ^ lastButtonState;
		if (buttonState != lastButtonState) {
		  /* iterate over each of the 4 LS bits looking for changes in state */
			for (i = 0; i <= 3; i = i + 1) {
				mask = 1 << i;
        if (changedState & mask) {
					PRESSED = (PRESSED == 0) ? 1 : 0;
					xTimerReset(xTimerMotion, 0); // Resets motion (keeps on)
					// HELD button status check
					if ((state == 0x00) && (ON_FIRE == 0)) {
						xTimerStart( xTimerStartMotion, 0);
						HELD = (HELD == PRESSED)?PRESSED:-1;
					}
					// CLAP button status check
					if (clapClap == 0) {
						xTimerStart( xTimerClapClap, 0 );
						clapID = i;
						clapClap++;
						BUTTON_PRESSED = i;
						CLAP_STATE = state;
					} else if (i == clapID) {
						clapClap++;
					}
					
					// Combinational FIRE alarm deployment
					// PUSH-BUTTON command <variable> KEY_CODES GLOBAL to activate/deactivate
					if (alternatingBit == 0) {
						// Check activate codes
						if (i == FIRE_ACTIVATE_CODE[combination_activate] - 1) {
							combination_activate++;
						} else if (i == FIRE_ACTIVATE_CODE[0]) {
							combination_activate = 1;
						} else {
							combination_activate = 0;
						} if (combination_activate == 4) {
							if (ON_FIRE == 0)
								state = fireAlarmInteraction((ON_FIRE==0)?1:ON_FIRE);
							combination_activate = 0;
						}
						// Check deactivate codes
						if (i == FIRE_DEACTIVATE_CODE[combination_deactivate] - 1) {
							combination_deactivate++;
						} else if (i == FIRE_DEACTIVATE_CODE[0]) {
							combination_deactivate = 1;
						} else {
							combination_deactivate = 0;
						} if (combination_deactivate == 4) {
							if (ON_FIRE == 1)
								state = fireAlarmInteraction((ON_FIRE==1)?0:ON_FIRE);
							combination_deactivate = 0;
						}
					}
					alternatingBit = (alternatingBit == 0)?1:0;
        }
		  }
			/* remember new state */
			lastButtonState = buttonState;
		}
    /* delay before next poll */
    vTaskDelayUntil( &xLastWakeTime, 20);
	}
}
