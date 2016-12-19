#include <stdlib.h>
#include <LPC24xx.h>

#define P210BIT ( ( unsigned long ) 0x4 )

static void vTimerTask( void *pvParameters );

static portTASK_FUNCTION( vSensorsTask, pvParameters ) {
	
}

int main(void)
{
	unsigned long currentState;

	/* Configure LED GPIO pin (P2.10) */
	PINSEL4 &= ~(3<<20);
	FIO2DIR1 = P210BIT;

	/* Configure TIMER0 to count for 1 second */
	T0TCR = 0x2;		/* Stop and reset TIMER0 */
	T0CTCR = 0x0;		/* Timer mode */
	T0MR0 = 6000000;	/* Match every 1 seconds */
	T0MCR = 0x4;		/* Stop on Match */
	T0PR = 0x0;			/* Prescale = 1 */
	T0TCR = 0x1;		/* Start TIMER0 */

	while (1)
	{
		/* Poll TCR waiting for TIMER0 to stop */
		while (T0TCR == 0x1);

		/* Invert LED state */
		currentState = (FIO2PIN1 & P210BIT);
		if (currentState)
		{
			FIO2CLR1 = P210BIT;
		}
		else
		{
			FIO2SET1 = P210BIT;
		}

		/* Reset and restart TIMER0 */
		T0TCR = 0x2;		/* Stop and reset TIMER0 */
		T0TCR = 0x1;		/* Start TIMER0 */
	}
}
