#ifndef CONSUMERTASK_H
#define CONSUMERTASK_H

#include "queue.h"

void vStartConsumerTask( unsigned portBASE_TYPE uxPriority, xQueueHandle xQueue );

#endif /* CONSUMERTASK_H */
