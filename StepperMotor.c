// Contains Code for Stepper motor Functions

#include "tm4c1294ncpdt.h"
#include <stdint.h>
#include "SysTick.h"

//Initialize Port H for Motor
void PortH_Init(void){
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R7;				// activate clock for Port H
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R7) == 0){};	// allow time for clock to stabilize
	
		GPIO_PORTH_DIR_R |= 0xFF;        								// make PH0 out 
  GPIO_PORTH_AFSEL_R &= ~0xFF;     								// disable alt funct on PH0
  GPIO_PORTH_DEN_R |= 0xFF;        								// enable digital I/O on PH0
																									// configure PN1 as GPIO
  GPIO_PORTH_AMSEL_R &= ~0xFF;     								// disable analog functionality on PN0		
	return;
}
void CW(int delay) { //CCW
				GPIO_PORTH_DATA_R = 0b00001001;
				SysTick_Wait10ms(delay);
				GPIO_PORTH_DATA_R = 0b00000011;

				SysTick_Wait10ms(delay);											

				GPIO_PORTH_DATA_R = 0b00000110;

				SysTick_Wait10ms(delay);

				GPIO_PORTH_DATA_R = 0b00001100;

				SysTick_Wait10ms(delay);

			
			GPIO_PORTH_DATA_R = GPIO_PORTH_DATA_R ^ 0b00000001; //complements motor
      // Exit the for loop prematurely to stop motor rotation
	
	
}

void CCW (int delay) {//CW
	GPIO_PORTH_DATA_R = 0b00001100;

			SysTick_Wait10ms(delay);
			GPIO_PORTH_DATA_R = 0b00000110;
			SysTick_Wait10ms(delay);
			GPIO_PORTH_DATA_R = 0b00000011;
			SysTick_Wait10ms(delay);	
			GPIO_PORTH_DATA_R = 0b00001001;
			SysTick_Wait10ms(delay);
			GPIO_PORTH_DATA_R = GPIO_PORTH_DATA_R ^ 0b00000001; //complements motor
			// Exit the for loop prematurely to stop motor rotation
}

void rotate(int delay, int dir) {
	if(dir == 1) {
			CW(delay);
	} 
	else if(dir == 0) {
		CCW(delay);
	}
}