#ifndef COMMANDS_H
#define COMMANDS_H

enum P1_CMDS {LED_OFF, LED_ON};

typedef struct Command
{
  int value;
	int identifier;
	int action;
	int dimValue;
	unsigned char state;
} Command;

#endif /* COMMANDS_H */
