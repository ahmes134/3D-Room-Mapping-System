/* Studio 7A Code
		Sample code provided for studio to demonstrate GPIO interrupt 
		Based upon interrupt textbook code by Valvano.

		This code will use a push button PJ1 to trigger an interrupt.  The trigger
		will cause the PortJ ISR to execute. The ISR will flash an onboard LED on/off.
		However, you can adapt this code to perform any operations simply by changing the code in the ISR


		Written by Dr. Tom Doyle
		Last Updated: February 21, 2024 by Dr. Shahrukh Athar.
*/

#include <stdint.h>
#include "tm4c1294ncpdt.h"
#include "PLL.h"
#include "SysTick.h"


// Flash LED D1 (D1 is connected to PN1)
void FlashLED1(int count) {
		while(count--) {
			GPIO_PORTN_DATA_R ^= 0b00000010; 								// hello world!
			SysTick_Wait10ms(10);														// 0.1s delay
			GPIO_PORTN_DATA_R ^= 0b00000010;			
		}
}

//Flash LED D2 (D2 is connected to PN0)
void FlashLED2(int count) {
		while(count--) {
			GPIO_PORTN_DATA_R ^= 0b00000001; 								// hello world!
			SysTick_Wait10ms(10);														// 0.1s delay
			GPIO_PORTN_DATA_R ^= 0b00000001;			
		}
}

// Initialize onboard LEDs
void PortN_Init(void){
	//Use PortN onboard LEDs
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R12;				// Activate clock for Port N
	while((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R12) == 0){};	// Allow time for clock to stabilize
	GPIO_PORTN_DIR_R |= 0x03;        								// Make PN0 and PN1 output (Built-in LEDs: D1 (PN1) and D2 (PN0))
  GPIO_PORTN_AFSEL_R &= ~0x03;     								// Disable alt funct on PN0 and PN1
  GPIO_PORTN_DEN_R |= 0x03;        								// Enable digital I/O on PN0 and PN1
																									
  //GPIO_PORTN_PCTL_R = (GPIO_PORTN_PCTL_R&0xFFFFFF00)+0x00000000;
  GPIO_PORTN_AMSEL_R &= ~0x03;     								// Disable analog functionality on PN0 and PN1
	FlashLED1(1);																		// Flash LED D1 (Hello World)
	return;
}

// Enable interrupts
void EnableInt(void)
{    __asm("    cpsie   i\n");
}

// Disable interrupts
void DisableInt(void)
{    __asm("    cpsid   i\n");
}

// Low power wait
void WaitForInt(void)
{    __asm("    wfi\n");
}

// Global variable visible in Watch window of debugger
// Increments at least once per button press
volatile unsigned long FallingEdges = 0;











































// Give clock to Port J and initalize PJ1 as Digital Input GPIO
void PortJ_Init(void){
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R8;					// Activate clock for Port J
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R8) == 0){};	// Allow time for clock to stabilize
  GPIO_PORTJ_DIR_R &= ~0x02;    										// Make PJ1 input 
  GPIO_PORTJ_DEN_R |= 0x02;     										// Enable digital I/O on PJ1
	
	GPIO_PORTJ_PCTL_R &= ~0x000000F0;	 								//  Configure PJ1 as GPIO 
	GPIO_PORTJ_AMSEL_R &= ~0x02;											//  Disable analog functionality on PJ1		
	GPIO_PORTJ_PUR_R |= 0x02;													//	Enable weak pull up resistor on PJ1
}


// Interrupt initialization for GPIO Port J IRQ# 51
void PortJ_Interrupt_Init(void){
	
		FallingEdges = 0;             		// Initialize counter

		GPIO_PORTJ_IS_R = 0;   						// (Step 1) PJ1 is Edge-sensitive 
		GPIO_PORTJ_IBE_R = 0;  						//     			PJ1 is not triggered by both edges 
		GPIO_PORTJ_IEV_R = 0;  						//     			PJ1 is falling edge event 
		GPIO_PORTJ_ICR_R = 0x02; 					// 					Clear interrupt flag by setting proper bit in ICR register
		GPIO_PORTJ_IM_R = 0x02;  					// 					Arm interrupt on PJ1 by setting proper bit in IM register
    
		NVIC_EN1_R = 0x00080000; 					// (Step 2) Enable interrupt 51 in NVIC (which is in Register EN1)
	
		NVIC_PRI12_R = 0xA0000000;				// (Step 4) Set interrupt priority to 5

		EnableInt();											// (Step 3) Enable Global Interrupt. lets go!
}


//	(Step 5) IRQ Handler (Interrupt Service Routine).  
//  				This must be included and match interrupt naming convention in startup_msp432e401y_uvision.s 
//					(Note - not the same as Valvano textbook).
void GPIOJ_IRQHandler(void){
  FallingEdges = FallingEdges + 1;	// Increase the global counter variable; Observe in Debug Watch Window
	FlashLED2(1);											// Flash the LED D2 once
	GPIO_PORTJ_ICR_R = 0x02; 					// Acknowledge flag by setting proper bit in ICR register
}


//	(Step 6) The main program -- 
//		Notice how we are only initializing the micro and nothing else.
// 		Our configured interrupts are being handled and tasks executed on an event driven basis.
int main(void){
  PLL_Init();           	// Set system clock to 120 MHz
	SysTick_Init();					// Initialize SysTick
	PortN_Init();						// Initialize the onboard LED on port N
	PortJ_Init();						// Initialize the onboard push button on PJ1
	PortJ_Interrupt_Init();	// Initalize and configure the Interrupt on Port J
	
	while(1){								// Inside an infinite while loop,
		WaitForInt();					// Call WaitForInt() function to wait for interrupts
	}
}