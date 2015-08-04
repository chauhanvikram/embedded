



/**
* SpeedControl.c
* This module contains the PID controller function for controlling
* the speed of the motor.
*/

#include <stdint.h>

#include "ES_Configure.h"
#include "ES_Framework.h"

#include "inc/hw_memmap.h"
#include "inc/hw_timer.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "bitdefs.h"

#include "SpeedControl.h"
#include "PWMDemo.h"
#include "EncoderLeft.h"
#include "EncoderRight.h"

/* 40,000 ticks per [ms] assumes a 40 MHz clock */
#define TICKS_PER_MS 40000
#define PERIOD 20 // update period for closed-loop control [ms]
#define PID_PRIO 5 // set priority to be 1 (i.e., "lower" than interrupt with priority 0)
#define PID_PRIO_REM 2 // Interrupt 102/4 = 25 with 2 remainder
#define TICKS_PER_MIN 2400000000 // number of clock ticks per [min]

#define SPEED_SP 50 // setpoint speed (temporary) [rpm]
#define K_P 0.2 // proportional (P) gain
#define K_I 0.03 // integral (I) gain 0.01
#define MAX_DUTY 99 // maximum duty cycle (i.e., 99%)
#define MAX_INT 100 // maximum error accumulation allowed

static float leftIntSum = 0; // variable which accumulates left motor error
static float rightIntSum = 0; // variable which accumulates right motor error
static uint8_t speedSetpoint = 0;
static bool reverseDirection = false;
//static uint32_t period = 0;
//static uint8_t equalCount = 0;

/**
* Initializes Timer A in Wide Timer 4 to generate an interrupt
* periodically.
*/
void initSpeedControl(void) {
	volatile uint32_t dummy; // use volatile to avoid over-optimization
  
	/* Start by enabling the clock to the timer (Wide Timer 4) */
	HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R4;
	/* Kill a few cycles to let the clock get going */
	dummy = HWREG(SYSCTL_RCGCGPIO);
  
	/* Make sure that timer (Timer A) is disabled before configuring */
	HWREG(WTIMER4_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
  
	/* Set it up in 32-bit wide (individual, not concatenated) mode */
	/* The constant name derives from the 16/32 bit timer, but this is a 32/64
	bit timer so we are setting the 32-bit mode */
	HWREG(WTIMER4_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;
  
	/* Set up timer A in periodic mode so that it repeats the time-outs */
	HWREG(WTIMER4_BASE+TIMER_O_TAMR) = 
     (HWREG(WTIMER4_BASE+TIMER_O_TAMR) & ~TIMER_TAMR_TAMR_M)| 
     TIMER_TAMR_TAMR_PERIOD;

	/* Set timeout to PERIOD ms */
	HWREG(WTIMER4_BASE+TIMER_O_TAILR) = TICKS_PER_MS*PERIOD;

	/* Enable a local timeout interrupt */
	maskSpeedControlInterrupt(true);

	/* Enable the Timer A in Wide Timer 4 interrupt in the NVIC */
	/* It is interrupt number 102 so appears in EN3 at bit 6 */
	/* Refer to pp. 105 and 142 in datasheet */
	HWREG(NVIC_EN3) = BIT6HI;

	/* Set the priority to be "lower" (i.e., > 0). Interrupt 102 is INTC in PRI25 */
	/* Refer to p. 152 in datasheet */
	HWREG(NVIC_PRI25) = (HWREG(NVIC_PRI25) & ~NVIC_PRI25_INTC_M) | ((PID_PRIO << 5) << (PID_PRIO_REM*8));

	/* Make sure interrupts are enabled globally */
	__enable_irq();
  
	/* Now kick the timer off by enabling it and enabling the timer to
	stall while stopped by the debugger */
	HWREG(WTIMER4_BASE+TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TASTALL);
}


void maskSpeedControlInterrupt(bool masked){
	if(masked) HWREG(WTIMER4_BASE+TIMER_O_IMR) &= ~TIMER_IMR_TATOIM;
	else  HWREG(WTIMER4_BASE+TIMER_O_IMR) |= TIMER_IMR_TATOIM;
}

void setSpeedSetpoint(uint8_t speed) {
	speedSetpoint = speed;
}

void setControlDirection(bool reverse){
	reverseDirection = reverse;
}

/**
* Sends the PI command signal to the motors. This is
* the interrupt service routine for WT4TA.
*/
void speedControlResponse(void) {
	//printf("SCR\r\n"); //****************************************************************************
	/* Start by clearing the source of the interrupt */
	HWREG(WTIMER4_BASE+TIMER_O_ICR) = TIMER_ICR_TATOCINT;
	
	/* Calculate speeds */
	float leftSpeed = TICKS_PER_MIN/(ENCODER_PPR*getPeriodLeft());
	float rightSpeed = TICKS_PER_MIN/(ENCODER_PPR*getPeriodRight());
	
	/* Compute errors */
	//float leftError = SPEED_SP - leftSpeed;
	//float rightError = SPEED_SP - rightSpeed;
	float leftError = speedSetpoint - leftSpeed;
	float rightError = speedSetpoint - rightSpeed;
	
	/* Integral terms */
	leftIntSum += leftError;
	rightIntSum += rightError;
	
	/* Compute control signals (signed duty cycles) */
	float leftControlSig = (float)K_P*leftError + (float)K_I*leftIntSum;
	float rightControlSig = (float)K_P*rightError + (float)K_I*rightIntSum;
	
	/* Saturate control signals, if necessary */
	if (leftControlSig > MAX_DUTY) {
		leftControlSig = MAX_DUTY;
		leftIntSum -= leftError; // prevent wind-up
	} else if (leftControlSig < 0) {
		leftControlSig = 0;
		leftIntSum -= leftError; // prevent wind-up
	}
	
	if (rightControlSig > MAX_DUTY) {
		rightControlSig = MAX_DUTY;
		rightIntSum -= rightError; // prevent wind-up
	} else if (rightControlSig < 0) {
		rightControlSig = 0;
		rightIntSum -= rightError; // prevent wind-up
	}

	/* Set the new duty cycles */
	if(reverseDirection)SetPWMDutyDrive(-leftControlSig, -rightControlSig);
	else SetPWMDutyDrive(leftControlSig, rightControlSig);
	//SetPWMDuty(leftControlSig, 0);
	//SetPWMDuty(0, rightControlSig);
	
	//printf("x\r\n");
}




/***************************************************************************************************************
//																									ORIENTATION CONTTROL
*********************************************************************************************************************


 SpeedControl.c
This module contains the PID controller function for controlling
 the speed of the motor.


//#include <stdint.h>

//#include "ES_Configure.h"
//#include "ES_Framework.h"

//#include "inc/hw_memmap.h"
//#include "inc/hw_timer.h"
//#include "inc/hw_sysctl.h"
//#include "inc/hw_nvic.h"
//#include "inc/hw_types.h"
//#include "bitdefs.h"

//#include "SpeedControl.h"
//#include "PWMDemo.h"
//#include "EncoderLeft.h"
//#include "ComServiceSM.h"
//#include "EncoderRight.h"

 40,000 ticks per [ms] assumes a 40 MHz clock */
//#define TICKS_PER_MS 40000
//#define PERIOD 100 // update period for closed-loop control [ms]
//#define PID_PRIO 5 // set priority to be 1 (i.e., "lower" than interrupt with priority 0)
//#define PID_PRIO_REM 2 // Interrupt 102/4 = 25 with 2 remainder
//#define TICKS_PER_MIN 2400000000 // number of clock ticks per [min]

//#define SPEED_SP 50 // setpoint speed (temporary) [rpm]
//#define K_P 0.2 // proportional (P) gain
//#define K_D 0 // integral (I) gain 0.01
//#define MAX_DUTY 99 // maximum duty cycle (i.e., 99%)
//#define MAX_INT 100 // maximum error accumulation allowed



////static uint32_t period = 0;
////static uint8_t equalCount = 0;
//static uint8_t nomDutyMotorA;
//static uint8_t nomDutyMotorB;
//static bool trackX = false;
//static uint16_t coordinateToTrack;


//void initSpeedControl(void) {
//	volatile uint32_t dummy; // use volatile to avoid over-optimization
//  
//	/* Start by enabling the clock to the timer (Wide Timer 4) */
//	HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R4;
//	/* Kill a few cycles to let the clock get going */
//	dummy = HWREG(SYSCTL_RCGCGPIO);
//  
//	/* Make sure that timer (Timer A) is disabled before configuring */
//	HWREG(WTIMER4_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
//  
//	/* Set it up in 32-bit wide (individual, not concatenated) mode */
//	/* The constant name derives from the 16/32 bit timer, but this is a 32/64
//	bit timer so we are setting the 32-bit mode */
//	HWREG(WTIMER4_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;
//  
//	/* Set up timer A in periodic mode so that it repeats the time-outs */
//	HWREG(WTIMER4_BASE+TIMER_O_TAMR) = 
//     (HWREG(WTIMER4_BASE+TIMER_O_TAMR) & ~TIMER_TAMR_TAMR_M)| 
//     TIMER_TAMR_TAMR_PERIOD;

//	/* Set timeout to PERIOD ms */
//	HWREG(WTIMER4_BASE+TIMER_O_TAILR) = TICKS_PER_MS*PERIOD;

//	/* Enable a local timeout interrupt */
//	maskSpeedControlInterrupt(true);

//	/* Enable the Timer A in Wide Timer 4 interrupt in the NVIC */
//	/* It is interrupt number 102 so appears in EN3 at bit 6 */
//	/* Refer to pp. 105 and 142 in datasheet */
//	HWREG(NVIC_EN3) = BIT6HI;

//	/* Set the priority to be "lower" (i.e., > 0). Interrupt 102 is INTC in PRI25 */
//	/* Refer to p. 152 in datasheet */
//	HWREG(NVIC_PRI25) = (HWREG(NVIC_PRI25) & ~NVIC_PRI25_INTC_M) | ((PID_PRIO << 5) << (PID_PRIO_REM*8));

//	/* Make sure interrupts are enabled globally */
//	__enable_irq();
//  
//	/* Now kick the timer off by enabling it and enabling the timer to
//	stall while stopped by the debugger */
//	HWREG(WTIMER4_BASE+TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TASTALL);
//}


//void maskSpeedControlInterrupt(bool masked){
//	if(masked) HWREG(WTIMER4_BASE+TIMER_O_IMR) &= ~TIMER_IMR_TATOIM;
//	else  HWREG(WTIMER4_BASE+TIMER_O_IMR) |= TIMER_IMR_TATOIM;
//}

//void setNominalMotorDuty(uint8_t nomMotorA, uint8_t nomMotorB){
//	nomDutyMotorA = nomMotorA;
//	nomDutyMotorB = nomMotorB;
//}

////if true use x, if false use y
//void setTargetCoordinate(bool xTrack, uint16_t coordinate){
//	trackX = xTrack;
//	coordinateToTrack = coordinate;	
//}

/////**
////* Sends the PI command signal to the motors. This is
////* the interrupt service routine for WT4TA.
////*/
//void speedControlResponse(void) {
//	//printf("SCR\r\n"); //****************************************************************************
//	/* Start by clearing the source of the interrupt */
//	HWREG(WTIMER4_BASE+TIMER_O_ICR) = TIMER_ICR_TATOCINT;
//	
//	static int oldError = 0;
//	
//	/* Error is the difference in Vout from sensors */
//	int error = 0;
//	uint16_t xVal = 0;
//	if(trackX){
//		xVal = getOurKartX();
//		error = -(coordinateToTrack - xVal);
//	}
//	else  error = -(coordinateToTrack - getOurKartY());
//	if(abs(error)<5) error = 0;
//	printf("Error = %i, Coordinate To Track = %i, Our X = %i\r\n", error, coordinateToTrack, xVal);

//	
//	/* PD control */
//	float adjustPWM = K_P*error + K_D*(error - oldError);
//	oldError = error; // reset oldError

//	
//	//printf("\r\nError: %d, AdjustPWM: %f", error, adjustPWM);
//	//printf("L: %d R: %d\r\n", tapeSensor1, tapeSensor2);
//	//printf("a\r\n");
//	
//	if (adjustPWM < 0) {
//		/* Turn left */
//		/* To turn left, turn down motor A speed */
//		adjustPWM = -adjustPWM; // force adjustPWM to be positive
//		float motorA_PWM = nomDutyMotorA - adjustPWM;
//		float motorB_PWM = nomDutyMotorB + adjustPWM;
//		if (motorA_PWM < 0) motorA_PWM = 0;
//		if(motorB_PWM > 100) motorB_PWM = 99;
//		SetPWMDutyDrive(motorA_PWM, motorB_PWM);
//	} else if (adjustPWM > 0) {
//		/* Turn right */
//		/* To turn right, turn down motor B speed */
//		float motorA_PWM = nomDutyMotorA + adjustPWM;
//		float motorB_PWM = nomDutyMotorB - adjustPWM;
//		if (motorB_PWM < 0) motorB_PWM = 0;
//		if(motorA_PWM > 100) motorA_PWM = 99;
//		SetPWMDutyDrive(motorA_PWM, motorB_PWM);
//	} else if (adjustPWM == 0) {
//		SetPWMDutyDrive(nomDutyMotorA, nomDutyMotorB);
//	}
//	
//}
