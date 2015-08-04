/*----------------------------- Include Files -----------------------------*/
// Basic includes for a program using the Events and Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "PWMDemo.h"
#include "FollowTape.h"
#include "Drive.h"
#include "EncoderRight.h"
#include "SpeedControl.h"
#include "EncoderLeft.h"
#include "BeaconSense.h"
#include "ComServiceSM.h"

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
void InitDrive(){
	InitPWMDrive();
	InitFollowTape();
	initSpeedControl();
	initEncoderRight();
	initEncoderLeft();
	initBeaconSense();
}

//TODO
//Implement Beacon Interrupt
//Implement Turn Interrupt with Encoders

//Align with beacon will cause the kart to turn CCW
void AlignWithBeacon(void){
	maskBeaconInterrupt(false); //Unmask beacon interrupt
	TurnKart(0, true);
	//turn until beacon interrupt triggers
	//track degrees turned, get degrees turned (from encoder turn interrrupt)
	//and post as an event TYPE BEACON FOUND with param (degrees turned)
	//store this param in strategySM for reversing turn later
}
//Positive  = CCW, Negative = CW
void TurnKart(float numDegrees, bool alignWithBeacon){
	//maskLeftEncoder(false); //set desired edges for left and right motors
	
	if(alignWithBeacon) {
		SetPWMDutyDrive(-BEACON_TURNING_DUTY, BEACON_TURNING_DUTY);
	}	else {
		float edges = abs(numDegrees) * (float)EDGES_PER_DEGREE;
		countEdges(edges);
		if(numDegrees > 0)SetPWMDutyDrive(-TURN_DUTY, TURN_DUTY);
		else if (numDegrees < 0 )SetPWMDutyDrive(TURN_DUTY, -TURN_DUTY);
	}	
}

void DriveDistance(float numInches,bool reverse){
	//maskRightEncoder(false);       	//unmask  encoder position control interrupt
	//maskLeftEncoder(false); //set desired edges for left and right motors
	float edges = (numInches)* (float)EDGES_PER_INCH;
	
	if(!reverse)DriveForward(DRIVE_DUTY);	
	else DriveReverse(REV_DRIVE_DUTY);
	countEdges(edges);
}

void countEdges(uint32_t edgeCount){
	//printf("%i\r\n", edgeCount);
	//maskRightEncoder(false); 
	maskLeftEncoder(false); 
	startPosControlLeft(edgeCount); // count edges if edge count reached set count to 0, mask interrupt, and post distance reached
	//startPosControlRight(edgeCount);
}

void DriveForward(uint8_t duty){
	SetPWMDutyDrive(duty,duty);
}

void DriveReverse(uint8_t duty){
	SetPWMDutyDrive(-duty,-duty);
}


void enableSpeedControl(bool control, uint8_t RPM_Speed, bool reverse){
	setSpeedSetpoint(RPM_Speed);
	setControlDirection(reverse);
	maskRightEncoder(!control);
	maskLeftEncoder(!control);
	maskSpeedControlInterrupt(!control);
	//unmask encoder interrupt for each motor
	//
}
void StopMotor(void){
	FollowTape(false,0,0);
	enableSpeedControl(false, 0,false);
	SetPWMDutyDrive(0, 0);
}


void FollowTape(bool follow, uint16_t motorADuty, uint16_t motorBDuty){
	setNominalTapeFollowDuty(motorADuty, motorBDuty);
	followTapeInterruptMask(!follow);
}


