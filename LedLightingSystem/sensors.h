#ifndef SENSORS_H
#define SENSORS_H

void vStartSensors( unsigned portBASE_TYPE uxPriority, xQueueHandle xQueue, xQueueHandle xLCDQueue, xSemaphoreHandle xLcdSemphr );

#endif
