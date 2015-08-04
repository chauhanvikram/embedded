/**
* InputCapture.c
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
#include "BeaconSense.h"


static uint32_t period;
static uint32_t lastCapture = 0;
static uint32_t targetPeriodMinus = 30400;
static uint32_t targetPeriodPlus = 33600;


/**
* Initializes Timer A in Wide Timer 0 to capture the input.
*/
void initBeaconSense(void) {
	/* Start by enabling the clock to the timer (Wide Timer 0) */
	HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R0;
	
	/* Enable the clock to Port C */
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R2;
	/* Since we added this Port C clock init, we can immediately start
	into configuring the timer, no need for further delay */
	
	/* Make sure that timer (Timer A) is disabled before configuring */
	HWREG(WTIMER0_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
	
	/* Set it up in 32-bit wide (individual, not concatenated) mode */
	/* The constant name derives from the 16/32 bit timer, but this is a 32/64
	bit timer so we are setting the 32-bit mode */
	HWREG(WTIMER0_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;
	
	/* We want to use the full 32-bit count, so initialize the Interval Load
	Register to 0xffff.ffff (its default value) */
	HWREG(WTIMER0_BASE+TIMER_O_TAILR) = 0xffffffff;
	
	/* We don't want any prescaler (it is unnecessary with a 32-bit count) */
	//HWREG(WTIMER0_BASE+TIMER_O_TAPR) = 0;

	/* Set up timer A in capture mode (TAMR = 3, TAAMS = 0), 
	for edge time (TACMR = 1) and up-counting (TACDIR = 1) */
	HWREG(WTIMER0_BASE+TIMER_O_TAMR) = 
			(HWREG(WTIMER0_BASE+TIMER_O_TAMR) & ~TIMER_TAMR_TAAMS) | 
				(TIMER_TAMR_TACDIR | TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP);

	/* To set the event to rising edge, we need to modify the TAEVENT bits 
	in GPTMCTL. Rising edge = 00, so we clear the TAEVENT bits */
	HWREG(WTIMER0_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEVENT_M;

	/* Now set up the port to do the capture (clock was enabled earlier)
	start by setting the alternate function for Port C bit 4 (WT0CCP0) */
	HWREG(GPIO_PORTC_BASE+GPIO_O_AFSEL) |= BIT4HI;

	/* Then, map bit 4's alternate function to WT0CCP0 */
	/* 7 is the mux value to select WT0CCP0, 16 to shift it over to the
	right nibble for bit 4 (4 bits/nibble * 4 bits) */
	HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL) = 
		(HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL) & 0xfff0ffff) + (7<<16);

	/* Enable pin on Port C for digital I/O */
	HWREG(GPIO_PORTC_BASE+GPIO_O_DEN) |= BIT4HI;
	
	/* Make pin 4 on Port C into an input */
	HWREG(GPIO_PORTC_BASE+GPIO_O_DIR) &= BIT4LO;

	/* Back to the timer to enable a local capture interrupt */
	maskBeaconInterrupt(true);

	/* Enable the Timer A in Wide Timer 0 interrupt in the NVIC */
	/* It is interrupt number 94 so appears in EN2 at bit 30 */
	HWREG(NVIC_EN2) |= BIT30HI;

	/* make sure interrupts are enabled globally */
	__enable_irq();

	/* now kick the timer off by enabling it and enabling the timer to
	stall while stopped by the debugger */
	HWREG(WTIMER0_BASE+TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TASTALL);
}

void maskBeaconInterrupt(bool maskStatus){
	if(maskStatus) HWREG(WTIMER0_BASE+TIMER_O_IMR) &= ~TIMER_IMR_CAEIM;
		else HWREG(WTIMER0_BASE+TIMER_O_IMR) |= TIMER_IMR_CAEIM;
}

/**
* Calculates the time between consecutive rising edges on PC4. This is
* the interrupt service routine (ISR).
*/
void BeaconSenseResponse(void) {
	//printf("BSR\r\n"); //****************************************************************************
	uint32_t thisCapture;
	/* Start by clearing the source of the interrupt, the input capture event */
	HWREG(WTIMER0_BASE+TIMER_O_ICR) = TIMER_ICR_CAECINT;
	thisCapture = HWREG(WTIMER0_BASE+TIMER_O_TAR);
	period = thisCapture - lastCapture; // add to periodSum
		/* Update lastCapture to prepare for the next edge */  
	lastCapture = thisCapture;
	
	/* Send a BeaconSensed event */
	
	if(period >=targetPeriodMinus && period <= targetPeriodPlus){
		ES_Event ThisEvent;
	  ThisEvent.EventType = BeaconDetected;
		PostStrategySM(ThisEvent);
		/* Mask the interrupt so that it doesn't call the ISR again */
		maskBeaconInterrupt(true);
	}
	//printf("B\r\n");
}
