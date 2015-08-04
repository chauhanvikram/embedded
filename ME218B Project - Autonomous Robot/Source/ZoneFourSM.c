/****************************************************************************
 Module
   HSMZoneFour.c

 Revision
   2.0.1

 Description
   This is a ZoneFour file for implementing state machines.

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
 02/18/99 10:19 jec      built ZoneFour from MasterMachine.c
 02/14/99 10:34 jec      Began Coding
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
// Basic includes for a program using the Events and Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"

/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ZoneFourSM.h"
#include "Drive.h"
#include "StrategySM.h"
#include "ComServiceSM.h"
#include "TurnFSM.h"
#include "PWMDemo.h"

/*----------------------------- Module Defines ----------------------------*/
// define constants for the states for this machine
// and any other local defines

#define ENTRY_STATE FOLLOW_TAPE_ZONE_FOUR
#define POST_BACK_UP_TIME 2500

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event DuringFollowTape( ES_Event Event);
static ES_Event DuringAlignWithObstacle( ES_Event Event);
static ES_Event DuringBackAwayFromObstacle( ES_Event Event);
static ES_Event DuringAlignWithObstacleRoundTwo( ES_Event Event);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static ZoneFourState_t CurrentState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    RunZoneFourSM

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
ES_Event RunZoneFourSM( ES_Event CurrentEvent )
{
   bool MakeTransition = false;/* are we making a state transition? */
   ZoneFourState_t NextState = CurrentState;
   ES_Event EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
   ES_Event ReturnEvent = CurrentEvent; // assume we are not consuming event

   switch ( CurrentState )
   {
       case FOLLOW_TAPE_ZONE_FOUR :  
				  printf("FOLLOW_TAPE_ZONE_FOUR \r\n");
         CurrentEvent = DuringFollowTape(CurrentEvent);
         if ( CurrentEvent.EventType != ES_NO_EVENT ) 
         {
            switch (CurrentEvent.EventType)
            {
               case CenterOfObstacle: 
								  printf("Center Of Obstacle \r\n");
									StopMotor();
                  NextState = ALIGN_WITH_OBSTACLE;
                  MakeTransition = true;
                  EntryEventKind.EventType = ES_ENTRY_HISTORY;
                  ReturnEvent.EventType = ES_NO_EVENT;
                  break;
            }
         }
         break;
				 //really driving forward
				 case BACK_AWAY_FROM_OBSTACLE :
					printf("BACK_AWAY_FROM_OBSTACLE \r\n");
					CurrentEvent = DuringBackAwayFromObstacle(CurrentEvent);
					if ( CurrentEvent.EventType != ES_NO_EVENT ) 
					{
            switch (CurrentEvent.EventType)
            {
               case PositionReached : 
								  printf("Position Reached \r\n");
									StopMotor();
									ES_Timer_InitTimer(BACK_UP_TIMER,POST_BACK_UP_TIME);
							 break;
							 
							 case ES_TIMEOUT :
									if(CurrentEvent.EventParam == BACK_UP_TIMER){
										NextState = ALIGN_WITH_OBSTACLE_ROUND_TWO;
										MakeTransition = true;
										EntryEventKind.EventType = ES_ENTRY_HISTORY;
										ReturnEvent.EventType = ES_NO_EVENT;
									}
                  break;
            }
         }
         break;
				 
				  
				 case ALIGN_WITH_OBSTACLE_ROUND_TWO :  
			    printf("ALIGN_WITH_OBSTACLE_ROUND_TWO \r\n");
          CurrentEvent = DuringAlignWithObstacleRoundTwo(CurrentEvent);
         if ( CurrentEvent.EventType != ES_NO_EVENT ) 
         {
            switch (CurrentEvent.EventType)
            {
               case TurnComplete : 
								  printf("Turn Complete \r\n");
									StopMotor();
									setZonesForAlign(0, 0,true);
									//enableSpeedControl(true, INTO_RAMP_SPEED, false); //was 100
									//enableSpeedControl(true, INTO_RAMP_SPEED, true); //was 100
									SetPWMDutyDrive(-99, -99);
                  //enableSpeedControl(true, INTO_RAMP_SPEED,false);
                  break;
            }
         }
         break;
				 
				 case ALIGN_WITH_OBSTACLE :  
			    printf("ALIGN_WITH_OBSTACLE \r\n");
         CurrentEvent = DuringAlignWithObstacle(CurrentEvent);
         if ( CurrentEvent.EventType != ES_NO_EVENT ) 
         {
            switch (CurrentEvent.EventType)
            {
               case TurnComplete: 
							//case PositionReached:
								  printf("Turn Complete \r\n");
									StopMotor();
                  NextState = BACK_AWAY_FROM_OBSTACLE;
									//enableSpeedControl(true, 100, false);
                  MakeTransition = true;
                  EntryEventKind.EventType = ES_ENTRY_HISTORY;
                  ReturnEvent.EventType = ES_NO_EVENT;
                  break;
            }
         }
         break;
    }
    if (MakeTransition == true)
    {
       CurrentEvent.EventType = ES_EXIT;
       RunZoneFourSM(CurrentEvent);
       CurrentState = NextState; 
       RunZoneFourSM(EntryEventKind);
     }
     return(ReturnEvent);
}
/****************************************************************************
 Function
     StartZoneFourSM

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
void StartZoneFourSM ( ES_Event CurrentEvent )
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
   RunZoneFourSM(CurrentEvent);
}

/****************************************************************************
 Function
     QueryZoneFourSM

 Parameters
     None

 Returns
     ZoneFourState_t The current state of the ZoneFour state machine

 Description
     returns the current state of the ZoneFour state machine
 Notes

 Author
     J. Edward Carryer, 2/11/05, 10:38AM
****************************************************************************/
ZoneFourState_t QueryZoneFourSM ( void )
{
   return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/
static ES_Event DuringBackAwayFromObstacle( ES_Event Event){
	 ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {   
			
			countEdges(BACK_UP_FROM_OBSTACLE_DISTANCE*EDGES_PER_INCH);
			//enableSpeedControl(true,BACK_UP_FROM_OBSTACLE_SPEED,true);
			enableSpeedControl(true,10,false);
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
			
			setZonesForAlign(0, 0,true);
			FollowTape(true,NOMINAL_FOLLOW_TAPE_DUTY, NOMINAL_FOLLOW_TAPE_DUTY);
			setZonesForAlign(4, 1,false);
			//if(!ObstacleCompletionStatus() &&  getKartAZone() != 5 && getKartBZone()!=5){
				//countEdges(EDGE_DISTANCE_TO_OBSTACLE_CENTER);
		//if(!ObstacleCompletionStatus()&& getLapsRemaining()==1)setMiddleOfObstacle(true);
			if((!ObstacleCompletionStatus()&&  getKartAZone() != 5 && getKartBZone()!=5 && 
				getLapsRemaining()==1))setMiddleOfObstacle(true);
			else setMiddleOfObstacle(false);
			
    }
    else if ( Event.EventType == ES_EXIT )
    {
    }else
    {
   
    }
    return(ReturnEvent);
}

static ES_Event DuringAlignWithObstacle( ES_Event Event)
{
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {  
			
		  setTargetOrientation(90);
			//setTargetOrientation(270);
			ES_Event Event = {StartTurn,0};
			PostTurningFSM(Event);
			
			//TurnKart(-90,false);
    }
			
    else if ( Event.EventType == ES_EXIT )
    {
    }else
    {
   
    }
    return(ReturnEvent);
}

static ES_Event DuringAlignWithObstacleRoundTwo( ES_Event Event)
{
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {  
			
			//setTargetOrientation(270);
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

