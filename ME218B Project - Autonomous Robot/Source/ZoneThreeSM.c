/****************************************************************************
 Module
   HSMZoneThree.c

 Revision
   2.0.1

 Description
   This is a ZoneThree file for implementing state machines.

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
 02/18/99 10:19 jec      built ZoneThree from MasterMachine.c
 02/14/99 10:34 jec      Began Coding
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
// Basic includes for a program using the Events and Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "StrategySM.h"
#include "Drive.h"
#include "EncoderRight.h"
#include "EncoderLeft.h"
#include "Shooter.h"
#include "TurnFSM.h"
#include "ComServiceSM.h"

/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ZoneThreeSM.h"

/*----------------------------- Module Defines ----------------------------*/
// define constants for the states for this machine
// and any other local defines

#define ENTRY_STATE DRIVE_TO_SHOOTING_DISTANCE

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/
static ES_Event DuringTurnTowardsZoneTwo( ES_Event Event);
static ES_Event DuringAlignWithBeacon( ES_Event Event);
static ES_Event DuringDriveToShootingDistance( ES_Event Event);
static ES_Event DuringShooting( ES_Event Event);
static ES_Event DuringTurnTowardsZoneTwo( ES_Event Event);
static ES_Event DuringWaitForShootMotor( ES_Event Event);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well
static ZoneThreeState_t CurrentState;
static int currentLapsRemaining;
static int lastLapsRemaining = -1;
//static uint32_t edgesForBeaconTurn;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    RunZoneThreeSM

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
ES_Event RunZoneThreeSM( ES_Event CurrentEvent )
{
   bool MakeTransition = false;/* are we making a state transition? */
   ZoneThreeState_t NextState = CurrentState;
   ES_Event EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
   ES_Event ReturnEvent = CurrentEvent; // assume we are not consuming event

   switch ( CurrentState )
   {
				case DRIVE_TO_SHOOTING_DISTANCE :
				printf("DRIVE_TO_SHOOTING_DISTANCE \r\n");
         CurrentEvent = DuringDriveToShootingDistance(CurrentEvent);
         if ( CurrentEvent.EventType != ES_NO_EVENT )
         {
            switch (CurrentEvent.EventType)
            {
               case PositionReached:
								  printf("Position Reached \r\n");
									StopMotor();
									currentLapsRemaining = getLapsRemaining();
								  if(currentLapsRemaining!=lastLapsRemaining){
										NextState = ALIGN_WITH_BEACON;
										lastLapsRemaining = currentLapsRemaining;
									}else NextState = TURN_TOWARDS_ZONE_TWO;
                  MakeTransition = true; 
                  break;
            }
         }
         break;
				case ALIGN_WITH_BEACON :
					printf("ALIGN_WITH_BEACON \r\n");
         CurrentEvent = DuringAlignWithBeacon(CurrentEvent);
        
         if ( CurrentEvent.EventType != ES_NO_EVENT )
         {
            switch (CurrentEvent.EventType)
            {
						
							case TurnComplete :
								 printf("Turning Complete \r\n");
								 StopMotor();
							   AlignWithBeacon();
							break;
               case BeaconDetected :
								  printf("Beacon Detected\r\n");
                  StopMotor();
									NextState = WAIT_FOR_SHOOT_MOTOR;
                  MakeTransition = true; 
              break;
            }
         }
         break;
				 
				 case WAIT_FOR_SHOOT_MOTOR:
					 printf("WAIT_FOR_SHOOT_MOTOR \r\n");
					CurrentEvent = DuringWaitForShootMotor(CurrentEvent);
        
         if ( CurrentEvent.EventType != ES_NO_EVENT )
         {
            switch (CurrentEvent.EventType)
            {
               case ES_TIMEOUT :
									printf("ES_Timeout \r\n");
									if(CurrentEvent.EventParam == FLY_WHEEL_TIMER){
											ES_Timer_InitTimer(BACK_GATE_TIMER, BACK_GATE_WAIT_TIME);
											ActivateBackGate(-1);
									}
									else if(CurrentEvent.EventParam == BACK_GATE_TIMER){
											NextState = SHOOTING;
											MakeTransition = true; 
									}
                  break;
            }
         }
         break;
				 
				  
				 case TURN_TOWARDS_ZONE_TWO:
					 printf("TURN_TOWARDS_ZONE_TWO \r\n");
         CurrentEvent = DuringTurnTowardsZoneTwo(CurrentEvent);
        
         if ( CurrentEvent.EventType != ES_NO_EVENT )
         {
            switch (CurrentEvent.EventType)
            {
              // case TurnComplete :
							case PositionReached:
								  printf("Turning Complete \r\n");
									StopMotor();
									setZonesForAlign(0, 0,true);
									enableSpeedControl(true,OUT_OF_ZONE_THREE_SPEED, false);
									//setZonesForAlign(2,3,false);
                  break;
            }
         }
         break;
				 
				 case SHOOTING :
					 printf("SHOOTING \r\n");
         CurrentEvent = DuringShooting(CurrentEvent);
        
         if ( CurrentEvent.EventType != ES_NO_EVENT )
         {
            switch (CurrentEvent.EventType)
            {
               case ES_TIMEOUT :
								 	printf("ES_Timeout \r\n");
								 if(CurrentEvent.EventParam == SHOOT_TIMER){
										NextState = TURN_TOWARDS_ZONE_TWO;
										MakeTransition = true; 
								 }
									break;
            }
         }
         break;
				 
    }
    if (MakeTransition == true)
    {
       //   Execute exit function for current state
       CurrentEvent.EventType = ES_EXIT;
       RunZoneThreeSM(CurrentEvent);

       CurrentState = NextState; //Modify state variable

       //   Execute entry function for new state
       // this defaults to ES_ENTRY
       RunZoneThreeSM(EntryEventKind);
     }
     return(ReturnEvent);
}
/****************************************************************************
 Function
     StartZoneThreeSM

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
void StartZoneThreeSM ( ES_Event CurrentEvent )
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
  
	 RunZoneThreeSM(CurrentEvent);
	 
}

/****************************************************************************
 Function
     QueryZoneThreeSM

 Parameters
     None

 Returns
     ZoneThreeState_t The current state of the ZoneThree state machine

 Description
     returns the current state of the ZoneThree state machine
 Notes

 Author
     J. Edward Carryer, 2/11/05, 10:38AM
****************************************************************************/
ZoneThreeState_t QueryZoneThreeSM ( void )
{
   return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/
 static ES_Event DuringWaitForShootMotor( ES_Event Event)
{
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
      StartFlyWheel();
			ActivateBackGate(1);
			ES_Timer_InitTimer(FLY_WHEEL_TIMER, MOTOR_WAIT_TIME);
    }
    else if ( Event.EventType == ES_EXIT )
    {
      
    }else
    {
    }
   
    return(ReturnEvent);
}

static ES_Event DuringTurnTowardsZoneTwo( ES_Event Event)
{
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
			
       //TurnKart(-((float)edgesForBeaconTurn/(float)EDGES_PER_DEGREE), false);
			//setTargetOrientation(30);
			//ES_Event Event = {StartTurn,0};
			//PostTurningFSM(Event);
			TurnKart(180,false);
    }
    else if ( Event.EventType == ES_EXIT )
    {
      
    }else
   
    {
        
    }
   
    return(ReturnEvent);
}

static ES_Event DuringShooting( ES_Event Event)
{
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
      ActivateFrontGate(1);   
			ES_Timer_InitTimer(SHOOT_TIMER, SHOOT_TIME);
				
    }
    else if ( Event.EventType == ES_EXIT )
    {
      ActivateFrontGate(-1); 
			StopFlyWheel();
    }else
   
    {
        
    }
   
    return(ReturnEvent);
}

static ES_Event DuringDriveToShootingDistance( ES_Event Event)
{
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
			setZonesForAlign(0, 0,true);
      StopMotor();
			setZonesForAlign(3, 2,false);
			enableSpeedControl(true, INTO_ZONE_THREE_SPEED,false);
			countEdges(SHOOTING_DISTANCE_FROM_BOUNDARY_OF_ZONE_THREE*EDGES_PER_INCH);
			//DriveDistance(SHOOTING_DISTANCE_FROM_BOUNDARY_OF_ZONE_THREE,false); //inches
    }
    else if ( Event.EventType == ES_EXIT )
    {
       
    }else
    {   
    }
    return(ReturnEvent);
}

static ES_Event DuringAlignWithBeacon( ES_Event Event)
{
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
			setTargetOrientation(270);
			ES_Event Event = {StartTurn,0};
			PostTurningFSM(Event);
			// AlignWithBeacon();
    }
    else if ( Event.EventType == ES_EXIT )
    {
       
    }else
    {
       
    }   
    return(ReturnEvent);
}

