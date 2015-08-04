/****************************************************************************
 Module
   EventCheckers.c

 Revision
   1.0.1 

 Description
   This is the sample for writing event checkers along with the event 
   checkers used in the basic framework test harness.

 Notes
   Note the use of static variables in sample event checker to detect
   ONLY transitions.
   
 History
 When           Who     What/Why
 -------------- ---     --------
 08/06/13 13:36 jec     initial version
****************************************************************************/

// this will pull in the symbolic definitions for events, which we will want
// to post in response to detecting events
#include "ES_Configure.h"
// this will get us the structure definition for events, which we will need
// in order to post events in response to detecting events
#include "ES_Events.h"
// if you want to use distribution lists then you need those function 
// definitions too.
#include "ES_PostList.h"
// This include will pull in all of the headers from the service modules
// providing the prototypes for all of the post functions
#include "ES_ServiceHeaders.h"
// this test harness for the framework references the serial routines that
// are defined in ES_Port.c
#include "ES_Port.h"

// include our own prototypes to insure consistency between header & 
// actual functionsdefinition
#include "EventCheckers.h"
#include "FollowTape.h"
#include "ADMulti.h"
#include "Drive.h"
#include "StrategySM.h"

#define TAPE_UPPER 600
#define TAPE_LOWER 50
#define FLOOR_UPPER 2600
#define FLOOR_LOWER 1400

static bool checkTapeFound = false;
static int numLaps = 5;
static bool targetStatus = false;
static bool obstacleStatus = false;
static uint32_t results[NUM_TAPE_SENSORS]; // Array to hold the A/D converter results from the tape sensors

// This is the event checking function sample. It is not intended to be 
// included in the module. It is only here as a sample to guide you in writing
// your own event checkers
#if 0
/****************************************************************************
 Function
   Check4Lock
 Parameters
   None
 Returns
   bool: true if a new event was detected
 Description
   Sample event checker grabbed from the simple lock state machine example
 Notes
   will not compile, sample only
 Author
   J. Edward Carryer, 08/06/13, 13:48
****************************************************************************/
bool Check4Lock(void)
{
  static uint8_t LastPinState = 0;
  uint8_t CurrentPinState;
  bool ReturnVal = false;
  
  CurrentPinState =  LOCK_PIN;
  // check for pin high AND different from last time
  // do the check for difference first so that you don't bother with a test
  // of a port/variable that is not going to matter, since it hasn't changed
  if ( (CurrentPinState != LastPinState) &&
       (CurrentPinState == LOCK_PIN_HI) )
  {                     // event detected, so post detected event
    ES_Event ThisEvent;
    ThisEvent.EventType = ES_LOCK;
    ThisEvent.EventParam = 1;
    // this could be any of the service post function, ES_PostListx or 
    // ES_PostAll functions
    ES_PostList01(ThisEvent); 
    ReturnVal = true;
  }
  LastPinState = CurrentPinState; // update the state for next time

  return ReturnVal;
}
#endif

/****************************************************************************
 Function
   Check4Keystroke
 Parameters
   None
 Returns
   bool: true if a new key was detected & posted
 Description
   checks to see if a new key from the keyboard is detected and, if so, 
   retrieves the key and posts an ES_NewKey event to TestHarnessService0
 Notes
   The functions that actually check the serial hardware for characters
   and retrieve them are assumed to be in ES_Port.c
   Since we always retrieve the keystroke when we detect it, thus clearing the
   hardware flag that indicates that a new key is ready this event checker 
   will only generate events on the arrival of new characters, even though we
   do not internally keep track of the last keystroke that we retrieved.
 Author
   J. Edward Carryer, 08/06/13, 13:48
****************************************************************************/
bool Check4Keystroke(void) {
  if (IsNewKeyReady()) { // new key waiting?
		ES_Event ThisEvent;
		ThisEvent.EventType = ES_NEW_KEY;
		ThisEvent.EventParam = GetNewKey();
			
		if (ThisEvent.EventParam == '1') {
			printf("KEY PRESS: ZoneUpdate 1\r\n");
			ES_Event Event = {ZoneUpdate, 1};
			PostStrategySM(Event);
			return true;
		}
		if (ThisEvent.EventParam == '2') {
			printf("KEY PRESS: ZoneUpdate 2\r\n");
			ES_Event Event = {ZoneUpdate, 2};
			PostStrategySM(Event);
			return true;
		}
		if (ThisEvent.EventParam == '3') {
			printf("KEY PRESS: ZoneUpdate 3\r\n");
			ES_Event Event = {ZoneUpdate, 3};
			PostStrategySM(Event);
			return true;
		}
		if (ThisEvent.EventParam == '4') {
			printf("KEY PRESS: ZoneUpdate 4\r\n");
			ES_Event Event = {ZoneUpdate, 4};
			PostStrategySM(Event);
			return true;
		}
		if (ThisEvent.EventParam == '5') {
			printf("KEY PRESS: ZoneUpdate 1\r\n");
			ES_Event Event = {ZoneUpdate, 5};
			PostStrategySM(Event);
			return true;
		}
		if (ThisEvent.EventParam == 'f') {
			printf("KEY PRESS: FlagDropped\r\n");
			ES_Event Event = {FlagDropped,0};
			PostStrategySM(Event);
			return true;
		}
		if (ThisEvent.EventParam == 'c') {
			printf("KEY PRESS: CautionFlag\r\n");
			ES_Event Event = {CautionFlag,0};
			PostStrategySM(Event);
			return true;
		}	
		if (ThisEvent.EventParam == 'k') {
			printf("KEY PRESS: RaceOver\r\n");
			ES_Event Event = {RaceOver,0};
			PostStrategySM(Event);
			return true;
		}	
		if (ThisEvent.EventParam == 'p') {
			printf("KEY PRESS: PositionReached\r\n");
			ES_Event Event = {PositionReached,0};
			PostStrategySM(Event);
			return true;
		}
		if (ThisEvent.EventParam == 't') {
			printf("KEY PRESS: TapeFound\r\n");
			ES_Event Event = {TapeFound,0};
			PostStrategySM(Event);
			return true;
		}
		if (ThisEvent.EventParam == 'l') {
			printf("KEY PRESS: LapUpdate\r\n");
			ES_Event Event = {LapUpdate,numLaps};
			numLaps--;
			PostStrategySM(Event);
			return true;
		}
		if (ThisEvent.EventParam == 's') {
			printf("KEY PRESS: TargetComplete\r\n");
			if(!targetStatus){
				ES_Event Event = {TargetComplete,0};
				PostStrategySM(Event);
				targetStatus = true;
			}
			return true;
		}
		if (ThisEvent.EventParam == 'm') {
			printf("KEY PRESS: ObstacleMostlyCrossed\r\n");
			ES_Event Event = {ObstacleMostlyCrossed,0};
			PostStrategySM(Event);
			return true;
		}
		if (ThisEvent.EventParam == 'b') {
			printf("KEY PRESS: BeaconDetected\r\n");
			ES_Event Event = {BeaconDetected,0};
			PostStrategySM(Event);
			return true;
		}
		if (ThisEvent.EventParam == 'o') {
			printf("KEY PRESS: ObstacleComplete\r\n");
			if(!obstacleStatus){
				ES_Event Event = {ObstacleComplete,0};
				PostStrategySM(Event);
				obstacleStatus = true;
			}
			return true;
		}
		if (ThisEvent.EventParam == 'j') {
			printf("KEY PRESS: Turn Complete\r\n");
				ES_Event Event = {TurnComplete,0};
				PostStrategySM(Event);
				obstacleStatus = true;
				return true;
			}
			
		
		
			if (ThisEvent.EventParam == 'x') {
			printf("KEY PRESS: Center of Obs\r\n");
		//if(!obstacleStatus){
			ES_Event Event;
			Event.EventType = CenterOfObstacle;
			PostStrategySM(Event);
		//}
			return true;
		}
	}
  return false;
}


//Todo
//Empirally determine bounds for tape/floor

bool CheckTapeStatus(void){
	/* Read the A/D converter results and store in variables */

	
	if(checkTapeFound) {
		ADC_MultiRead(results);
		int tapeSensor1 = (int)results[0];
		int tapeSensor2 = (int)results[1];
		
		//printf("L: %i R: %i\r\n", tapeSensor1, tapeSensor2);
	
		if ((tapeSensor1 < TAPE_UPPER && tapeSensor1 > TAPE_LOWER) || 
				(tapeSensor2 < TAPE_UPPER && tapeSensor2 > TAPE_LOWER)) {
				ES_Event Event;
				Event.EventType = TapeFound;
				PostStrategySM(Event);
			
				setCheckTapeFound(false);
				return true;
			
		}
		
		
		/*
		if(tapeSensor1 < TAPE_UPPER &&  tapeSensor1 > TAPE_LOWER &&
			tapeSensor2 < TAPE_UPPER &&  tapeSensor2 > TAPE_LOWER){
			 	if(  lastTapeSensor1Val < FLOOR_UPPER &&  lastTapeSensor1Val > FLOOR_LOWER &&
						lastTapeSensor2Val < FLOOR_UPPER &&  lastTapeSensor2Val> FLOOR_LOWER){
							ES_Event Event;
							Event.EventType = TapeFound;
							PostStrategySM(Event);
							lastTapeSensor1Val = tapeSensor1;
							lastTapeSensor2Val = tapeSensor2;
							return true;
						}
				
		}
		lastTapeSensor1Val = tapeSensor1;
		lastTapeSensor2Val = tapeSensor2;
		*/
	}
	return false;
}

void setCheckTapeFound(bool eventCheckerStatus){
	checkTapeFound = eventCheckerStatus;
}

