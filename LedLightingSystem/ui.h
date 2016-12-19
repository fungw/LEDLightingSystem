#ifndef LCD_UI
#define LCD_UI
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

void drawStatusBar(int indicator);
void drawButtons(int indicator);
void initial(xQueueHandle xLCDQueue);
Command checkPressed(int x, int y);
Command checkSlider(int x, int y);
void drawSlider(void);
void snapSlider(int n, int indicator);
Command checkSliderButton(int x, int y);
void forceShutdown(void);
void fire(int inverse);
Command checkPresets(int x, int y);
void presetReset(void);
void reflectState(unsigned char state);
int getLEDState(int n);

#endif
