#include <stdio.h>
#include <stdint.h>

#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_pwm.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "bitdefs.h"
#include "FollowTape.h"
#include "PWMDemo.h"

#define ALL_BITS (0xff<<2)

/* Drive motors */
#define PWMTicksPerUS 1250000 // = 40000000/32
#define PeriodInUS 300

/* Shooting motor */
#define PWM_TICKS_PER_MS 1250 // = 40000/32
#define PWM_FREQ_SHOOT 1000 // PWM frequency [Hz]
#define BITS_PER_NIBBLE 4
/* 40000 ticks per [ms] assumes a 40 Mhz clock; use SysClk/32 for PWM */
#define PWM_FREQ 50 // PWM frequency [Hz]
#define BitsPerNibble 4


/* Prototypes */
static void initDirectionPins(void);
static void setPositionBack(float duty);
static void setPositionFront(float duty);


// PWM - M0PWM0/1
void InitPWMDrive( void ){
   volatile uint32_t Dummy; // use volatile to avoid over-optimization
  
// start by enabling the clock to the PWM Module (PWM0)
  HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R0;

// enable the clock to Port B  
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;

// Select the PWM clock as System Clock/32
  HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) |
    (SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_32);
  
// make sure that the PWM module clock has gotten going
	while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R0) != SYSCTL_PRPWM_R0);

// disable the PWM while initializing
  HWREG( PWM0_BASE+PWM_O_0_CTL ) = 0;

// program generator A to go to 0 at rising comare A, 1 on falling compare A  
  HWREG( PWM0_BASE+PWM_O_0_GENA) = 
    (PWM_0_GENA_ACTCMPAU_ZERO | PWM_0_GENA_ACTCMPAD_ONE );

// program generator B to go to 0 at rising comare B, 1 on falling compare B  
  HWREG( PWM0_BASE+PWM_O_0_GENB) = 
    (PWM_0_GENA_ACTCMPBU_ZERO | PWM_0_GENA_ACTCMPBD_ONE );

// Set the PWM period. Since we are counting both up & down, we initialize
// the load register to 1/2 the desired total period
  HWREG( PWM0_BASE+PWM_O_0_LOAD) = (((PeriodInUS * PWMTicksPerUS))>>1);
  
// Set the initial Duty cycle on A to 50% by programming the compare value
// to 1/2 the period to count up (or down) 
  HWREG( PWM0_BASE+PWM_O_0_CMPA) = ((PeriodInUS * PWMTicksPerUS))>>2;

// Set the initial Duty cycle on B to 25% by programming the compare value
// to 1/4 the period  
  HWREG( PWM0_BASE+PWM_O_0_CMPB) = ((PeriodInUS * PWMTicksPerUS))>>3;

// set changes to the PWM output Enables to be locally syncronized to a 
// zero count
  HWREG(PWM0_BASE+PWM_O_ENUPD) =  (HWREG(PWM0_BASE+PWM_O_ENUPD) & 
      ~(PWM_ENUPD_ENUPD0_M | PWM_ENUPD_ENUPD1_M)) |
      (PWM_ENUPD_ENUPD0_LSYNC | PWM_ENUPD_ENUPD1_LSYNC);

// enable the PWM outputs
  HWREG( PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM1EN | PWM_ENABLE_PWM0EN);

// now configure the Port B pins to be PWM outputs
// start by selecting the alternate function for PB6 & 7
  HWREG(GPIO_PORTB_BASE+GPIO_O_AFSEL) |= (BIT7HI | BIT6HI);

// now choose to map PWM to those pins, this is a mux value of 4 that we
// want to use for specifying the function on bits 6 & 7
  HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) = 
    (HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) & 0x00fffff) + (4<<(7*BitsPerNibble)) +
      (4<<(6*BitsPerNibble));

// Enable pins 6 & 7 on Port B for digital I/O
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (BIT7HI | BIT6HI);
	
// make pins 6 & 7 on Port B into outputs
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= (BIT7HI |BIT6HI);
  
// set the up/down count mode and enable the PWM generator
  HWREG(PWM0_BASE+ PWM_O_0_CTL) |= (PWM_0_CTL_MODE | PWM_0_CTL_ENABLE);
	
	initDirectionPins(); // initialize direction pins

}


//left A, Right B
//Sets PWMDuty for CMPA on Channel 0
//Sets PWMDuty for CMPA on Channel 0
void SetPWMDutyDrive(float NewDutyA, float NewDutyB) {
	/* Motor A */
  if(NewDutyA<0) {
    HWREG(GPIO_PORTA_BASE +(GPIO_O_DATA + ALL_BITS)) |= (BIT6HI);
    HWREG(PWM0_BASE+PWM_O_ENABLE) |= PWM_ENABLE_PWM0EN; 
		HWREG(PWM0_BASE+PWM_O_0_CMPA) = (uint32_t)( HWREG(PWM0_BASE+PWM_O_0_LOAD)*(-NewDutyA)/100 );
  } else if(NewDutyA == 0){
		HWREG(GPIO_PORTA_BASE +(GPIO_O_DATA + ALL_BITS))&= ~(BIT6HI);
		HWREG( PWM0_BASE+PWM_O_0_CMPA) = 0;
		HWREG( PWM0_BASE+PWM_O_ENABLE) &= ~PWM_ENABLE_PWM0EN;
	} else if(NewDutyA>0){
    HWREG(GPIO_PORTA_BASE +(GPIO_O_DATA + ALL_BITS)) &= ~(BIT6HI);
		HWREG(PWM0_BASE+PWM_O_ENABLE) |= PWM_ENABLE_PWM0EN; 
		HWREG( PWM0_BASE+PWM_O_0_CMPA) = (uint32_t)( HWREG(PWM0_BASE+PWM_O_0_LOAD)*NewDutyA/100 );
	}
  
	/* Motor B */
  if(NewDutyB<0) {
    HWREG(GPIO_PORTA_BASE +(GPIO_O_DATA + ALL_BITS)) |= (BIT7HI);
    HWREG(PWM0_BASE+PWM_O_ENABLE) |= PWM_ENABLE_PWM1EN; 
		HWREG( PWM0_BASE+PWM_O_0_CMPB) = (uint32_t)( HWREG(PWM0_BASE+PWM_O_0_LOAD)*(-NewDutyB)/100 );
  } else if(NewDutyB == 0){ 
		HWREG(GPIO_PORTA_BASE +(GPIO_O_DATA + ALL_BITS))&= ~(BIT7HI);	  
		HWREG(PWM0_BASE+PWM_O_0_CMPB) = 0;
		HWREG(PWM0_BASE+PWM_O_ENABLE) &= ~PWM_ENABLE_PWM1EN;
	} else if(NewDutyB>0) {
		HWREG(GPIO_PORTA_BASE +(GPIO_O_DATA + ALL_BITS))&= ~(BIT7HI);
		HWREG(PWM0_BASE+PWM_O_ENABLE) |= PWM_ENABLE_PWM1EN; 
		HWREG(PWM0_BASE+PWM_O_0_CMPB) = (uint32_t)( HWREG(PWM0_BASE+PWM_O_0_LOAD)*NewDutyB/100 );
  }   
}



static void initDirectionPins(void) {
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R0;
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R0) != SYSCTL_PRGPIO_R0);
	HWREG(GPIO_PORTA_BASE+ GPIO_O_DEN)|= (BIT6HI|BIT7HI);
	HWREG(GPIO_PORTA_BASE+ GPIO_O_DIR)|= (BIT6HI|BIT7HI);
	HWREG(GPIO_PORTA_BASE +(GPIO_O_DATA + ALL_BITS))&= ~(BIT6HI|BIT7HI);
}







/**************************************************************************************/
//																			SHOOTING                                      //

void InitPWMShooter(void){
	/* Start by enabling the clock to the PWM Module (PWM0) */
	HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R0;

	/* Enable the clock to Port B */  
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;

	/* Select the PWM clock as System Clock/32 */
	HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) |
    (SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_32);
  
	/* Make sure that the PWM module clock has gotten going */
	while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R0) != SYSCTL_PRPWM_R0);

	/* Disable the PWM while initializing */
	HWREG(PWM0_BASE+PWM_O_1_CTL) = 0; // PB4 is on PWM Generator 1

	/* Program generator A to go to 0 at rising (up) compare A, 1 on falling (down) compare A */  
	HWREG(PWM0_BASE+PWM_O_1_GENA) = (PWM_1_GENA_ACTCMPAU_ZERO | PWM_1_GENA_ACTCMPAD_ONE);

	/* Set the PWM period. Since we are counting both up & down, we initialize
	the load register to 1/2 the desired total period */
	HWREG(PWM0_BASE+PWM_O_1_LOAD) = ((1000*PWM_TICKS_PER_MS)/(PWM_FREQ)) >> 1;
  
	/* Set the initial duty cycle on A to 50% by programming the compare value
	to 1/2 the period to count up (or down) */
	HWREG(PWM0_BASE+PWM_O_1_CMPA) = ((1000*PWM_TICKS_PER_MS)/(PWM_FREQ))/4;

	/* Set changes to the PWM output enables to be locally synchronized to a 
	zero count */
	HWREG(PWM0_BASE+PWM_O_ENUPD) = (HWREG(PWM0_BASE+PWM_O_ENUPD) & 
      ~(PWM_ENUPD_ENUPD2_M)) | (PWM_ENUPD_ENUPD2_LSYNC);

	/* Enable the PWM outputs */
	HWREG(PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM2EN);

	/* Now configure the Port B pin to be PWM output
	start by selecting the alternate function for PB4 */
	HWREG(GPIO_PORTB_BASE+GPIO_O_AFSEL) |= (BIT4HI);

	/* Now choose to map PWM to those pins, this is a mux value of 4 that we
	want to use for specifying the function on bit 4 */
	HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) = 
    (HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) & 0xff0ffff) + (4 << (4*BITS_PER_NIBBLE));

	/* Enable pin 4 on Port B for digital I/O */
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (BIT4HI);
	
	/* Make pin 4 on Port B into output */
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= (BIT4HI);
  
	/* Set the up/down count mode and enable the PWM generator */
	HWREG(PWM0_BASE+PWM_O_1_CTL) |= (PWM_1_CTL_MODE | PWM_1_CTL_ENABLE);
	
}



void SetPWMDutyShooter(float NewDuty) {
	if (NewDuty == 0) {
		/* Set the duty cycle to 0% by programming the compare value
		to 0. However, since the CmpADn action (set to one) wins, we also
		need to disable the output */
		HWREG(PWM0_BASE+PWM_O_1_CMPA) = 0;
		HWREG(PWM0_BASE+PWM_O_ENABLE) &= ~PWM_ENABLE_PWM2EN;
	} else if (NewDuty == 100) {
		/* Set the Duty cycle on B to 100% by programming the compare value
		to the load value. Since the CmpBDn action (set to one) wins, we get 100% */
		HWREG(PWM0_BASE+PWM_O_ENABLE) |= PWM_ENABLE_PWM2EN;
		HWREG(PWM0_BASE+PWM_O_1_CMPA) = HWREG(PWM0_BASE+PWM_O_1_LOAD);
	} else {
		HWREG(PWM0_BASE+PWM_O_ENABLE) |= PWM_ENABLE_PWM2EN;
		HWREG(PWM0_BASE+PWM_O_1_CMPA) = (uint32_t)(HWREG(PWM0_BASE+PWM_O_1_LOAD)*NewDuty/100);
	}
}


void InitPWMGates(void){
	/* Start by enabling the clock to the PWM Module (PWM0) */
	HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R0;

	/* Enable the clock to Port E */  
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R4;

	/* Select the PWM clock as System Clock/32 */
	HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) |
    (SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_32);
  
	/* Make sure that the PWM module clock has gotten going */
	while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R0) != SYSCTL_PRPWM_R0);

	/* Disable the PWM while initializing */
	HWREG(PWM0_BASE+PWM_O_2_CTL) = 0; // PE4 is on PWM Generator 2

	/* Program generator A to go to 0 at rising (up) compare A, 1 on falling (down) compare A */  
	HWREG(PWM0_BASE+PWM_O_2_GENA) = (PWM_2_GENA_ACTCMPAU_ZERO | PWM_2_GENA_ACTCMPAD_ONE);
	HWREG(PWM0_BASE+PWM_O_2_GENB) = (PWM_2_GENB_ACTCMPBU_ZERO | PWM_2_GENB_ACTCMPBD_ONE);

	/* Set the PWM period. Since we are counting both up & down, we initialize
	the load register to 1/2 the desired total period */
	HWREG(PWM0_BASE+PWM_O_2_LOAD) = ((1000*PWM_TICKS_PER_MS)/(PWM_FREQ)) >> 1;
  
	/* Set the initial duty cycle on A to 50% by programming the compare value
	to 1/2 the period to count up (or down) */
	HWREG(PWM0_BASE+PWM_O_2_CMPA) = ((1000*PWM_TICKS_PER_MS)/(PWM_FREQ))/4; //(PERIOD_IN_MS*PWM_TICKS_PER_MS) >> 2;
	HWREG(PWM0_BASE+PWM_O_2_CMPB) = ((1000*PWM_TICKS_PER_MS)/(PWM_FREQ))/4;

	/* Set changes to the PWM output enables to be locally synchronized to a 
	zero count */
	HWREG(PWM0_BASE+PWM_O_ENUPD) = (HWREG(PWM0_BASE+PWM_O_ENUPD) & 
      ~(PWM_ENUPD_ENUPD4_M|PWM_ENUPD_ENUPD5_M)) | (PWM_ENUPD_ENUPD4_LSYNC|PWM_ENUPD_ENUPD5_LSYNC);

	/* Enable the PWM outputs */
	//HWREG( PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM1EN | PWM_ENABLE_PWM0EN);
	HWREG(PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM4EN);
	HWREG(PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM5EN);

	/* Now configure the Port E pin to be PWM outputs
	start by selecting the alternate function for PE4 & PE5 */
	HWREG(GPIO_PORTE_BASE+GPIO_O_AFSEL) |= (BIT4HI);
	HWREG(GPIO_PORTE_BASE+GPIO_O_AFSEL) |= (BIT5HI);

	/* Now choose to map PWM to those pins, this is a mux value of 4 that we
	want to use for specifying the function on bit 4 */
	HWREG(GPIO_PORTE_BASE+GPIO_O_PCTL) = 
    (HWREG(GPIO_PORTE_BASE+GPIO_O_PCTL) & 0xff0ffff) + (4 << (4*BITS_PER_NIBBLE));
	HWREG(GPIO_PORTE_BASE+GPIO_O_PCTL) = 
    (HWREG(GPIO_PORTE_BASE+GPIO_O_PCTL) & 0xf0fffff) + (4 << (5*BITS_PER_NIBBLE));

	/* Enable pin 4 & 5 on Port E for digital I/O */
	HWREG(GPIO_PORTE_BASE+GPIO_O_DEN) |= (BIT4HI);
	HWREG(GPIO_PORTE_BASE+GPIO_O_DEN) |= (BIT5HI);
	
	/* Make pin 4 & 5 on Port E into output */
	HWREG(GPIO_PORTE_BASE+GPIO_O_DIR) |= (BIT4HI);
	HWREG(GPIO_PORTE_BASE+GPIO_O_DIR) |= (BIT5HI);
  
	/* Set the up/down count mode and enable the PWM generator */
	HWREG(PWM0_BASE+PWM_O_2_CTL) |= (PWM_2_CTL_MODE | PWM_2_CTL_ENABLE);
}
void openGateFront(void){
		setPositionFront(0);
}

void closeGateFront(void) {
		setPositionFront(6);
}

void openGateBack(void) {
		setPositionBack(5);
}

void closeGateBack(void) {
		setPositionBack(10);
}

/**
* Sets the PWM duty cycle for M0PWM5 (PE5).
* This function is for active-brake control.
* @param duty: duty cycle (between 0 and 100)
*/
static void setPositionBack(float duty) {
	duty = duty+3;
	if (duty == 0) {
		/* Set the duty cycle to 0% by programming the compare value
		to 0. However, since the CmpADn action (set to one) wins, we also
		need to disable the output */
		HWREG(PWM0_BASE+PWM_O_2_CMPB) = 0;
		HWREG(PWM0_BASE+PWM_O_ENABLE) &= ~PWM_ENABLE_PWM5EN;
	} else if (duty == 100) {
		/* Set the Duty cycle on B to 100% by programming the compare value
		to the load value. Since the CmpBDn action (set to one) wins, we get 100% */
		HWREG(PWM0_BASE+PWM_O_ENABLE) |= PWM_ENABLE_PWM5EN;
		HWREG(PWM0_BASE+PWM_O_2_CMPB) = HWREG(PWM0_BASE+PWM_O_2_LOAD);
	} else {
		HWREG(PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM4EN|PWM_ENABLE_PWM5EN);
		HWREG(PWM0_BASE+PWM_O_2_CMPB) = (uint32_t)(HWREG(PWM0_BASE+PWM_O_2_LOAD)*duty/100);
	}
}

static void setPositionFront(float duty) {
	duty = duty+3;
	if (duty == 0) {
		/* Set the duty cycle to 0% by programming the compare value
		to 0. However, since the CmpADn action (set to one) wins, we also
		need to disable the output */
		HWREG(PWM0_BASE+PWM_O_2_CMPA) = 0;
		HWREG(PWM0_BASE+PWM_O_ENABLE) &= ~PWM_ENABLE_PWM4EN;
	} else if (duty == 100) {
		/* Set the Duty cycle on B to 100% by programming the compare value
		to the load value. Since the CmpBDn action (set to one) wins, we get 100% */
		HWREG(PWM0_BASE+PWM_O_ENABLE) |= PWM_ENABLE_PWM4EN;
		HWREG(PWM0_BASE+PWM_O_2_CMPA) = HWREG(PWM0_BASE+PWM_O_2_LOAD);
	} else {
		HWREG(PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM4EN|PWM_ENABLE_PWM5EN);
		HWREG(PWM0_BASE+PWM_O_2_CMPA) = (uint32_t)(HWREG(PWM0_BASE+PWM_O_2_LOAD)*duty/100);
	}
}



