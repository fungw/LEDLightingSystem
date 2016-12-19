#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "lpc24xx.h"
#include "commands.h"


/* FreeRTOS definitions */
#define consumerTaskSTACK_SIZE			( ( unsigned portBASE_TYPE ) 256 )


/* Hardware definitions */
#define P210BIT			( ( unsigned long ) 0x0004 )


/* The Consumer task. */
static void vConsumerTask( void *pvParameters );


void vStartConsumerTask( unsigned portBASE_TYPE uxPriority, xQueueHandle xQueue )
{
    /* We're going to pass a pointer to the new task's parameter block. We don't want the
       parameter block (in this case just a queue handle) that we point to to disappear
       during the lifetime of the task. Declaring the parameter block as static
       achieves this but only works here because we ony ever create one cunsumer.
       (We should probably add some application logic to prevent multiple,
       accidental nitialisations of the consumer task.) */
    static xQueueHandle xCmdQ;

    xCmdQ = xQueue;

	/* Spawn the console task . */
	xTaskCreate( vConsumerTask, "Consumer", consumerTaskSTACK_SIZE, &xCmdQ, uxPriority, ( xTaskHandle * ) NULL );
}


static portTASK_FUNCTION( vConsumerTask, pvParameters )
{
    xQueueHandle xCmdQ;
    Command cmd;

    /* pvParameters is actually a pointer to an xQueueHandle. Cast it and then
       dereference it to save it for later use. */
    xCmdQ = * ( ( xQueueHandle * ) pvParameters );

	while(1)
	{
	    /* Get command from Q */
	    xQueueReceive(xCmdQ, &cmd, portMAX_DELAY);

	    /* Execute command */
	    switch (cmd.value)
	    {
	        case LED_ON:
                FIO2SET1 = P210BIT;
                break;

	        case LED_OFF:
                FIO2CLR1 = P210BIT;
                break;
	    }
	}
}
