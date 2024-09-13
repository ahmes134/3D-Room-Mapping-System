//PROJECT DELIVERABLE 2 - March 25, 2024
// NAME: Sarah Ahmed
// STUDENT #: 400389144
// MACID: Ahmes134
// ASSIGNED BUS SPEED: 24MHz
// ASSIGNED DISTANCE STATUS PIN: PF4 (D3)
// ASSIGNED DISPLACEMENT STATUS PIN: PN1 (D1)
// The default bus speed is 120MHz


//
// Adapted from Time of Flight for 2DX4 -- Studio W8-0 Code

#include <stdint.h>
#include "tm4c1294ncpdt.h"
#include "Systick.h"
#include "PLL.h"
#include "OnboardButton.h"
#include "StepperMotor.h"

//For Communication
#include "onboardLEDs.h"
#include "VL53L1X_api.h"
#include "uart.h"

uint16_t dev = 0x29;			//address of the ToF sensor as an I2C slave peripheral


//Initialize Ports PN1 and PF4 for on-board GPIOs (from lab 6)
//Enable LED D1, D2. Remember D1 is connected to PN1 and D2 is connected to PN0
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


//Enable LED D3, D4. Remember D3 is connected to PF4 and D4 is connected to PF0
void PortF_Init(void){
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;                 // Activate the clock for Port F
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R5) == 0){};
	GPIO_PORTF_DIR_R=0b00010000;														// Enable PF4 as digital outputs
	GPIO_PORTF_DEN_R=0b00010000;
	return;
}
//For Bus Speed Confirmation
void PortE_Init(void){	
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;		              // Activate the clock for Port E
	while((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R4) == 0){};	      // Allow time for clock to stabilize
  
	GPIO_PORTE_DIR_R = 0b00000001;														// Enable PE0 as output
	GPIO_PORTE_DEN_R = 0b00000001;                        		// Enable PE0 as digital pins
	
	return;
	}

//I2C Communication Protocol
void I2C_Init(void){
  SYSCTL_RCGCI2C_R |= SYSCTL_RCGCI2C_R0;           													// activate I2C0
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;          												// activate port B
  while((SYSCTL_PRGPIO_R&0x0002) == 0){};																		// ready?

    GPIO_PORTB_AFSEL_R |= 0x0C;           																	// 3) enable alt funct on PB2,3       0b00001100
    GPIO_PORTB_ODR_R |= 0x08;             																	// 4) enable open drain on PB3 only

    GPIO_PORTB_DEN_R |= 0x0C;             																	// 5) enable digital I/O on PB2,3
//    GPIO_PORTB_AMSEL_R &= ~0x0C;          																// 7) disable analog functionality on PB2,3

                                                                            // 6) configure PB2,3 as I2C
//  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFF00FF)+0x00003300;
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFF00FF)+0x00002200;    //TED
    I2C0_MCR_R = I2C_MCR_MFE;                      													// 9) master function enable
    I2C0_MTPR_R = 0b0000000000000101000000000111011;                       	// 8) configure for 100 kbps clock (added 8 clocks of glitch suppression ~50ns)
//    I2C0_MTPR_R = 0x3B;                                        						// 8) configure for 100 kbps clock
        
}

//The VL53L1X needs to be reset using XSHUT.  We will use PG0
void PortG_Init(void){
    //Use PortG0
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R6;                // activate clock for Port N
    while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R6) == 0){};    // allow time for clock to stabilize
    GPIO_PORTG_DIR_R &= 0x00;                                        // make PG0 in (HiZ)
  GPIO_PORTG_AFSEL_R &= ~0x01;                                     // disable alt funct on PG0
  GPIO_PORTG_DEN_R |= 0x01;                                        // enable digital I/O on PG0
                                                                                                    // configure PG0 as GPIO
  //GPIO_PORTN_PCTL_R = (GPIO_PORTN_PCTL_R&0xFFFFFF00)+0x00000000;
  GPIO_PORTG_AMSEL_R &= ~0x01;                                     // disable analog functionality on PN0

    return;
}

//XSHUT     This pin is an active-low shutdown input; 
//					the board pulls it up to VDD to enable the sensor by default. 
//					Driving this pin low puts the sensor into hardware standby. This input is not level-shifted.
void VL53L1X_XSHUT(void){
    GPIO_PORTG_DIR_R |= 0x01;                                        // make PG0 out
    GPIO_PORTG_DATA_R &= 0b11111110;                                 //PG0 = 0
    FlashAllLEDs();
    SysTick_Wait10ms(10);
    GPIO_PORTG_DIR_R &= ~0x01;                                            // make PG0 input (HiZ)
    
}

int status = 0; //initialize status to off

int direction = 0;
//int angle = 
uint16_t debugArray[512]; //capture values from VL53L1X for inspection

//For Communication
	uint8_t byteData, sensorState=0, myByteArray[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} , i=0;
  uint16_t wordData;
  uint16_t Distance;
  uint8_t dataReady;
	
void GPIOJ_IRQHandler(void)
{
	direction = !direction;
	FlashLED2(1);											// Flash the LED D2 once
	GPIO_PORTJ_ICR_R = 0x02; 					// Acknowledge flag by setting proper bit in ICR register
		for(int i=1; i<=512; i++)
		{ 
				GPIO_PORTN_DATA_R = 0b00000001; //turn LED D2 ON 
				if (i%16 ==0)
				{
					//5 wait until the ToF sensor's data is ready
					while (dataReady == 0)
					{
						SysTick_Wait10ms(10);
						status = VL53L1X_CheckForDataReady(dev, &dataReady);
								FlashLED3(5);
								VL53L1_WaitMs(dev, 5);
					}
					dataReady = 0;
					status = VL53L1X_ClearInterrupt(dev); /* 8 clear interrupt has to be called to enable next interrupt*/
				
				//7 read the data values from ToF sensor
				status = VL53L1X_GetDistance(dev, &Distance);					//7 The Measured Distance value
				sprintf(printf_buffer,"%d\r\n",Distance);
				UART_printf(printf_buffer);
			}
				rotate(1,direction);
		}		
	}


int main(void){
	PLL_Init();	           													// Default Set System Clock to 120MHz
	SysTick_Init();																	// Initialize SysTick configuration
	PortN_Init(); // On-board LEDs PN0 
	PortF_Init(); // On-board LEDs PF
	PortH_Init();
	PortE_Init(); // For Bus Speed Confirmation
	PortJ_Init(); // Initialize the onboard push button on PJ1
	PortJ_Interrupt_Init();	// Initalize and configure the Interrupt on Port J
 
	onboardLEDs_Init();
	I2C_Init();
	UART_Init();
	
	int mynumber = 1;
	sprintf(printf_buffer,"2DX ToF Program Studio Code %d\r\n",mynumber);
	UART_printf(printf_buffer);

	// 1 Wait for device ToF booted
	while(sensorState==0){
		status = VL53L1X_BootState(dev, &sensorState);
		SysTick_Wait10ms(10);
  }
	
  /* 2 Initialize the sensor with the default setting  */
  status = VL53L1X_SensorInit(dev);
	UART_printf("Press GPIO J1 to start/stop data capture\r\n");
  status = VL53L1X_StartRanging(dev);   // 4 This function has to be called to enable the ranging 

	while(1)
	{	
			WaitForInt();	
			GPIO_PORTE_DATA_R ^= 0b00000001; 
			SysTick_Wait10ms(100);
	}
}