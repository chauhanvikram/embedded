
#include <stdint.h>

#include "inc/hw_memmap.h"
#include "inc/hw_timer.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "bitdefs.h"

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"

#include "ADMulti.h"
#include "PWMDemo.h"
#include "FollowTape.h"

#define TicksPerMS 40000 // 40000 ticks per [ms] assumes a 40 MHz clock
#define TIMER_IN_MS 20
#define NUM_TAPE_SENSORS 2



/* Note: Height between floor and tape sensors is 0.164" */
#define K_P 0.08// proportional gain
#define K_D 0.16 // derivative gain.

/* Module-level variables */
static uint32_t results[NUM_TAPE_SENSORS]; // Array to hold the A/D converter results from the tape sensors
static int tapeSensor1;
static int tapeSensor2;
static uint16_t nom_motor_A = 35;
static uint16_t nom_motor_B = 35;

// we will use Timer A in Wide Timer 3 to generate the interrupt

void setNominalTapeFollowDuty(uint16_t nom_A, uint16_t nom_B){
	nom_motor_A = nom_A;
	nom_motor_B = nom_B;
	
}
void InitFollowTape( void ){
	
	SetPWMDutyDrive(nom_motor_A, nom_motor_B);
	//printf("\r\n In Following Tape Init");
	ADC_MultiInit(NUM_TAPE_SENSORS);
	
  volatile uint32_t Dummy; // use volatile to avoid over-optimization
  
  // start by enabling the clock to the timer (Wide Timer 3)
  HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R3;
	// kill a few cycles to let the clock get going
	Dummy = HWREG(SYSCTL_RCGCGPIO);
  
  // make sure that timer (Timer A) is disabled before configuring
  HWREG(WTIMER3_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
  
	// set it up in 32bit wide (individual, not concatenated) mode
	// the constant name derives from the 16/32 bit timer, but this is a 32/64
	// bit timer so we are setting the 32bit mode
  HWREG(WTIMER3_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;
  
	// set up timer A in periodic mode so that it repeats the time-outs
  HWREG(WTIMER3_BASE+TIMER_O_TAMR) = 
     (HWREG(WTIMER3_BASE+TIMER_O_TAMR)& ~TIMER_TAMR_TAMR_M)| 
     TIMER_TAMR_TAMR_PERIOD;

	// set timeout to some number of milliseconds
  HWREG(WTIMER3_BASE+TIMER_O_TAILR) = TicksPerMS*TIMER_IN_MS;

	// enable a local timeout interrupt
  //interruptMask(true);
	followTapeInterruptMask(true);

	// enable the Timer A in Wide Timer 3 interrupt in the NVIC
	// it is interrupt number 100 so apppears in EN3 at bit 4
	/* Refer to pp. 105 and 142 in datasheet */
  HWREG(NVIC_EN3) = BIT4HI;

	// make sure interrupts are enabled globally
  __enable_irq();
  
	// now kick the timer off by enabling it and enabling the timer to
	// stall while stopped by the debugger
	
  HWREG(WTIMER3_BASE+TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TASTALL);

}

int getTapeSensor1Val(void) {
	return tapeSensor1;
}

int getTapeSensor2Val(void) {
	return tapeSensor2;
}

//True = interrupt is masked
 void followTapeInterruptMask(bool interruptMaskStatus) {
	if (interruptMaskStatus) HWREG(WTIMER3_BASE+TIMER_O_IMR) &= ~TIMER_IMR_TATOIM;
	else HWREG(WTIMER3_BASE+TIMER_O_IMR) |= TIMER_IMR_TATOIM;
}


void FollowTapeResponse(void) {
	/* Start by clearing the source of the interrupt */
  HWREG(WTIMER3_BASE+TIMER_O_ICR) = TIMER_ICR_TATOCINT;
	
	static int oldError = 0; // previous error

	/* Read the A/D converter results and store in variables */
	ADC_MultiRead(results);
	//int tapeSensor1 = (int)results[0];
	//int tapeSensor2 = (int)results[1];
	tapeSensor1 = (int)results[0];
	tapeSensor2 = (int)results[1];
	
	/* Error is the difference in Vout from sensors */
	int error = tapeSensor1 - tapeSensor2;
	//int error = tapeSensor2 - tapeSensor1;
	
	/* PD control */
	float adjustPWM = K_P*error + K_D*(error - oldError);
	oldError = error; // reset oldError
	
	//printf("\r\nError: %d, AdjustPWM: %f", error, adjustPWM);
	//printf("L: %d R: %d\r\n", tapeSensor1, tapeSensor2);
	//printf("a\r\n");
	
	if (adjustPWM < 0) {
		/* Turn left */
		/* To turn left, turn down motor A speed */
		adjustPWM = -adjustPWM; // force adjustPWM to be positive
		float motorA_PWM = nom_motor_A - adjustPWM;
		if (motorA_PWM < 0) motorA_PWM = 0;
		SetPWMDutyDrive(motorA_PWM, nom_motor_B);
	} else if (adjustPWM > 0) {
		/* Turn right */
		/* To turn right, turn down motor B speed */
		float motorB_PWM = nom_motor_B - adjustPWM;
		if (motorB_PWM < 0) motorB_PWM = 0;
		SetPWMDutyDrive(nom_motor_A, motorB_PWM);
	} else if (adjustPWM == 0) {
		SetPWMDutyDrive(nom_motor_A, nom_motor_B);
	}
	
	//SetPWMDutyDrive(-99, -99); // DEBUG

}
