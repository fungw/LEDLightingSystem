#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "producerTask.h"
#include "commands.h"
#include <stdio.h>


/* FreeRTOS definitions */
#define producerTaskSTACK_SIZE			( ( unsigned portBASE_TYPE ) 256 )


/* Application definitions */
#define NPRODUCERS 2


/* Structure for passing arguments to producer task instances */
typedef struct {
	xQueueHandle cmdQ; /* P--->C command queue */
	unsigned int period; /* LED period */
	unsigned int id; /* producer id (not currently used) */
	/* ... anything else you want to pass to a producer */
} ProducerConfig;


/* We could have used malloc() to allocate memory for each producer as required
   but doing it this way illustrates static application supporting up to
   NPRODUCERS producers. We often want to avoid the unpredictable overhead of
   malloc in embedded systems */
static ProducerConfig producerConf[NPRODUCERS];


/* The Producer task. */
static void vProducerTask( void *pvParameters );


/* Function for creating and initialising producer tasks
   Note the tasks won't be started until the scheduler is started in main! */
void vStartProducerTask(unsigned portBASE_TYPE uxPriority, xQueueHandle xCmdQueue, unsigned int period)
{
	/* Declared as static so the value is maintained between invocations */
	static unsigned int nextProducerId = 0;

	/* A buffer to create the task name "Producern" at runtime */
	char producerName[10];

	/* Don't create more than NPRODUCERS producers */
	if (nextProducerId >= NPRODUCERS) {
		return;
	}

	/* Initialisation */
	sprintf(producerName, "Producer%1d", nextProducerId);
	producerConf[nextProducerId].cmdQ = xCmdQueue;
	producerConf[nextProducerId].period = period;
	producerConf[nextProducerId].id = nextProducerId;

	/* Spawn the producer task */
	xTaskCreate( vProducerTask, producerName, producerTaskSTACK_SIZE, &producerConf[nextProducerId], uxPriority, ( xTaskHandle * ) NULL );

	nextProducerId++;
}


static portTASK_FUNCTION( vProducerTask, pvParameters )
{
	ProducerConfig *pConfig;
	portTickType xLastWakeTime;
	Command cmd;

    /* pvParameters is actually a pointer to a ProducerConfig structure that
	   we created above. 'Cast' it and then dereference it to save it for later
	   use. */
	pConfig = ( ProducerConfig * ) pvParameters;

	/* Needed for precise periods - see FreeRTOS docs */
	xLastWakeTime = xTaskGetTickCount();

	while(1)
	{
		/* Delay for a second; modify command to invert the LED; send to Q */
		vTaskDelayUntil(&xLastWakeTime, pConfig->period);
		cmd.value = cmd.value == LED_ON ? LED_OFF : LED_ON;
		xQueueSendToBack(pConfig->cmdQ, &cmd, portMAX_DELAY);
	}
}
