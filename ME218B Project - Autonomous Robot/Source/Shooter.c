/*----------------------------- Include Files -----------------------------*/
// Basic includes for a program using the Events and Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "PWMDemo.h"
#include "FollowTape.h"
#include "Drive.h"
#include "ZoneOneSM.h"

#define SHOOTER_PWM_SPEED 31
//#define  SHOOTER_PWM_SPEED 17

//Worked at 3.28V 10%duty, 22% of 15.1V


/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/


/*----------------------------- Module Defines ----------------------------*/
// define constants for the states for this machine
// and any other local defines



/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/


/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************/


//make sure you do this before init drive or you will be very sorry :)

void StopFlyWheel(void){
	SetPWMDutyShooter(0);
}


void StartFlyWheel(void){
	SetPWMDutyShooter(SHOOTER_PWM_SPEED);
}

//if direction is > 0 forward, if < 0 reverse
void ActivateBackGate(int direction){
	if(direction >= 0) openGateBack();
	else closeGateBack(); 
	
}


//if direction is > 0 forward, if < 0 reverse
void ActivateFrontGate(int direction){
	if(direction >= 0) openGateFront();
	else closeGateFront();
	}

void InitShooter(void){
	InitPWMShooter();
	InitPWMGates();
	StopFlyWheel();
	ActivateBackGate(-1);
	ActivateFrontGate(-1);
}

