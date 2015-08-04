/**
* UltrasoundSense.c
* This file contains the input capture initialization and interrupt
* service routine for the ultrasound sensor.
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
#include "UltrasoundSense.h"

static uint32_t period;
static uint32_t lastCapture = 0;
static uint32_t targetPeriodMinus = 950;
static uint32_t targetPeriodPlus = 1050;

/**
* Initializes Timer A in Wide Timer 2 to capture the input.
*/
void initUltrasoundSense(void) {
	/* Start by enabling the clock to the timer (Wide Timer 2) */
	HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R2;
				
	/* Enable the clock to Port D */
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R3;
	/* Since we added this Port D clock init, we can immediately start
	into configuring the timer, no need for further delay */
		
	/* Make sure that timer (Timer A) is disabled before configuring */
	HWREG(WTIMER2_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
		
	/* Set it up in 32-bit wide (individual, not concatenated) mode */
	/* The constant name derives from the 16/32 bit timer, but this is a 32/64
	bit timer so we are setting the 32-bit mode */
	HWREG(WTIMER2_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;

	/* We want to use the full 32-bit count, so initialize the Interval Load
	Register to 0xffff.ffff (its default value) */
	HWREG(WTIMER2_BASE+TIMER_O_TAILR) = 0xffffffff;

	/* We don't want any prescaler (it is unnecessary with a 32-bit count) */
	//  HWREG(WTIMER2_BASE+TIMER_O_TAPR) = 0;

	/* Set up timer A in capture mode (TAMR = 3, TAAMS = 0), 
	for edge time (TACMR = 1) and up-counting (TACDIR = 1) */
	HWREG(WTIMER2_BASE+TIMER_O_TAMR) = (HWREG(WTIMER2_BASE+TIMER_O_TAMR) & ~TIMER_TAMR_TAAMS) | 
			(TIMER_TAMR_TACDIR | TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP);

	/* To set the event to rising edge, we need to modify the TAEVENT bits 
	in GPTMCTL. Rising edge = 00, so we clear the TAEVENT bits */
	HWREG(WTIMER2_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEVENT_M;

	/* Now set up the port to do the capture (clock was enabled earlier)
	start by setting the alternate function for Port D bit 0 (WT2CCP0) */
	HWREG(GPIO_PORTD_BASE+GPIO_O_AFSEL) |= BIT0HI;

	/* Then, map bit 0's alternate function to WT2CCP0 */
	/* 7 is the mux value to select WT2CCP0 (refer to pp. 1350 and 689) */
	HWREG(GPIO_PORTD_BASE+GPIO_O_PCTL) = (HWREG(GPIO_PORTD_BASE+GPIO_O_PCTL) & 0xfffffff0) + 7;

	/* Enable pin on Port D for digital I/O */
	HWREG(GPIO_PORTD_BASE+GPIO_O_DEN) |= BIT0HI;
				
	/* Make pin 0 on Port D into an input */
	HWREG(GPIO_PORTD_BASE+GPIO_O_DIR) &= BIT0LO;

	/* Back to the timer to enable a local capture interrupt */
	maskUltrasoundInterrupt(true);

	/* Enable the Timer A in Wide Timer 2 interrupt in the NVIC */
	/* It is interrupt number 98 so appears in EN3 at bit 2 */
	/* Refer to pp. 104 and 142 in datasheet */
	HWREG(NVIC_EN3) |= BIT2HI;

	/* Make sure interrupts are enabled globally */
	__enable_irq();

	/* Now kick the timer off by enabling it and enabling the timer to
	stall while stopped by the debugger */
	HWREG(WTIMER2_BASE+TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TASTALL);
}

void maskUltrasoundInterrupt(bool maskStatus){
	if (maskStatus) HWREG(WTIMER2_BASE+TIMER_O_IMR) &= ~TIMER_IMR_CAEIM;
	else HWREG(WTIMER2_BASE+TIMER_O_IMR) |= TIMER_IMR_CAEIM;
}

/**
* Calculates the time between consecutive rising edges on PD0. This is
* the interrupt service routine (ISR).
*/
void ultrasoundSenseResponse(void) {
	//printf("USR\r\n"); //****************************************************************************
	uint32_t thisCapture;
	/* Start by clearing the source of the interrupt, the input capture event */
	HWREG(WTIMER2_BASE+TIMER_O_ICR) = TIMER_ICR_CAECINT;
	thisCapture = HWREG(WTIMER2_BASE+TIMER_O_TAR);
	period = thisCapture - lastCapture; // add to periodSum
		/* Update lastCapture to prepare for the next edge */  
	lastCapture = thisCapture;
	
	/* Send a BeaconSensed event */
	
	if (period >=targetPeriodMinus && period <= targetPeriodPlus) {
//		ES_Event ThisEvent;
	//	ThisEvent.EventType = UltrasoundDetected;
		//PostStrategySM(ThisEvent);
		/* Mask the interrupt so that it doesn't call the ISR again */
		maskUltrasoundInterrupt(true);
	}
}
