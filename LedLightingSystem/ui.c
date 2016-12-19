/* 
	User Interface file for Lecture Theature Lights Control System

	Wesley Fung (fungw@tcd.ie)
*/
#include <stdlib.h>
#include <stdio.h>

#include "ui.h"
#include "lcd_grph.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "timers.h"

#define WidgetButtonCount 7 // Includes MASTER I/O button
#define SliderButtonCount 4
#define PresetButtonCount 2

int STATUS_TOP_OFFSET = 13;
int LEFT_MARGIN = 15;
int RIGHT_MARGIN = DISPLAY_WIDTH - 15;
int POWER = 1;

unsigned char STR_LIGHTS[18] = "LIGHTS CONTROLLER";
unsigned char STR_SAVED[20] = "CONFIGURATION SAVED";
unsigned char STR_WHITEBOARD[11] = "WHITEBOARD";
unsigned char STR_LECTURER[9] = "LECTURER";
unsigned char STR_SEATING[8] = "SEATING";
unsigned char EXIT[17] = "EXIT IMMEDIATELY";
unsigned char STR_P1[3] = "P1";
unsigned char STR_P2[3] = "P2";

// Three types of user inputs
typedef struct WidgetButton {
	int x0;
	int y0;
	int x1;
	int y1;
	int str_x;
	int str_y;
	int state;
	int color;
	int radius;
	int bounding_x0;
	int bounding_y0;
	int bounding_x1;
	int bounding_y1;
	int alterColor;
	int fire_color1;
	int fire_color2;
} WidgetButton;
typedef struct PresetButton {
	int x0;
	int y0;
	int x1;
	int y1;
	int str_x;
	int str_y;
	int color;
	int alterColor;
	int state;
} PresetButton;

typedef struct WidgetSlider {
	int rect1_x0;
	int rect1_y0;
	int rect1_x1;
	int rect1_y1;
	int rect1_color;
	int rect1_inactive_color;
	int rect2_x0;
	int rect2_y0;
	int rect2_x1;
	int rect2_y1;
	int rect2_color;
	int rect2_inactive_color;
	int circle_x0;
	int circle_y0;
	int radius;
	double circle_percent;
	int circle_color;
	int circle_inactive_color;
	int bounding_x0;
	int bounding_y0;
	int bounding_x1;
	int bounding_y1;
	int plus_x0;
	int plus_y0;
	int plus_x1;
	int plus_y1;
	int minus_x0;
	int minus_y0;
	int minus_x1;
	int minus_y1;
	int state;
	int snap_state;
} WidgetSlider;

struct WidgetButton button[WidgetButtonCount];
struct WidgetSlider slider[SliderButtonCount];
struct PresetButton preset[PresetButtonCount];
int counter;
int doubleClick;
int doubleClickID;
int aisleStrOffset;
char * aislePointer;

TimerHandle_t xTimerDoubleClick;
xQueueHandle xCmdUIQ;

/*
	resetCounter()
	- Description: Double click timer callback
	- Parameters: xTimerDoubleClick - Timer checking
								if double click within time period.
*/
void resetCounter (TimerHandle_t xTimerDoubleClick) {
	Command cmd;
	cmd.action = 3;
	cmd.value = doubleClickID;
	if (doubleClick == 2) {
		xQueueSendToBack(xCmdUIQ, &cmd, 0);
	}
	doubleClick = 0;
	doubleClickID = 0;
	xTimerReset(xTimerDoubleClick, 0);
	xTimerStop(xTimerDoubleClick, 0);
}

/*
	initial()
	- Description: Initial settings for button locations
	and other relevant states etc. LCD queue received 
	through here also.
	- Parameters: xCmdQ - LCD queue
*/
void initial(xQueueHandle xCmdQ) {
	xCmdUIQ = xCmdQ;
	xTimerDoubleClick = xTimerCreate("TimerDoubleClick", 750, pdFALSE, (void *) 0, resetCounter);
	lcd_fillScreen(BLACK);
	
	// Whiteboard
	button[0].x0 = DISPLAY_WIDTH /3;
	button[0].y0 = DISPLAY_HEIGHT - 25;
	button[0].x1 = DISPLAY_WIDTH * 2/3;
	button[0].y1 = DISPLAY_HEIGHT - 5;
	button[0].str_x = DISPLAY_WIDTH/2 - 28;
	button[0].str_y = DISPLAY_HEIGHT - 17;
	button[0].state = 0;
	button[0].color = WHITE;
	button[0].alterColor = GREEN;
	button[0].radius = 0;
	button[0].fire_color1 = RED;
	button[0].fire_color2 = BLUE;
	
	// Lecturer
	button[1].x0 = DISPLAY_WIDTH/4;
	button[1].y0 = DISPLAY_HEIGHT - 65;
	button[1].x1 = DISPLAY_WIDTH * 3/4;
	button[1].y1 = DISPLAY_HEIGHT - 35;
	button[1].str_x = DISPLAY_WIDTH/2 - 23;
	button[1].str_y = DISPLAY_HEIGHT - 53;
	button[1].state = 0;
	button[1].color = WHITE;
	button[1].alterColor = GREEN;
	button[1].radius = 0;
	button[1].fire_color1 = RED;
	button[1].fire_color2 = BLUE;
	
	// Seating
	button[2].x0 = DISPLAY_WIDTH/4;
	button[2].y0 = STATUS_TOP_OFFSET + 125;
	button[2].x1 = DISPLAY_WIDTH * 3/4;
	button[2].y1 = DISPLAY_HEIGHT - 70;
	button[2].str_x = DISPLAY_WIDTH/2 - 22;
	button[2].str_y = DISPLAY_HEIGHT - 125;
	button[2].state = 0;
	button[2].color = WHITE;
	button[2].alterColor = GREEN;
	button[2].radius = 0;
	button[2].fire_color1 = RED;
	button[2].fire_color2 = BLUE;
	
	// Aisle Left
	button[3].x0 = LEFT_MARGIN;
	button[3].y0 = STATUS_TOP_OFFSET + 125;
	button[3].x1 = LEFT_MARGIN + 30;
	button[3].y1 = DISPLAY_HEIGHT - 50;
	button[3].str_x = LEFT_MARGIN + 13;
	button[3].str_y = STATUS_TOP_OFFSET + 160;
	button[3].state = 0;
	button[3].color = WHITE;
	button[3].alterColor = GREEN;
	button[3].radius = 0;
	button[3].fire_color1 = RED;
	button[3].fire_color2 = BLUE;
	
	// Aisle Right
	button[4].x0 = RIGHT_MARGIN - 30;
	button[4].y0 = STATUS_TOP_OFFSET + 125;
	button[4].x1 = RIGHT_MARGIN;
	button[4].y1 = DISPLAY_HEIGHT - 50;
	button[4].str_x = RIGHT_MARGIN -17;
	button[4].str_y = STATUS_TOP_OFFSET + 160;
	button[4].state = 0;
	button[4].color = WHITE;
	button[4].alterColor = GREEN;
	button[4].radius = 0;
	button[4].fire_color1 = RED;
	button[4].fire_color2 = BLUE;
	
	// Master Power Button
	button[5].x0 = DISPLAY_WIDTH - 15;
	button[5].y0 = 4;
	button[5].x1 = DISPLAY_WIDTH - 13;
	button[5].y1 = 13;
	button[5].color = RED;
	button[5].alterColor = GREEN;
	button[5].radius = 0;
	button[5].state = 0;
	button[5].fire_color1 = RED;
	button[5].fire_color2 = BLUE;
	
	// Circles
	button[6].x0 = DISPLAY_WIDTH - 14;
	button[6].y0 = STATUS_TOP_OFFSET/2 + 6;
	button[6].color = RED;
	button[6].radius = 5;
	button[6].alterColor = GREEN;
	button[6].state = 0;
	button[6].bounding_x0 = DISPLAY_WIDTH - 30;
	button[6].bounding_y0 = 0;
	button[6].bounding_x1 = DISPLAY_WIDTH;
	button[6].bounding_y1 = 30;
	button[6].fire_color1 = RED;
	button[6].fire_color2 = BLUE;
	
	// Preset #1
	preset[0].x0 = 10;
	preset[0].y0 = DISPLAY_HEIGHT - 25;
	preset[0].x1 = 60;
	preset[0].y1 = DISPLAY_HEIGHT - 5;
	preset[0].str_x = 30;
	preset[0].str_y = DISPLAY_HEIGHT - 17;
	preset[0].color = DARK_GRAY;
	preset[0].alterColor = GREEN;
	preset[0].state = 0;

	// Preset #2
	preset[1].x0 = DISPLAY_WIDTH - 60;
	preset[1].y0 = DISPLAY_HEIGHT - 25;
	preset[1].x1 = DISPLAY_WIDTH - 10;
	preset[1].y1 = DISPLAY_HEIGHT - 5;
	preset[1].str_x = DISPLAY_WIDTH - 40;
	preset[1].str_y = DISPLAY_HEIGHT - 17;
	preset[1].color = DARK_GRAY;
	preset[1].alterColor = GREEN;
	preset[1].state = 0;

	// Sliders
	for (counter = 0; counter < SliderButtonCount; counter++) {
		slider[counter].rect1_x0 = LEFT_MARGIN;
		slider[counter].rect1_y0 = 45 + (counter * 25);
		slider[counter].rect1_x1 = DISPLAY_WIDTH/2;
		slider[counter].rect1_y1 = 47 + (counter * 25);
		slider[counter].rect1_color = BLUE;
		slider[counter].rect1_inactive_color = DARK_GRAY;
		slider[counter].rect2_x0 = DISPLAY_WIDTH/2;
		slider[counter].rect2_y0 = 45 + (counter * 25);
		slider[counter].rect2_x1 = RIGHT_MARGIN;
		slider[counter].rect2_y1 = 47 + (counter * 25);
		slider[counter].rect2_color = WHITE;
		slider[counter].rect2_inactive_color = LIGHT_GRAY;
		slider[counter].circle_x0 = DISPLAY_WIDTH/2;
		slider[counter].circle_y0 = 46 + (counter * 25);
		slider[counter].radius = 8;
		slider[counter].circle_percent = 50.0;
		slider[counter].circle_color = WHITE;
		slider[counter].circle_inactive_color = DARK_GRAY;
		slider[counter].state = 0;
		slider[counter].bounding_x0 = slider[counter].circle_x0 - 15;
		slider[counter].bounding_y0 = slider[counter].circle_y0 - 12;
		slider[counter].bounding_x1 = slider[counter].circle_x0 + 15;
		slider[counter].bounding_y1 = slider[counter].circle_y0 + 12;
		slider[counter].plus_x0 = slider[counter].rect2_x1 - 12;
		slider[counter].plus_y0 = slider[counter].rect2_y1 - 7;
		slider[counter].plus_x1 = slider[counter].rect2_x1;
		slider[counter].plus_y1 = slider[counter].rect2_y1 + 5;
		slider[counter].minus_x0 = slider[counter].rect1_x0;
		slider[counter].minus_y0 = slider[counter].rect1_y0 - 5;
		slider[counter].minus_x1 = slider[counter].rect1_x0 + 12;
		slider[counter].minus_y1 = slider[counter].rect1_y0 + 7;
		slider[counter].snap_state = 3;
	}
	
}

/*
	drawSlider()
	- Description: Draws the sliders 
	- Parameters: N/A
*/
void drawSlider() {
	int circle_counter;
	int draw_counter;
	for (draw_counter = 0; draw_counter < SliderButtonCount; draw_counter++) {
		// Minimise the flickering of the background as its being redrawn with rectangle in between as opposed redrawing entire section
		lcd_fillRect(slider[draw_counter].minus_x1, slider[draw_counter].rect1_y0 - 7, slider[draw_counter].plus_x0, slider[draw_counter].rect1_y1 - 2, BLACK);
		lcd_fillRect(slider[draw_counter].minus_x1, slider[draw_counter].rect2_y0 - 7, slider[draw_counter].plus_x0, slider[draw_counter].rect2_y1 - 2, BLACK);
		lcd_fillRect(slider[draw_counter].minus_x1, slider[draw_counter].rect1_y0 + 2, slider[draw_counter].plus_x0, slider[draw_counter].rect1_y1 + 7, BLACK);
		lcd_fillRect(slider[draw_counter].minus_x1, slider[draw_counter].rect2_y0 + 2, slider[draw_counter].plus_x0, slider[draw_counter].rect2_y1 + 7, BLACK);
		
		lcd_fillRect(slider[draw_counter].rect1_x0, slider[draw_counter].rect1_y0, slider[draw_counter].rect1_x1, slider[draw_counter].rect1_y1, (slider[draw_counter].state == 1) ? slider[draw_counter].rect1_color : slider[draw_counter].rect1_inactive_color);
		lcd_fillRect(slider[draw_counter].rect2_x0, slider[draw_counter].rect2_y0, slider[draw_counter].rect2_x1, slider[draw_counter].rect2_y1, (slider[draw_counter].state == 1) ? slider[draw_counter].rect2_color : slider[draw_counter].rect2_inactive_color);
		
		// PLUS signs
		lcd_fontColor(BLACK, (slider[draw_counter].state == 1) ? slider[draw_counter].rect2_color : slider[draw_counter].rect2_inactive_color);
		lcd_fillRect(slider[draw_counter].plus_x0, slider[draw_counter].plus_y0, slider[draw_counter].plus_x1, slider[draw_counter].plus_y1, (slider[draw_counter].state == 1) ? slider[draw_counter].rect2_color : slider[draw_counter].rect2_inactive_color);
		lcd_putChar(slider[draw_counter].plus_x0 + 4, slider[draw_counter].plus_y0 + 3, '+');
		// MINUS signs
		lcd_fontColor(BLACK, (slider[draw_counter].state == 1) ? DARK_GRAY : slider[draw_counter].rect1_inactive_color);
		lcd_fillRect(slider[draw_counter].minus_x0, slider[draw_counter].minus_y0, slider[draw_counter].minus_x1, slider[draw_counter].minus_y1, (slider[draw_counter].state == 1) ? DARK_GRAY : slider[draw_counter].rect1_inactive_color);
		lcd_putChar(slider[draw_counter].minus_x0 + 4, slider[draw_counter].minus_y0 + 3, '-');
		
		// Circles (fill)
		for (circle_counter = 0; circle_counter < slider[draw_counter].radius + 1; circle_counter++) {
			lcd_circle(slider[draw_counter].circle_x0, slider[draw_counter].circle_y0, circle_counter, (slider[draw_counter].state == 1) ? slider[draw_counter].circle_color : slider[draw_counter].circle_inactive_color);
		}
	}
}

/*
	generateCmd()
	- Description: Pre-clearance to generate commands from UI state at any one
	instance prior to sending commands to LCD queue
	- Parameters: n - Button pressed number
*/
Command generateCmd(int n){
	Command cmd;
	int action;
	int value;
	
	/* 
		Switch description (CASE 6 + 7 omitted as they are power buttons),
		when n==5 / n ==6 (master power button), forces to POWER variable 
		case which turns all lights ON or OFF.
		1 - OFF
		2 - ON + PWM0
		3 - OFF (INITIAL STATE) [When sliders are in the middle]
		4 - ON + PWM1
		5 - ON
		8 - SAVE PRESET 1
		9 - SAVE PRESET 2
	
		ACTION: 0 - OFF
						1 - ON
						2 - SET STATE
	
		POWER: Either 1 OR 5
	*/
	switch (((n==5)||(n==6))?POWER:((n==8)||(n==9))?n:slider[n].snap_state) {
		case 1:
			action = 0;
		  value = 0;
		  if ((n==3)||(n==4)) {
				button[3].state = 0;
				button[4].state = 0;
			} else {
				button[n].state = 0;
			}
			break;
		case 2:
			action = 1;
			value = 2;
			if ((n==3)||(n==4)) {
				button[3].state = 1;
				button[4].state = 1;
			} else {
				button[n].state = 1;
			}
			break;
		case 3: 
			action = 0;
			value = 0;
			if ((n==3)||(n==4)) {
				button[3].state = 0;
				button[4].state = 0;
			} else {
				button[n].state = 0;
			}
			break;
		case 4:
			action = 1;
	    value = 4;
			if ((n==3)||(n==4)) {
				button[3].state = 1;
				button[4].state = 1;
			} else {
				button[n].state = 1;
			}
			break;
		case 5: 
			action = 0;
		  value = 1;
			if ((n==3)||(n==4)) {
				button[3].state = 1;
				button[4].state = 1;
			} else {
				button[n].state = 1;
			}
			break;
		case 8:
			action = 2;
			value = 0;
			break;
		case 9:
			action = 2;
			value = 1;
			break;
		default:
			action = -1;
			value = -1;
			break;
	}
	cmd.value = value;
	cmd.dimValue = value;
	cmd.identifier = n;
	cmd.action = action;
	return cmd;
}

/*
	checkSliderButton()
	- Description: Checks if any of the PLUS or MINUS buttons are pressed
	- Parameters: x - x coordinate of touch event
								y - y coordinate of touch event
*/
Command checkSliderButton(int x, int y) {
	int buttonRegistered = -1;
	int offset = 1;
	for (counter = 0; counter < SliderButtonCount; counter++) {
		if ((x >= slider[counter].minus_x0 - 3) && (x <= slider[counter].minus_x1 + 3) && (y >= slider[counter].minus_y0 - 3) && (y <= slider[counter].minus_y1 + 3) && (slider[counter].state == 1) && (slider[counter].snap_state > 1)) {
			if (slider[counter].snap_state == 4) {
				offset = 2;
			}
			snapSlider(counter, slider[counter].snap_state - offset);
			buttonRegistered = counter;
		} else if ((x >= slider[counter].plus_x0 - 3) && (x <= slider[counter].plus_x1 + 3) && (y >= slider[counter].plus_y0 - 3) && (y <= slider[counter].plus_y1 + 3) && (slider[counter].state == 1) && (slider[counter].snap_state < 5)) {
			if (slider[counter].snap_state == 2) {
				offset = 2;
			}
			snapSlider(counter, slider[counter].snap_state + offset);
			buttonRegistered = counter;
		}
	}
	return generateCmd(buttonRegistered);
}


/*
	move()
	- Description: Moves the corresponding slider from current position of
	circle to pre-determined destination (variable position).
	- Parameters: position - destination
								n - slider to move
								halfpoint - which direction?
								indicator - Switch the snap_state status i.e. the update
								the position of the slider state position
*/
void move(double position, int n, int halfpoint, int indicator) {
	int toInfinity;
	slider[n].snap_state = indicator;
	if (halfpoint == 0) {
		// <-- 
		for (toInfinity = 0; slider[n].circle_x0 > position; toInfinity++) {
			if (toInfinity % 500 == 0) {
				slider[n].circle_x0 = slider[n].circle_x0 - 2;
				slider[n].rect1_x1 = slider[n].rect1_x1 - 2;
				slider[n].rect2_x0 = slider[n].rect2_x0 - 2;
				drawSlider();
			}
		}
	} else {
		// -->
		for (toInfinity = 0; slider[n].circle_x0 < position; toInfinity++) {
			if (toInfinity % 500 == 0) {
				slider[n].circle_x0 = slider[n].circle_x0 + 2;
				slider[n].rect1_x1 = slider[n].rect1_x1 + 2;
				slider[n].rect2_x0 = slider[n].rect2_x0 + 2;
				drawSlider();
			}
		}
	}
}


/*
	snapSlider()
	- Description: Snaps the slider to a location depending on the
	where it is at any one instance.
	- Parameters: n - the slider id
							  indicator - to which position
*/
void snapSlider(int n, int indicator) {
	double position;
	int displacement;
	switch (indicator) {
		case 1:
			position = (double)slider[n].minus_x1 + slider[n].radius*2;
			move(position, n, 0, indicator);
			break;
		case 2:
			position = (double)DISPLAY_WIDTH/3;
			displacement = (slider[n].circle_x0 > position) ? 0 : 1;
			move(position, n, displacement, indicator);
			break;
		case 3:
			position = (double)DISPLAY_WIDTH/2;
			displacement = (slider[n].circle_x0 > position) ? 0 : 1;
			move(position, n, displacement, indicator);
			break;
		case 4:
			position = (double)(0.66)*DISPLAY_WIDTH;
			displacement = (slider[n].circle_x0 > position) ? 0 : 1;
			move(position, n, displacement, indicator);
			break;
		case 5:
			position = (double)slider[n].plus_x0 - slider[n].radius*2;
			move(position, n, 1, indicator);
			break;
	}
}

/*
	drawStatusBar()
	- Description: Draws the status bar, and master power button
	- Parameters: indicator - FIRE alarm status, used for deciding
								whether to print default status bar or EXIT message.
								Also used for coloring appropiately
*/
// Draws the yellow status bar and master power button
void drawStatusBar(int indicator) {
	int circles = 0;	
	int refreshCheck = 0;

	// Status bar
	lcd_fillRect(0, 0, DISPLAY_WIDTH, 25, ((indicator == 0)?YELLOW:(indicator == 2) ? LIGHT_GRAY : GREEN));
	lcd_fontColor(BLACK, ((indicator == 0)?YELLOW:(indicator == 2) ? LIGHT_GRAY : GREEN));
	lcd_putString(DISPLAY_WIDTH/3 - 7.5, STATUS_TOP_OFFSET, ((indicator == 0)? STR_LIGHTS: (indicator == 2)? EXIT : STR_SAVED));
	
	// Master power button
	for (counter = 0; counter < WidgetButtonCount; counter++) {
		if (button[counter].state == 1) {
			refreshCheck = 1;
		}
	}
	for (counter = 5; counter < 7; counter++) {
		if (counter == 5) {
			lcd_fillRect(button[counter].x0, button[counter].y0, button[counter].x1, button[counter].y1, (refreshCheck == 1) ? button[counter].alterColor : button[counter].color);
		} else {
			for (circles = 0; circles < 3; circles++) {
				lcd_circle(button[counter].x0, button[counter].y0, button[counter].radius + circles, (refreshCheck == 1) ? button[counter].alterColor : button[counter].color);
			}
		}
	}
}

/*
	drawButtons()
	- Description: Draws all the buttons
	- Parameters: indicator - FIRE status, alters the colors accordingly
*/
// Draws the buttons
void drawButtons(int indicator) {
	for (counter = 0; counter < 5; counter = counter + 1) {
		lcd_fillRect(button[counter].x0, button[counter].y0, button[counter].x1, button[counter].y1, (indicator == 0)?(button[counter].state == 1) ? button[counter].alterColor: button[counter].color:(indicator == 1) ? button[counter].fire_color1: button[counter].fire_color2);
	}
	
	// Whiteboard
	lcd_fontColor(BLACK, (indicator == 0)?(button[0].state == 1) ? button[0].alterColor: button[0].color:(indicator == 1) ? button[0].fire_color1: button[0].fire_color2);
	lcd_putString(button[0].str_x, button[0].str_y, STR_WHITEBOARD);
	
	// Lecturer
	lcd_fontColor(BLACK, (indicator == 0)?(button[1].state == 1) ? button[1].alterColor: button[1].color:(indicator == 1) ? button[1].fire_color1: button[1].fire_color2);
	lcd_putString(button[1].str_x, button[1].str_y, STR_LECTURER);
	
	// Seating
	lcd_fontColor(BLACK, (indicator == 0)?(button[2].state == 1) ? button[2].alterColor: button[2].color:(indicator == 1) ? button[2].fire_color1: button[2].fire_color2);
	lcd_putString(button[2].str_x, button[2].str_y, STR_SEATING);
	
	// AISLE LEFT
	aisleStrOffset = button[3].str_y;
	lcd_fontColor(BLACK, (indicator == 0)?(button[3].state == 1) ? button[3].alterColor: button[3].color:(indicator == 1) ? button[3].fire_color1: button[3].fire_color2);
	lcd_putChar(button[3].str_x, aisleStrOffset, 'A');
	aisleStrOffset = aisleStrOffset + 10;
	lcd_putChar(button[3].str_x, aisleStrOffset, 'I');
	aisleStrOffset = aisleStrOffset + 10;
	lcd_putChar(button[3].str_x, aisleStrOffset, 'S');
	aisleStrOffset = aisleStrOffset + 10;
	lcd_putChar(button[3].str_x, aisleStrOffset, 'L');
	aisleStrOffset = aisleStrOffset + 10;
	lcd_putChar(button[3].str_x, aisleStrOffset, 'E');
	
	// AISLE RIGHT
	aisleStrOffset = button[4].str_y;
	lcd_fontColor(BLACK, (indicator == 0)?(button[4].state == 1) ? button[4].alterColor: button[4].color:(indicator == 1) ? button[4].fire_color1: button[4].fire_color2);
	lcd_putChar(button[4].str_x, aisleStrOffset, 'A');
	aisleStrOffset = aisleStrOffset + 10;
	lcd_putChar(button[4].str_x, aisleStrOffset, 'I');
	aisleStrOffset = aisleStrOffset + 10;
	lcd_putChar(button[4].str_x, aisleStrOffset, 'S');
	aisleStrOffset = aisleStrOffset + 10;
	lcd_putChar(button[4].str_x, aisleStrOffset, 'L');
	aisleStrOffset = aisleStrOffset + 10;
	lcd_putChar(button[4].str_x, aisleStrOffset, 'E');
	
	// Presets
	for (counter = 0; counter < PresetButtonCount; counter++) {
		lcd_fillRect(preset[counter].x0, preset[counter].y0, preset[counter].x1, preset[counter].y1, (preset[counter].state == 1) ? preset[counter].alterColor: preset[counter].color);
	}
	lcd_fontColor(WHITE, (preset[0].state == 1) ? preset[0].alterColor: preset[0].color);
	lcd_putString(preset[0].str_x, preset[0].str_y, STR_P1);
	lcd_fontColor(WHITE, (preset[1].state == 1) ? preset[1].alterColor: preset[1].color);
	lcd_putString(preset[1].str_x, preset[1].str_y, STR_P2);
}

/*
	checkPresets()
	- Description: Checks whether the presets are pressed, also starts
	a timer to see if user has held or double clicked the buttons.
	Holding the buttons will save the preset on the current state of
	the lighting system. Double clicking will change the state
	of the lighting system to the preset.
	- Parameters: x - x coordinate of TS event
								y - y coordinate of TS event
*/
Command checkPresets(int x, int y) {
	int buttonRegistered = -1;
	for (counter = 0; counter < PresetButtonCount; counter++) {
		if ((x > preset[counter].x0) && (x < preset[counter].x1) && (y > preset[counter].y0) && (y < preset[counter].y1)) {
			if (preset[counter].state == 0) {
				preset[counter].state = 1;
			} else {
				preset[counter].state = 0;
			}
			buttonRegistered = counter;
			if (doubleClick == 0) {
				xTimerStart( xTimerDoubleClick, 0 );
				doubleClickID = buttonRegistered;
				doubleClick++;
			} else if ((doubleClick == 1) && (buttonRegistered == doubleClickID)) {
				doubleClick++;
			}
		}
	}
	return generateCmd((buttonRegistered == -1)?buttonRegistered:(buttonRegistered == 0)?8:9);
}

/*
	presetReset()
	- Description: Reset all states of preset buttons to 0
	- Parameters: N/A
*/
void presetReset() {
	for (counter = 0; counter < PresetButtonCount; counter++) {
		preset[counter].state = 0;
	}
}

/*
	checkPressed()
	- Description: Checks whether any of the buttons are pressed
	- Parameters: x - x coordinate of TS event
								y - y coordinate of TS event
	- CMD identifiers:
		0-2: WHITEBOARD / LECTURER / SEATING
		3-4: AISLE
		5-6: MASTER POWER
*/
Command checkPressed(int x, int y) {
  int buttonRegistered;
	int reflectCounter;
	int allOffCheckCount;
	int allOff = 0;
	buttonRegistered = -1;
	for (counter = 0; counter < WidgetButtonCount; counter++) {
		if ((x > button[counter].x0) && (x < button[counter].x1) && (y > button[counter].y0) && (y < button[counter].y1)) {
			if (button[counter].state == 0) {
				// Turn on the POWER state
				button[5].state = 1;
				button[6].state = 1;
				POWER = 5;
				
				if ((counter == 3) || (counter == 4)) { // Turn on both the aisles on the UI!
					button[3].state = 1;
					button[4].state = 1;
				} else {
					button[counter].state = 1;
				}
				slider[(counter == 4) ? 3 : counter].state = 1;
				snapSlider((counter == 4) ? 3 : counter, 5);
			} else {
				if ((counter == 3) || (counter == 4)) { // Or turn them off
					button[3].state = 0;
					button[4].state = 0;
				} else {
					button[counter].state = 0;
				}
				slider[(counter == 4) ? 3 : counter].state = 0;
				snapSlider((counter == 4) ? 3 : counter, 3);
				
				// Checks if state of all buttons are off; turn master off
				for (allOffCheckCount = 0; allOffCheckCount < 4; allOffCheckCount++) {
					if (button[allOffCheckCount].state == 1) {
						allOff = 1;
					}
				}
				if (allOff == 0) {
					button[5].state = 0;
					button[6].state = 0;
					POWER = 1;
				}
			}
			buttonRegistered = counter;
		}
		// Checks if within the bounding box of all 3 circles and rectangle of Master Power button; changes the state accordingly
		if ((counter == 5) || (counter == 6)) {
			if ((x > button[6].bounding_x0) && (y > button[6].bounding_y0) && (x < button[6].bounding_x1) && (y < button[6].bounding_y1)) {
				if (button[counter].state == 0) {
					button[counter].state = 1;
					POWER = 5;
					for (reflectCounter = 0; reflectCounter < 5; reflectCounter++) {
						button[reflectCounter].state = 1;
						slider[(reflectCounter == 4) ? 3 : reflectCounter].state = 1;
						slider[(reflectCounter == 4) ? 3 : reflectCounter].snap_state = 5;
						snapSlider((reflectCounter == 4) ? 3 : reflectCounter, 5);
					}
				} else {
					button[counter].state = 0;
					POWER = 1;
					for (reflectCounter = 0; reflectCounter < 5; reflectCounter++) {
						button[reflectCounter].state = 0;
						slider[(reflectCounter == 4) ? 3 : reflectCounter].state = 0;
						slider[(reflectCounter == 4) ? 3 : reflectCounter].snap_state = 3;
						snapSlider((reflectCounter == 4) ? 3 : reflectCounter, 3);
					}
				}
				buttonRegistered = counter;
			}
		}
	}
	return generateCmd((buttonRegistered == 4) ? 3 : buttonRegistered);
}

/* 
	 reflectState()
	- Description: Checks which light is, used mainly
	for CLAP timeout to reflect the changes of the
	corresponding light set to ON
	- Parameters: state - the state to force it to
*/
void reflectState(unsigned char state) {
	int stateCounter;
	unsigned char temp = 0;
	int allOffCheckCount;
	int allOff = 0;
	
	button[5].state = 1;
	button[6].state = 1;
	POWER = 5;
	
	// Checks to see which light is of effect
	for (stateCounter = 0; stateCounter < 4; stateCounter++) {
		temp = (3 << stateCounter * 2);
		temp &= state;
		
		switch ((unsigned char) temp >> stateCounter * 2) {
			case 0:
				if (stateCounter == 3) {
					button[4].state = 0;
				}
				button[stateCounter].state = 0;
				slider[stateCounter].state = 0;
				snapSlider(stateCounter, 3);
				break;
			case 1:
				if (stateCounter == 3) {
					button[4].state = 1;
				}
				button[stateCounter].state = 1;
				slider[stateCounter].state = 1;
				snapSlider(stateCounter, 5);
				break;
			case 2:
				if (stateCounter == 3) {
					button[4].state = 1;
				}
				button[stateCounter].state = 1;
				slider[stateCounter].state = 1;
				snapSlider(stateCounter, 4);
				break;
			case 3:
				if (stateCounter == 3) {
					button[4].state = 1;
				}
				button[stateCounter].state = 1;
				slider[stateCounter].state = 1;
				snapSlider(stateCounter, 2);
				break;
			default:
				break;
		}
		temp = 0;
	}
	// Checks if state of all buttons are off; turn master off
	for (allOffCheckCount = 0; allOffCheckCount < 4; allOffCheckCount++) {
		if (button[allOffCheckCount].state == 1) {
			allOff = 1;
		}
	}
	if (allOff == 0) {
		button[5].state = 0;
		button[6].state = 0;
		POWER = 1;
	}
}

/*
	forceShutdown()
	- Description: Used by PIR timeout function mainly, 
	forces the shutdown reflection on the UI, moves
	sliders back to initial state, sets everything OFF
	- Parameters: N/A
*/
void forceShutdown() {
	int buttonCounter;
	int sliderCounter;
	
	POWER = 1;
	
	for (buttonCounter = 0; buttonCounter < WidgetButtonCount; buttonCounter++) {
		button[buttonCounter].state = 0;
	}
	for (sliderCounter = 0; sliderCounter < 5; sliderCounter++) {
		slider[(sliderCounter == 4) ? 3 : sliderCounter].state = 0;
		slider[(sliderCounter == 4) ? 3 : sliderCounter].snap_state = 3;
		snapSlider((sliderCounter == 4) ? 3 : sliderCounter, 3);
	}
}

/*
	getLEDState()
	- Description: Gets the state of the LED. button structs
	are created here, thus ClapTimeout will query for button state
	- Parameters: n - the button to query the state of
*/
int getLEDState(int n) {
	return button[n].state;
}
