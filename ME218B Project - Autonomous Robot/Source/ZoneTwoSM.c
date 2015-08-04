/****************************************************************************
 Module
   HSMZoneTwo.c

 Revision
   2.0.1

 Description
   This is a ZoneTwo file for implementing state machines.

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
 02/18/99 10:19 jec      built ZoneTwo from MasterMachine.c
 02/14/99 10:34 jec      Began Coding
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
// Basic includes for a program using the Events and Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"

/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ZoneTwoSM.h"
#include "Drive.h"
#include "StrategySM.h"
#include "ComServiceSM.h"
#include "EventCheckers.h"
#include "TurnFSM.h"
/*----------------------------- Module Defines ----------------------------*/
// define constants for the states for this machine
// and any other local defines

#define ENTRY_STATE FOLLOW_TAPE_ZONE_TWO
#define POST_ZONE_THREE_ENTRY_STATE FINDING_TAPE_ZONE_TWO

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event DuringFollowTape( ES_Event Event);
static ES_Event DuringEnterShoot( ES_Event Event);
static ES_Event DuringRealign( ES_Event Event);
static ES_Event DuringFindingTape( ES_Event Event);
static ES_Event DuringForwardToAlign( ES_Event Event);	

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static ZoneTwoState_t CurrentState;
static int lastLapCount = -1;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    RunZoneTwoSM

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
ES_Event RunZoneTwoSM( ES_Event CurrentEvent )
{
   bool MakeTransition = false;/* are we making a state transition? */
   ZoneTwoState_t NextState = CurrentState;
   ES_Event EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
   ES_Event ReturnEvent = CurrentEvent; // assume we are not consuming event

   switch ( CurrentState )
   {
       case FOLLOW_TAPE_ZONE_TWO :
					printf("FOLLOW_TAPE_ZONE_TWO \r\n");
         CurrentEvent = DuringFollowTape(CurrentEvent);
         
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case PositionReached: 
								  printf("Position Reached \r\n");
                  StopMotor();
									NextState = ENTER_SHOOT;
                  MakeTransition = true; 
                  EntryEventKind.EventType = ES_ENTRY_HISTORY;
                  break;
            }
         }
         break;
				 
				 case ENTER_SHOOT:  
					printf("ENTER_SHOOT \r\n");
					CurrentEvent = DuringEnterShoot(CurrentEvent);
					 switch (CurrentEvent.EventType)
            {
               case TurnComplete: 
								  printf("Turn Complete \r\n");
                  StopMotor();
									setZonesForAlign(0, 0,true);
									enableSpeedControl(true,INTO_ZONE_THREE_SPEED,false);
                  break;
            }
         break;
				 case FINDING_TAPE_ZONE_TWO:  
					printf("FINDING_TAPE_ZONE_TWO \r\n");
					CurrentEvent = DuringFindingTape(CurrentEvent);
					 switch (CurrentEvent.EventType)
            {
						 case TapeFound:
									printf("Tape Found\r\n");
									StopMotor();
									NextState = FORWARD_TO_ALIGN_ZONE_TWO;
                  MakeTransition = true;
                  EntryEventKind.EventType = ES_ENTRY_HISTORY;
                  break;
            }
         break;
				case FORWARD_TO_ALIGN_ZONE_TWO:   
					printf("FORWARD_TO_ALIGN_ZONE_TWO \r\n");
					CurrentEvent = DuringForwardToAlign(CurrentEvent);
					 switch (CurrentEvent.EventType)
            {
						 case PositionReached:
									printf("Position Reached \r\n");
									StopMotor();
									NextState = REALIGN_ZONE_TWO;
                  MakeTransition = true;
                  EntryEventKind.EventType = ES_ENTRY_HISTORY;
                  break;
            }
         break;
						 
				 case REALIGN_ZONE_TWO:  
					printf("REALIGN_ZONE_TWO \r\n");
					CurrentEvent = DuringRealign(CurrentEvent);
					 switch (CurrentEvent.EventType)
            {
                case TurnComplete: 
									printf("Turn Complete \r\n");
									StopMotor();
                  NextState = FOLLOW_TAPE_ZONE_TWO;
                  MakeTransition = true;
                  EntryEventKind.EventType = ES_ENTRY_HISTORY;
                  break;
            }
         break;
					
    }
    if (MakeTransition == true)
    {

       CurrentEvent.EventType = ES_EXIT;
       RunZoneTwoSM(CurrentEvent);
       CurrentState = NextState;
       RunZoneTwoSM(EntryEventKind);
     }
     return(ReturnEvent);
}
/****************************************************************************
 Function
     StartZoneTwoSM

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
void StartZoneTwoSM ( ES_Event CurrentEvent )
{
   // to implement entry to a history state or directly to a substate
   // you can modify the initialization of the CurrentState variable
   // otherwise just start in the entry state every time the state machine
   // is started
   if ( ES_ENTRY_HISTORY != CurrentEvent.EventType )
   {
        CurrentState = ENTRY_STATE;
   }
	 if(CurrentEvent.EventParam == ZONE_THREE) CurrentState = POST_ZONE_THREE_ENTRY_STATE;
   // call the entry function (if any) for the ENTRY_STATE
   RunZoneTwoSM(CurrentEvent);
}

/****************************************************************************
 Function
     QueryZoneTwoSM

 Parameters
     None

 Returns
     ZoneTwoState_t The current state of the ZoneTwo state machine

 Description
     returns the current state of the ZoneTwo state machine
 Notes

 Author
     J. Edward Carryer, 2/11/05, 10:38AM
****************************************************************************/
ZoneTwoState_t QueryZoneTwoSM ( void )
{
   return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/
static ES_Event DuringForwardToAlign( ES_Event Event)
{
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
			 DriveDistance(CENTER_OF_ROT_TO_TAPE_SENSORS,false);
		}
    else if ( Event.EventType == ES_EXIT )
    {
			
			
    }else
    {
    }
    return(ReturnEvent);
}




static ES_Event DuringFindingTape( ES_Event Event)
{
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
			setZonesForAlign(0, 0,true);
			setZonesForAlign(2,3,false);
			// setZonesForAlign(2, 1,false);
			 setCheckTapeFound(true);
		}
    else if ( Event.EventType == ES_EXIT )
    {
			setCheckTapeFound(false);
			
    }else
    {
    }
    return(ReturnEvent);
}

static ES_Event DuringRealign( ES_Event Event)
{
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
			setTargetOrientation(90);
			ES_Event Event = {StartTurn,0};
			PostTurningFSM(Event);
			
		}
    else if ( Event.EventType == ES_EXIT )
    {
	
			
    }else
    {
    }
    return(ReturnEvent);
}
		
		


static ES_Event DuringFollowTape( ES_Event Event)
{
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
			FollowTape(true,NOMINAL_FOLLOW_TAPE_DUTY, NOMINAL_FOLLOW_TAPE_DUTY);
			setZonesForAlign(0, 0,true);
			// Guard condition for entering zone three
			//Target Complete?
			//Position of other Karts?
			//Lap Count?
			/*
			if(!TargetCompletionStatus() && lastLapCount != getLapsRemaining() && 
				getKartAZone() != 3 && getKartBZone()!=3)
			*/
			
			if((!TargetCompletionStatus() && lastLapCount != getLapsRemaining()&& 
				getKartAZone() != 3 && getKartBZone()!=3)){
				countEdges(EDGE_DISTANCE_TO_SHOOTING_ZONE_CENTER); //edges
			}
				//if(!TargetCompletionStatus()){
				//countEdges(EDGE_DISTANCE_TO_SHOOTING_ZONE_CENTER); //edges
				//}
			}
    else if ( Event.EventType == ES_EXIT )
    {

    }else
    {
    }
    return(ReturnEvent);
}
		

static ES_Event DuringEnterShoot( ES_Event Event)
{
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
			lastLapCount = getLapsRemaining();
			setZonesForAlign(2, 3,false);
			setTargetOrientation(180);
			ES_Event Event = {StartTurn,0};
			PostTurningFSM(Event);
    }
    else if ( Event.EventType == ES_EXIT )
    {
    }else
    {
			
    }
    return(ReturnEvent);
}
