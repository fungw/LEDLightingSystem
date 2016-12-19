#ifndef LCD_H
#define LCD_H

void vStartLcd( unsigned portBASE_TYPE uxPriority, xQueueHandle xQueue, xQueueHandle xLCDQueue, xSemaphoreHandle xLcdSemphr );

#endif
