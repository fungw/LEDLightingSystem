#ifndef PRODUCERTASK_H
#define PRODUCERTASK_H

#include "queue.h"

void vStartProducerTask( unsigned portBASE_TYPE uxPriority, xQueueHandle xCmdQueue, unsigned int period);

#endif /* PRODUCERTASK_H */
