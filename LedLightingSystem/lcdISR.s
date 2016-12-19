; This is the LPC2468 platform-specific interrupt handler for
; LCD touch-screen interrupts. It simply saves the context of the
; current task, calls the real interrupt handler vLCD_ISRHandler()
; and then restores the context of the next task, which may
; be different from the task that was running when the interrupt
; occurred.
 
	INCLUDE portmacro.inc
	
	IMPORT vLCD_ISRHandler
	EXPORT vLCD_ISREntry

	;/* Interrupt entry must always be in ARM mode. */
	ARM
	AREA	|.text|, CODE, READONLY


vLCD_ISREntry

	PRESERVE8

	; Save the context of the interrupted task.
	portSAVE_CONTEXT			

	; Call the C handler function - defined within lcd.c.
	LDR R0, =vLCD_ISRHandler
	MOV LR, PC				
	BX R0

	; Finish off by restoring the context of the task that has been chosen to 
	; run next - which might be a different task to that which was originally
	; interrupted.
	portRESTORE_CONTEXT

	END