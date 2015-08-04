/**
* EncoderRight.c
* This file contains the input capture initialization and interrupt
* service routine.
*/

#include <stdint.h>

#include "ES_Configure.h"
#include "ES_Framework.h"

#include "inc/hw_memmap.h"
#include "inc/hw_timer.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "bitdefs.h"
#include "StrategySM.h"
#include "EncoderRight.h"
#include "TurnFSM.h"
#include "EncoderLeft.h"

#define TICKS_PER_MS 40000
//#define AVG_COUNT 5 // number of period measurements to average
#define ENC_R_PRIO 0 // set priority to be 0
#define ENC_R_PRIO_REM 2 // Interrupt 98/4 = 24 with 2 remainder
#define PERIOD_THRESHOLD 5*TICKS_PER_MS

static uint32_t period;
static uint32_t lastCapture;
static uint32_t count = 0;
static bool positionControlStatus = false;
static bool postEvent = false;

static uint32_t desiredEdgeCount = 0;
//static uint8_t capCount = 0;
//static uint32_t periodSum = 0;



// If the desired function is to simply count edges, use param edgeCount = 0 
void startPosControlRight(uint32_t edgeCount){
	count = 0;
	desiredEdgeCount = edgeCount;
	positionControlStatus = true;
	if(edgeCount == 0)postEvent = false;
	else postEvent = true;
	}

/**
* Initializes Timer A in Wide Timer 2 to capture the input.
* Initializes Timer B in Wide Timer 1 to capture the input.
*/
void initEncoderRight(void) {
	/* Start by enabling the clock to the timer (Wide Timer 1) */
	HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R1;
			
	/* Enable the clock to Port C */
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R2;
	/* Since we added this Port C clock init, we can immediately start
	into configuring the timer, no need for further delay */
		  
	/* Make sure that timer (Timer B) is disabled before configuring */
	HWREG(WTIMER1_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TBEN;
		  
	/* Set it up in 32-bit wide (individual, not concatenated) mode */
	/* The constant name derives from the 16/32 bit timer, but this is a 32/64
	bit timer so we are setting the 32-bit mode */
	HWREG(WTIMER1_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;

	/* We want to use the full 32-bit count, so initialize the Interval Load
	Register to 0xffff.ffff (its default value) */
	HWREG(WTIMER1_BASE+TIMER_O_TBILR) = 0xffffffff;

	/* We don't want any prescaler (it is unnecessary with a 32-bit count) */
	//  HWREG(WTIMER1_BASE+TIMER_O_TBPR) = 0;

	/* Set up timer B in capture mode (TBMR = 3, TBAMS = 0), 
	for edge time (TBCMR = 1) and up-counting (TBCDIR = 1) */
	HWREG(WTIMER1_BASE+TIMER_O_TBMR) = (HWREG(WTIMER1_BASE+TIMER_O_TBMR) & ~TIMER_TBMR_TBAMS) | 
			(TIMER_TBMR_TBCDIR | TIMER_TBMR_TBCMR | TIMER_TBMR_TBMR_CAP);

	/* To set the event to rising edge, we need to modify the TBEVENT bits 
	in GPTMCTL. Rising edge = 00, so we clear the TBEVENT bits */
	HWREG(WTIMER1_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TBEVENT_M;

	/* Now set up the port to do the capture (clock was enabled earlier)
	start by setting the alternate function for Port C bit 7 (WT1CCP1) */
	HWREG(GPIO_PORTC_BASE+GPIO_O_AFSEL) |= BIT7HI;

	/* Then, map bit 7's alternate function to WT1CCP1 */
	/* 7 is the mux value to select WT1CCP1 */
	HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL) = (HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL) & 0x0fffffff) + (7<<28);

	/* Enable pin on Port C for digital I/O */
	HWREG(GPIO_PORTC_BASE+GPIO_O_DEN) |= BIT7HI;
			
	/* Make pin 7 on Port C into an input */
	HWREG(GPIO_PORTC_BASE+GPIO_O_DIR) &= BIT7LO;

	/* Back to the timer to enable a local capture interrupt */
	maskRightEncoder(true);

	/* Enable the Timer B in Wide Timer 1 interrupt in the NVIC */
	/* It is interrupt number 97 so appears in EN3 at bit 1 */
	/* Refer to pp. 105 and 142 in datasheet */
	HWREG(NVIC_EN3) |= BIT1HI;

	/* Make sure interrupts are enabled globally */
	__enable_irq();

	/* Now kick the timer off by enabling it and enabling the timer to
	stall while stopped by the debugger */
	HWREG(WTIMER1_BASE+TIMER_O_CTL) |= (TIMER_CTL_TBEN | TIMER_CTL_TBSTALL);

}

void maskRightEncoder(bool masked){
		if(masked) HWREG(WTIMER1_BASE+TIMER_O_IMR) &= ~TIMER_IMR_CBEIM;
		else 	HWREG(WTIMER1_BASE+TIMER_O_IMR) |= TIMER_IMR_CBEIM;
}

/**
* Calculates the time between consecutive rising edges on PD0. This is
* the interrupt service routine (ISR).
*/
void encoderRightResponse(void) {
	//printf("eRR\r\n"); //****************************************************************************
	uint32_t thisCapture;
	/* Start by clearing the source of the interrupt, the input capture event */
	//HWREG(WTIMER2_BASE+TIMER_O_ICR) = TIMER_ICR_CAECINT;
	HWREG(WTIMER1_BASE+TIMER_O_ICR) = TIMER_ICR_CBECINT;
	
	/* Now grab the captured value and calculate the period */
	//thisCapture = HWREG(WTIMER2_BASE+TIMER_O_TAR);
	thisCapture = HWREG(WTIMER1_BASE+TIMER_O_TBR);
	
	period = thisCapture - lastCapture;
	lastCapture = thisCapture;
		if(positionControlStatus){
			if(period>PERIOD_THRESHOLD)count++;
			if (count == desiredEdgeCount && postEvent) {
			//	ES_Event Event;
				//Event.EventType = PositionReached;
				//PostStrategySM(Event);
				//PostTurningFSM(Event);
				//maskLeftEncoder(true); 
				//maskRightEncoder(true);
				desiredEdgeCount = 0;
				positionControlStatus = false;
				postEvent = false;
		}
}
	
	
	//printf("%i\r\n", period);
}

uint32_t getEdgeCountRight(void){
	return count;
}

/**
* Returns the last stored period.
* @return last period captured by the input capture register
*/
uint32_t getPeriodRight(void) {
	return period;
}
