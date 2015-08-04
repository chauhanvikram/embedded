/****************************************************************************
 Module
   HSMZoneFive.c

 Revision
   2.0.1

 Description
   This is a ZoneFive file for implementing state machines.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/07/13 21:00 jec      corrections to return variable (should have been
                         ReturnEvent, not CurrentEvent) and several EV_xxx
                         event names that were left over from the old version
 02/08/12 09:56 jec      revisions for the Events and Services Framework Gen2
 02/13/10 14:29 jec      revised Start and run to add new kind of entry function
                         to make implementing history entry cleaner
 02/13/10 12:29 jec      added NewEvent local variable to During function and
                         comments about using either it or Event as the return
 02/11/10 15:54 jec      more revised comments, removing last comment in during
                         function that belongs in the run function
 02/09/10 17:21 jec      updated comments about internal transitions on During funtion
 02/18/09 10:14 jec      removed redundant call to RunLowerlevelSM in EV_Entry
                         processing in During function
 02/20/07 21:37 jec      converted to use enumerated type for events & states
 02/13/05 19:38 jec      added support for self-transitions, reworked
                         to eliminate repeated transition code
 02/11/05 16:54 jec      converted to implment hierarchy explicitly
 02/25/03 10:32 jec      converted to take a passed event parameter
 02/18/99 10:19 jec      built ZoneFive from MasterMachine.c
 02/14/99 10:34 jec      Began Coding
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
// Basic includes for a program using the Events and Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "Drive.h"

/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ZoneFiveSM.h"
#include "ComServiceSM.h"

/*----------------------------- Module Defines ----------------------------*/
// define constants for the states for this machine
// and any other local defines

#define ENTRY_STATE CLIMBING_OBSTACLE

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event DuringClimbing(ES_Event Event);
static ES_Event DuringDescending(ES_Event Event);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static ZoneFiveState_t CurrentState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    RunZoneFiveSM

 Parameters
   ES_Event: the event to process

 Returns
   ES_Event: an event to return

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   J. Edward Carryer, 2/11/05, 10:45AM
****************************************************************************/
ES_Event RunZoneFiveSM(ES_Event CurrentEvent) {
	bool MakeTransition = false;/* are we making a state transition? */
	ZoneFiveState_t NextState = CurrentState;
	ES_Event EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
	ES_Event ReturnEvent = CurrentEvent; // assume we are not consuming event

	switch (CurrentState) {
		case CLIMBING_OBSTACLE: {
			printf("CLIMBING_OBSTACLE \r\n");
			CurrentEvent = DuringClimbing(CurrentEvent);
			if (CurrentEvent.EventType != ES_NO_EVENT) { // if an event is active
				switch (CurrentEvent.EventType) {
					case ObstacleMostlyCrossed: {
						printf("Obstacle Mostly Crossed\r\n");
						NextState = DESCENDING_OBSTACLE;
						MakeTransition = true;
						EntryEventKind.EventType = ES_ENTRY_HISTORY;
					}
					break;
				}
			}
		}
		break;
		
		case DESCENDING_OBSTACLE: {
			printf("DESCENDING_OBSTACLE \r\n");
			CurrentEvent = DuringDescending(CurrentEvent);
		}
		break; 
	}
	
    //   If we are making a state transition
    if (MakeTransition == true) {
		// Execute exit function for current state
		CurrentEvent.EventType = ES_EXIT;
		RunZoneFiveSM(CurrentEvent);
		CurrentState = NextState; //Modify state variable

		// Execute entry function for new state
		// this defaults to ES_ENTRY
		RunZoneFiveSM(EntryEventKind);
	}
	
	return ReturnEvent;
}
/****************************************************************************
 Function
     StartZoneFiveSM

 Parameters
     None

 Returns
     None

 Description
     Does any required initialization for this state machine
 Notes

 Author
     J. Edward Carryer, 2/18/99, 10:38AM
****************************************************************************/
void StartZoneFiveSM ( ES_Event CurrentEvent )
{
   // to implement entry to a history state or directly to a substate
   // you can modify the initialization of the CurrentState variable
   // otherwise just start in the entry state every time the state machine
   // is started
   if ( ES_ENTRY_HISTORY != CurrentEvent.EventType )
   {
        CurrentState = ENTRY_STATE;
   }
   // call the entry function (if any) for the ENTRY_STATE
   RunZoneFiveSM(CurrentEvent);
}

/****************************************************************************
 Function
     QueryZoneFiveSM

 Parameters
     None

 Returns
     ZoneFiveState_t The current state of the ZoneFive state machine

 Description
     returns the current state of the ZoneFive state machine
 Notes

 Author
     J. Edward Carryer, 2/11/05, 10:38AM
****************************************************************************/
ZoneFiveState_t QueryZoneFiveSM ( void )
{
   return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/

static ES_Event DuringClimbing( ES_Event Event) {
    ES_Event ReturnEvent = Event;

    if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
			setZonesForAlign(0, 0,true);
			//FollowTape(true,  FOLLOW_TAPE_CLIMBING_DUTY, FOLLOW_TAPE_CLIMBING_DUTY);
			//enableSpeedControl(true, CLIMBING_SPEED, true);
			setListenToCrossOverPoint(true);
			setZonesForAlign(5, 1,false);
			//DriveReverse(99);
    } else if (Event.EventType == ES_EXIT) {
			//FollowTape(false,  0, 0);
    } else {
			
    }
	
    return(ReturnEvent);
}

static ES_Event DuringDescending( ES_Event Event) {
    ES_Event ReturnEvent = Event;

    if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
				setComeListenToPostFive(true);
				//enableSpeedControl(true, REVERSE_BREAKING_SPEED, true);
			//enableSpeedControl(true, 0, false);
			  //riveForward(99);
			 //StopMotor();
			 enableSpeedControl(true,10,true);
       setZonesForAlign(0, 0,true);			
			
    } else if (Event.EventType == ES_EXIT) {
				
    } else {

    }
	
    return(ReturnEvent);
}



