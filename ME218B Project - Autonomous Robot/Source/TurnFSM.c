/****************************************************************************
 Module
   TurningFSM.c

 Revision
   1.0.1

 Description
   This is a Turning file for implementing flat state machines under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 11:12 jec      revisions for Gen2 framework
 11/07/11 11:26 jec      made the queue static
 10/30/11 17:59 jec      fixed references to CurrentEvent in RunTurningSM()
 10/23/11 18:20 jec      began conversion from SMTurning.c (02/20/07 rev)
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "TurnFSM.h"
#include "StrategySM.h"
#include "ComServiceSM.h"
#include "Drive.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_pwm.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"

#define SAMPLE_TIME 150
#define SAMPLE_MAX 10
#define ERROR_BAND 4
#define TURN_COUNT_MAX 4

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

//static void initDebuggingLEDs(void);
static int determineError(int currentOrientation);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static TurningState_t CurrentState;
static int sampleCount = 0;
static uint16_t targetOrientation; //relative to coordinate system

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;
static uint16_t currentOrientation = 0;
static int error = 0;
static int turnCount = 0;
static int upperThreshold2;//defined to be largest angular distance from target
static int upperThreshold1;
static int lowerThreshold1;
static int lowerThreshold2;//defined to be largest angular distance from target




/*------------------------------ Module Code ------------------------------*/

void setTargetOrientation(uint16_t orientation){
	targetOrientation = orientation;
	
	/* Thresholds */
	upperThreshold1 = targetOrientation;
	
	lowerThreshold1 = targetOrientation - 1;
	if(lowerThreshold1 < 0) lowerThreshold1 += 360;
	
	upperThreshold2 = targetOrientation + ERROR_BAND;
	if(upperThreshold2 >= 360) upperThreshold2 -= 360;
	
	lowerThreshold2 = targetOrientation - ERROR_BAND;
	if(lowerThreshold2 < 0) lowerThreshold2 += 360;
	
}


/****************************************************************************
 Function
     InitTurningFSM


 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 18:55
****************************************************************************/
bool InitTurningFSM ( uint8_t Priority )
{
  ES_Event ThisEvent;
	//initDebuggingLEDs();
	printf("Debgugging LEDs initialized \r\n");
  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = WAITING_TURNING_FSM;
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
  if (ES_PostToService( MyPriority, ThisEvent) == true)
  {
      return true;
  }else
  {
      return false;
  }
}

/****************************************************************************
 Function
     PostTurningFSM

 Parameters
     EF_Event ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostTurningFSM( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}


/****************************************************************************
 Function
    RunTurningFSM

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event RunTurningFSM( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
//	ES_Event TurnFailEvent = {TurnFail,0};

  switch ( CurrentState )
  {
    case WAITING_TURNING_FSM :
			printf("WAITING_TURNING_FSM\r\n");
        if ( ThisEvent.EventType == StartTurn)// only respond to ES_Init
        {
						printf("StartTurn\r\n");
						CurrentState = SAMPLING;
						currentOrientation = 0;
						ES_Timer_InitTimer(SAMPLING_TIMER, SAMPLE_TIME);
						
				}
    break;
	
		case SAMPLING :
				
        switch ( ThisEvent.EventType )
      { 
					case CautionFlag:
						StopMotor();
						CurrentState = WAITING_TURNING_FSM;
					break;
				case RaceOver:
						StopMotor();
						CurrentState = WAITING_TURNING_FSM;
					break;
				case ZoneUpdate:
						printf("ZoneUpdate\r\n");
						StopMotor();
						CurrentState = WAITING_TURNING_FSM;
						//HWREG(GPIO_PORTB_BASE +(GPIO_O_DATA + ALL_BITS))|= (BIT0HI|BIT1HI|BIT2HI);
						//PostStrategySM(TurnFailEvent);
				break;
        case ES_TIMEOUT: 
					//printf("ES_TIMEOUT\r\n");
					sampleCount++;
					if(sampleCount > SAMPLE_MAX){
							ES_Event Event;
							CurrentState = TURNING;
							currentOrientation = currentOrientation/SAMPLE_MAX;
							//printf("Current Orientation Post Sample= %i\r\n", currentOrientation);
							sampleCount = 0;
							if( (currentOrientation >= upperThreshold1 && currentOrientation <= upperThreshold2) ||
								(currentOrientation >= lowerThreshold2 && currentOrientation <= lowerThreshold1) ||
								(currentOrientation >= lowerThreshold1 && currentOrientation <= upperThreshold1)
								|| (turnCount >= TURN_COUNT_MAX)){
								Event.EventType = TurnComplete;
								StopMotor();
								CurrentState = WAITING_TURNING_FSM;
								//printf("Final Current Location: %i at Target Location = %i\r\n", currentOrientation, targetOrientation);
								PostStrategySM(Event);
								turnCount = 0;
							}
							else{
								Event.EventType = SamplingComplete;
								PostTurningFSM(Event);
							}
					}else{
						currentOrientation += getOurKartAbsoluteOrientation();
						ES_Timer_InitTimer(SAMPLING_TIMER, SAMPLE_TIME);
					}
          break;
        
      }  // end switch on CurrentEvent
      break;
    case TURNING :       // If current state is state one
      printf("TURNING\r\n");
			switch ( ThisEvent.EventType )
      {
				
				case CautionFlag:
						StopMotor();
						CurrentState = WAITING_TURNING_FSM;
					break;
				case RaceOver:
						StopMotor();
						CurrentState = WAITING_TURNING_FSM;
				 break;
				
				case ZoneUpdate:
						printf("ZoneUpdate\r\n");
						StopMotor();
						CurrentState = WAITING_TURNING_FSM;
						//HWREG(GPIO_PORTB_BASE +(GPIO_O_DATA + ALL_BITS))|= (BIT0HI|BIT1HI|BIT2HI);
				   	//PostStrategySM(TurnFailEvent);
				break;
				
				
        case SamplingComplete: //If event is event one
					printf("Sampling Complete \r\n");
					turnCount++;
					error = 0;
					error = determineError(currentOrientation);
				
				 // printf("Error = %i, Current Orientation = %i, Target = % i \r\n", error, currentOrientation, targetOrientation);
					if(error>0){
						TurnKart(error,false);
					}
					else if(error<0){
						TurnKart(error,false);
					}
         break;
					
				case PositionReached:
					printf("PositionReached\r\n");
					 StopMotor();
					 CurrentState = SAMPLING;
					 currentOrientation = 0;
					 ES_Timer_InitTimer(SAMPLING_TIMER, SAMPLE_TIME);
				break;
					
      }  
      break;
  }                                   // end switch on Current State
  return ReturnEvent;
}

/****************************************************************************
 Function
     QueryTurningSM

 Parameters
     None

 Returns
     TurningState_t The current state of the Turning state machine

 Description
     returns the current state of the Turning state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
TurningState_t QueryTurningFSM ( void )
{
   return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/

/*
static void initDebuggingLEDs(void){
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != SYSCTL_PRGPIO_R1);
	HWREG(GPIO_PORTB_BASE+ GPIO_O_DEN)|= (BIT0HI|BIT1HI|BIT2HI);
	HWREG(GPIO_PORTB_BASE+ GPIO_O_DIR)|= (BIT0HI|BIT1HI|BIT2HI);
	HWREG(GPIO_PORTB_BASE +(GPIO_O_DATA + ALL_BITS))&= ~(BIT0HI|BIT1HI|BIT2HI);
}

*/
static int determineError(int currentOrientation){
	int error = targetOrientation-currentOrientation;
	if(abs(error)>180){
		if(error < 0)error +=360;
		else error-=360;
	}
	return error;
}
					
					
	/*
	if(targetOrientation == 0){
						if(currentOrientation>180)error = 360-currentOrientation;
						else error = 0 - currentOrientation;
					}
	*/
					
	


