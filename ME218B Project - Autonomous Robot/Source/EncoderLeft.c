/**
* EncoderLeft.c
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

#include "EncoderLeft.h"
#include "StrategySM.h"
#include "TurnFSM.h"
#include "EncoderRight.h"

#define TICKS_PER_MS 40000 // number of clock ticks per [ms]
//#define AVG_COUNT 5 // number of period measurements to average
#define ENC_L_PRIO 0 // set priority to be 0
#define ENC_L_PRIO_REM 0 // Interrupt 96/4 = 24 with 0 remainder
#define PERIOD_THRESHOLD 5*TICKS_PER_MS


static uint32_t period;
static uint32_t lastCapture;
static uint32_t count = 0;
static uint32_t desiredEdgeCount = 0;
static bool positionControlStatus = false;
static bool postEvent = false;

void startPosControlLeft(uint32_t edgeCount){
	count = 0;
	desiredEdgeCount = edgeCount;
	//printf("Desired EdgeCount = %u\r\n", desiredEdgeCount);
	positionControlStatus = true;
	if(edgeCount == 0)postEvent = false;
	else postEvent = true;
}


/**
* Initializes Timer A in Wide Timer 1 to capture the input.
*/
void initEncoderLeft(void) {
	/* Start by enabling the clock to the timer (Wide Timer 1) */
	HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R1;
			
	/* Enable the clock to Port C */
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R2;
	/* Since we added this Port C clock init, we can immediately start
	into configuring the timer, no need for further delay */
		  
	/* Make sure that timer (Timer A) is disabled before configuring */
	HWREG(WTIMER1_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
		  
	/* Set it up in 32-bit wide (individual, not concatenated) mode */
	/* The constant name derives from the 16/32 bit timer, but this is a 32/64
	bit timer so we are setting the 32-bit mode */
	HWREG(WTIMER1_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;

	/* We want to use the full 32-bit count, so initialize the Interval Load
	Register to 0xffff.ffff (its default value) */
	HWREG(WTIMER1_BASE+TIMER_O_TAILR) = 0xffffffff;

	/* We don't want any prescaler (it is unnecessary with a 32-bit count) */
	//  HWREG(WTIMER1_BASE+TIMER_O_TAPR) = 0;

	/* Set up timer A in capture mode (TAMR = 3, TAAMS = 0), 
	for edge time (TACMR = 1) and up-counting (TACDIR = 1) */
	HWREG(WTIMER1_BASE+TIMER_O_TAMR) = (HWREG(WTIMER1_BASE+TIMER_O_TAMR) & ~TIMER_TAMR_TAAMS) | 
			(TIMER_TAMR_TACDIR | TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP);

	/* To set the event to rising edge, we need to modify the TAEVENT bits 
	in GPTMCTL. Rising edge = 00, so we clear the TAEVENT bits */
	HWREG(WTIMER1_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEVENT_M;

	/* Now set up the port to do the capture (clock was enabled earlier)
	start by setting the alternate function for Port C bit 6 (WT1CCP0) */
	HWREG(GPIO_PORTC_BASE+GPIO_O_AFSEL) |= BIT6HI;

	/* Then, map bit 6's alternate function to WT1CCP0 */
	/* 7 is the mux value to select WT1CCP0, 16 to shift it over to the
	right nibble for bit 6 (4 bits/nibble * 6 bits) */
	HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL) = (HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL) & 0xfff0ffff) + (7<<24);

	/* Enable pin on Port C for digital I/O */
	HWREG(GPIO_PORTC_BASE+GPIO_O_DEN) |= BIT6HI;
			
	/* Make pin 6 on Port C into an input */
	HWREG(GPIO_PORTC_BASE+GPIO_O_DIR) &= BIT6LO;

	/* Back to the timer to enable a local capture interrupt */
  maskLeftEncoder(true);

	/* Enable the Timer A in Wide Timer 1 interrupt in the NVIC */
	/* It is interrupt number 96 so appears in EN3 at bit 0 */
	HWREG(NVIC_EN3) |= BIT0HI;
	
	/* Set the priority to be high (i.e., = 0). Interrupt 96 is INTA in PRI24 */
	/* Refer to p. 152 in datasheet */
	//HWREG(NVIC_PRI24) = (HWREG(NVIC_PRI24) & ~NVIC_PRI24_INTA_M) | ((ENC_L_PRIO << 5) << (ENC_L_PRIO_REM*8));

	/* Make sure interrupts are enabled globally */
	__enable_irq();

	/* Now kick the timer off by enabling it and enabling the timer to
	stall while stopped by the debugger */
	HWREG(WTIMER1_BASE+TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TASTALL);
}

void maskLeftEncoder(bool masked){
	if (masked)HWREG(WTIMER1_BASE+TIMER_O_IMR) &= ~TIMER_IMR_CAEIM;
	else 	HWREG(WTIMER1_BASE+TIMER_O_IMR) |= TIMER_IMR_CAEIM;
}

/**
* Calculates the time between consecutive rising edges on PC6. This is
* the interrupt service routine (ISR).
*/
void encoderLeftResponse(void) {
	//printf("eLR\r\n"); //****************************************************************************
	uint32_t thisCapture;
	/* Start by clearing the source of the interrupt, the input capture event */
	HWREG(WTIMER1_BASE+TIMER_O_ICR) = TIMER_ICR_CAECINT;
	/* Now grab the captured value and calculate the period */
	thisCapture = HWREG(WTIMER1_BASE+TIMER_O_TAR);
	period = thisCapture - lastCapture;
	lastCapture = thisCapture;
	
	if(positionControlStatus){
			
			if(period>PERIOD_THRESHOLD)count++;
			 // increment edge count
			if (count == desiredEdgeCount && postEvent) {
				ES_Event Event;
				Event.EventType = PositionReached;
  			PostStrategySM(Event);
			  PostTurningFSM(Event);
				//maskRightEncoder(true); 
				//maskLeftEncoder(true); 
				desiredEdgeCount = 0;
				positionControlStatus = false;
				postEvent = false;
		}
	}
}

uint32_t getEdgeCountLeft(void){
	return count;
}

/**
* Returns the last stored period.
* @return last period captured by the input capture register
*/
uint32_t getPeriodLeft(void) {
	return period;
}
