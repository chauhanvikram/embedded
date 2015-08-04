/****************************************************************************
 Module
   HSMZoneOne.c

 Revision
   2.0.1

 Description
   This is a ZoneOne file for implementing state machines.

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
 02/18/99 10:19 jec      built ZoneOne from MasterMachine.c
 02/14/99 10:34 jec      Began Coding
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
// Basic includes for a program using the Events and Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"

/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ZoneOneSM.h"
#include "Drive.h"
#include "StrategySM.h"
#include "EventCheckers.h"
#include "TurnFSM.h"
#include "ComServiceSM.h"

/*----------------------------- Module Defines ----------------------------*/
// define constants for the states for this machine
// and any other local defines

#define ENTRY_STATE FOLLOW_TAPE_ZONE_ONE
#define POST_ZONE_FIVE_ENTRY_STATE FROM_ZONE_FIVE


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event DuringFindingTape( ES_Event Event);
static ES_Event DuringRealign( ES_Event Event);
static ES_Event DuringFollowTape( ES_Event Event);
static ES_Event DuringForwardToAlign( ES_Event Event);
static ES_Event DuringFromZoneFive( ES_Event Event);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static ZoneOneState_t CurrentState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    RunZoneOneSM

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
ES_Event RunZoneOneSM( ES_Event CurrentEvent )
{
   bool MakeTransition = false;/* are we making a state transition? */
   ZoneOneState_t NextState = CurrentState;
   ES_Event EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
   ES_Event ReturnEvent = CurrentEvent; // assume we are not consuming event

	 
   switch ( CurrentState )
   {
		 
			case FROM_ZONE_FIVE: 
				 printf("DRIVE_FORWARD_ZONE_ONE\r\n");
         CurrentEvent = DuringFromZoneFive(CurrentEvent);
         if ( CurrentEvent.EventType != ES_NO_EVENT ) 
         {
            switch (CurrentEvent.EventType)
            {
               case ZoneOneStop:{ 
								 printf("ZoneOneStop \r\n");
                 StopMotor();
								 setTargetOrientation(75);
								 ES_Event Event = {StartTurn,0};
								 PostTurningFSM(Event);
							 } break;
							 
							 case TurnComplete:
								printf("TurnComplete\r\n");
								StopMotor();
								NextState = FINDING_TAPE_ZONE_ONE;
                MakeTransition = true;
                EntryEventKind.EventType = ES_ENTRY_HISTORY;
							  break;
							 
            }
         }
         break;
			 
		 
		 
       case FOLLOW_TAPE_ZONE_ONE: 
				 printf("FOLLOW_TAPE_ZONE_ONE\r\n");
         CurrentEvent = DuringFollowTape(CurrentEvent);
         break;
				
			 
			 case FORWARD_TO_ALIGN_ZONE_ONE: 
					printf("FORWARD_TO_ALIGN_ZONE_ONE\r\n");
         CurrentEvent = DuringForwardToAlign(CurrentEvent);
         if ( CurrentEvent.EventType != ES_NO_EVENT ) 
         {
            switch (CurrentEvent.EventType)
            {
               case PositionReached: 
								 printf("Position Reached \r\n");
                  StopMotor();
									NextState = REALIGN_ZONE_ONE;
                  MakeTransition = true;
                  EntryEventKind.EventType = ES_ENTRY_HISTORY;
                  break;
            }
         }
         break;
			 
			 case REALIGN_ZONE_ONE:  
					printf("REALIGN_ZONE_ONE\r\n");
         CurrentEvent = DuringRealign(CurrentEvent);
         if ( CurrentEvent.EventType != ES_NO_EVENT ) 
         {
            switch (CurrentEvent.EventType)
            {
               //case TurnComplete: 
							case PositionReached:
									printf("Turn Complete \r\n");
                  StopMotor();
									NextState = FOLLOW_TAPE_ZONE_ONE;
                  MakeTransition = true;
                  EntryEventKind.EventType = ES_ENTRY_HISTORY;
                  break;
            }
         }
         break;
				 
				 case FINDING_TAPE_ZONE_ONE:   
				printf("FINDING_TAPE_ZONE_ONE\r\n");
         CurrentEvent = DuringFindingTape(CurrentEvent);
         if ( CurrentEvent.EventType != ES_NO_EVENT ) 
         {
            switch (CurrentEvent.EventType)
            {
               case TapeFound: 
								 	printf("Tape Found \r\n");
                  StopMotor();
									NextState = FORWARD_TO_ALIGN_ZONE_ONE;
                  MakeTransition = true;
                  EntryEventKind.EventType = ES_ENTRY_HISTORY;
                  break;
            }
         }
         break;
		}
 
    if (MakeTransition == true)
    {
       //   Execute exit function for current state
       CurrentEvent.EventType = ES_EXIT;
       RunZoneOneSM(CurrentEvent);

       CurrentState = NextState; //Modify state variable

       //   Execute entry function for new state
       // this defaults to ES_ENTRY
       RunZoneOneSM(EntryEventKind);
     }
     return(ReturnEvent);
}
/****************************************************************************
 Function
     StartZoneOneSM

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
void StartZoneOneSM ( ES_Event CurrentEvent )
{
   // to implement entry to a history state or directly to a substate
   // you can modify the initialization of the CurrentState variable
   // otherwise just start in the entry state every time the state machinek
   // is started
   if ( ES_ENTRY_HISTORY != CurrentEvent.EventType )
   {
        CurrentState = ENTRY_STATE; 
	 }

	 if(CurrentEvent.EventParam == ZONE_FIVE) CurrentState = POST_ZONE_FIVE_ENTRY_STATE;
   // call the entry function (if any) for the ENTRY_STATE
   RunZoneOneSM(CurrentEvent);
}

/****************************************************************************
 Function
     QueryZoneOneSM

 Parameters
     None

 Returns
     ZoneOneState_t The current state of the ZoneOne state machine

 Description
     returns the current state of the ZoneOne state machine
 Notes

 Author
     J. Edward Carryer, 2/11/05, 10:38AM
****************************************************************************/
ZoneOneState_t QueryZoneOneSM ( void )
{
   return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/


static ES_Event DuringFromZoneFive( ES_Event Event)
{
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
			 setZonesForAlign(0, 0,true);
			 StopMotor();
			 setZonesForAlign(1, 5,false);
			 enableSpeedControl(true, TAPE_FINDING_SPEED, true);
		}
    else if ( Event.EventType == ES_EXIT )
    {
			
			
    }else
    {
    }
    return(ReturnEvent);
}



static ES_Event DuringForwardToAlign( ES_Event Event)
{
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
			 DriveDistance(0.75,false);
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
			StopMotor();
			
			enableSpeedControl(true,TAPE_FINDING_SPEED,false);
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
			//setTargetOrientation(0);
			//ES_Event Event = {StartTurn,0};
			//PostTurningFSM(Event);
			TurnKart(-90,false);
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
			setZonesForAlign(0, 0,true); // reset zones
    }
    else if ( Event.EventType == ES_EXIT )
    {
      setZonesForAlign(0, 0,true);
    }else
   
    {
     
    }
   
    return(ReturnEvent);
}
