/****************************************************************************
 Module
   StrategySM.c

 Revision
   2.0.1

 Description
   This is a template for the top level Hierarchical state machine

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 3/1/15	115								1. Still need to add guard conditions for out of zone four in both 
														zone four sm and in com
													2. Fix caution flag
													3.guard conditions into zone 3
 
 2/28/16 5:40 AM					1. Changed out of zone three target to 10 degrees
							
 
 2/28/16									1. Increase align with beacon speed to 21
													2. Increased zone y min from 48 to 55
													3. disabled follow tape in zone five
													4. Changed shooting from 11 to 10
													5. Slowed down initial into ramp speed
 4:15AM
 
 2/28/15					AB			1. Adjusted beacon turn speed to 18
 4:01 AM									2. changed into ramp speed control to 70rpm
													3. adjusted center of obstacle to 180 x
													4. Adjusted realign with zone one follow tape angle to 354 deg
 
 2/25/15					AB			To do: 1) Modify zone four for a "runway"
														`			2) Tape_finding problems
																	3) Distance constants

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "StrategySM.h"
#include "ComServiceSM.h"
#include "FollowTape.h"
#include "PWMDemo.h"
#include "Shooter.h"
#include "Drive.h"
#include "ZoneOneSM.h"
#include "ZoneTwoSM.h"
#include "ZoneThreeSM.h"
#include "ZoneFourSM.h"
#include "ZoneFiveSM.h"
#include "TurnFSM.h"


/*----------------------------- Module Defines ----------------------------*/
#define ENTRY_STATE ZONE_ONE
/*---------------------------- Module Functions ---------------------------*/
static ES_Event DuringZoneOne( ES_Event Event);
static ES_Event DuringZoneTwo( ES_Event Event);
static ES_Event DuringZoneThree( ES_Event Event);
static ES_Event DuringZoneFour( ES_Event Event);
static ES_Event DuringZoneFive( ES_Event Event);
static ES_Event DuringWaiting( ES_Event Event);
static void generalHardwareInit(void);
static void updateGameStatus(ES_Event CurrentEvent);




/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, though if the top level state machine
// is just a single state container for orthogonal regions, you could get
// away without it
static StrategyState_t CurrentState;
static StrategyState_t LastState;
// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

static bool PauseHasHappened = false;
static bool shootingComplete = false;
static bool obstacleComplete = false;
static int lapsRemaining = 5;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitStrategySM

 Parameters
     uint8_t : the priorty of this service

 Returns
     boolean, False if error in initialization, True otherwise

 Description
     Saves away the priority,  and starts
     the top level state machine
 Notes

 Author
     J. Edward Carryer, 02/06/12, 22:06
****************************************************************************/
bool InitStrategySM ( uint8_t Priority )
{
  ES_Event ThisEvent;

  MyPriority = Priority;  // save our priority
	LastState = ENTRY_STATE;
	
  ThisEvent.EventType = ES_ENTRY;
  // Start the Strategy State machine

	generalHardwareInit();
  StartStrategySM( ThisEvent );

  return true;
}

/****************************************************************************
 Function
     PostStrategySM

 Parameters
     ES_Event ThisEvent , the event to post to the queue

 Returns
     boolean False if the post operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostStrategySM( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunStrategySM

 Parameters
   ES_Event: the event to process

 Returns
   ES_Event: an event to return

 Description
   the run function for the top level state machine 
 Notes
   uses nested switch/case to implement the machine.
 Author
   J. Edward Carryer, 02/06/12, 22:09
****************************************************************************/
ES_Event RunStrategySM( ES_Event CurrentEvent )
{
   bool MakeTransition = false;/* are we making a state transition? */
   StrategyState_t NextState = CurrentState;

   ES_Event EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
   ES_Event ReturnEvent = { ES_NO_EVENT, 0 }; // assume no error
	 
   switch ( CurrentState )
   {
				
			  case WAITING_STATE:	
					
				
					printf("WAITING_STATE \r\n");
				  CurrentEvent = DuringWaiting(CurrentEvent);
					if ( CurrentEvent.EventType != ES_NO_EVENT ) 
					{
            switch (CurrentEvent.EventType)
           
						{	
						
               case FlagDropped: 
								  printf("Flag Dropped\r\n");
                  NextState = LastState;
                  MakeTransition = true; 
									if(PauseHasHappened){
										EntryEventKind.EventType = ES_ENTRY_HISTORY;
									}
									else EntryEventKind.EventType = ES_ENTRY;
									
									EntryEventKind.EventParam = CurrentState;
                  break;
					
            }
					}
         break;
	    	case ZONE_ONE :   
					printf("ZONE_ONE \r\n");
         CurrentEvent = DuringZoneOne(CurrentEvent);
         if ( CurrentEvent.EventType != ES_NO_EVENT )
         {
            switch (CurrentEvent.EventType)
            {
               case ZoneUpdate:
									printf("Zone Update, Zone %i Entered\r\n", CurrentEvent.EventParam);
									if(CurrentEvent.EventParam == 2){
										NextState = ZONE_TWO;
										MakeTransition = true; 
										EntryEventKind.EventType = ES_ENTRY;
										EntryEventKind.EventParam = CurrentState;
									}
									else if (CurrentEvent.EventParam == 4){
										NextState = ZONE_FOUR;
										MakeTransition = true; 
										EntryEventKind.EventType = ES_ENTRY;
										EntryEventKind.EventParam = CurrentState;
									}
                  break;
							 
							}
						}						
         break;
				 case ZONE_TWO :  
					printf("ZONE_TWO \r\n");
         CurrentEvent = DuringZoneTwo(CurrentEvent);
         if ( CurrentEvent.EventType != ES_NO_EVENT ) 
         {
            switch (CurrentEvent.EventType)
            {
                 case ZoneUpdate:
									 printf("Zone Update, Zone %i Entered\r\n", CurrentEvent.EventParam);
									if(CurrentEvent.EventParam == 3){
										NextState = ZONE_THREE;
										MakeTransition = true; 
										EntryEventKind.EventType = ES_ENTRY;
										EntryEventKind.EventParam = CurrentState;
									}else if(CurrentEvent.EventParam == 1){
										NextState = ZONE_ONE;
										MakeTransition = true; 
										EntryEventKind.EventType = ES_ENTRY;
										EntryEventKind.EventParam = CurrentState;
									}
									
									break;
								}
					}
         break;
				 case ZONE_THREE :
					 printf("ZONE_THREE \r\n");
         CurrentEvent = DuringZoneThree(CurrentEvent);
         if ( CurrentEvent.EventType != ES_NO_EVENT ) 
         {
            switch (CurrentEvent.EventType)
            {
                 case ZoneUpdate:
									 printf("Zone Update, Zone %i Entered\r\n", CurrentEvent.EventParam);
									if(CurrentEvent.EventParam == 2){
										NextState = ZONE_TWO;
										MakeTransition = true; 
										EntryEventKind.EventType = ES_ENTRY;
										EntryEventKind.EventParam = CurrentState;
									}
									
									break;

								}
					}
				 
         break;

        case ZONE_FOUR: 
					printf("ZONE_FOUR \r\n");
         CurrentEvent = DuringZoneFour(CurrentEvent);
         if ( CurrentEvent.EventType != ES_NO_EVENT )
         {
            switch (CurrentEvent.EventType)
            {
                case ZoneUpdate:
									printf("Zone Update, Zone %i Entered\r\n", CurrentEvent.EventParam);
									if(CurrentEvent.EventParam == 1){
										NextState = ZONE_ONE;
										MakeTransition = true; 
										EntryEventKind.EventType = ES_ENTRY;
										EntryEventKind.EventParam = CurrentState;
									}
									else if(CurrentEvent.EventParam == 5){
										NextState = ZONE_FIVE;
										MakeTransition = true; 
										EntryEventKind.EventType = ES_ENTRY;
										EntryEventKind.EventParam = CurrentState;
									}
								break;
					}
			 }
				 
         break;
		     case ZONE_FIVE : 
					 printf("ZONE_FIVE \r\n");
         CurrentEvent = DuringZoneFive(CurrentEvent);
         if ( CurrentEvent.EventType != ES_NO_EVENT ) 
         {
            switch (CurrentEvent.EventType)
            {
              case ZoneUpdate:
									printf("Zone Update, Zone %i Entered\r\n", CurrentEvent.EventParam);
									if(CurrentEvent.EventParam == 1){
										NextState = ZONE_ONE;
										MakeTransition = true; 
										EntryEventKind.EventType = ES_ENTRY;
										EntryEventKind.EventParam = CurrentState;
									}
							break;
						}
			     }
         break;
			 
		
			}	
		updateGameStatus(CurrentEvent);
		
		if(CurrentEvent.EventType == CautionFlag || CurrentEvent.EventType == RaceOver){
			
			if(CurrentEvent.EventType == CautionFlag)printf("Caution Flag\r\n");
			else printf("Race Over\r\n");
			StopMotor();
			StopFlyWheel();
			NextState = WAITING_STATE;
			MakeTransition = true;	
			PauseHasHappened = true;
			if(CurrentEvent.EventType == RaceOver) {
				 LastState = ZONE_ONE;
				 PauseHasHappened = false;
				//printf("Last State = %i Current State = %i\r\n", LastState, CurrentState);
			 }
		}

    //   If we are making a state transition
    if (MakeTransition == true)
    {
       //   Execute exit function for current state
			if(CurrentEvent.EventType == RaceOver)LastState = ZONE_ONE;
			else LastState = CurrentState;
       CurrentEvent.EventType = ES_EXIT;
       RunStrategySM(CurrentEvent);
		
			
				
       CurrentState = NextState; //Modify state variable
       // Execute entry function for new state
       // this defaults to ES_ENTRY
       RunStrategySM(EntryEventKind);
     }
   // in the absence of an error the top level state machine should
   // always return ES_NO_EVENT, which we initialized at the top of func
   return(ReturnEvent);
}
	 

/****************************************************************************
 Function
     StartStrategySM

 Parameters
     ES_Event CurrentEvent

 Returns
     nothing

 Description
     Does any required initialization for this state machine
 Notes

 Author
     J. Edward Carryer, 02/06/12, 22:15
****************************************************************************/
void StartStrategySM ( ES_Event CurrentEvent )
{
  // if there is more than 1 state to the top level machine you will need 
  // to initialize the state variable
	CurrentState = WAITING_STATE;
	
  // now we need to let the Run function init the lower level state machines
  // use LocalEvent to keep the compiler from complaining about unused var
  RunStrategySM(CurrentEvent);
  return;
}


/***************************************************************************
 private functions
 ***************************************************************************/


static ES_Event DuringWaiting( ES_Event Event){
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
		
    }
    else if ( Event.EventType == ES_EXIT )
    {
			
    }else
    {
			
    }
    return(ReturnEvent);
	}

static ES_Event DuringZoneOne( ES_Event Event)
{
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
			 StartZoneOneSM(ReturnEvent);
    }
    else if ( Event.EventType == ES_EXIT )
    {
    }else
    {
				ReturnEvent = RunZoneOneSM(Event);
    }
    return(ReturnEvent);
}

static ES_Event DuringZoneTwo( ES_Event Event)
{
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
			//If last state was zone 3 then don't shoot-> follow tape
			//If last state was zone 1 check shot made status then check other karts
			
			StartZoneTwoSM(Event);
    }
    else if ( Event.EventType == ES_EXIT )
    {
			
    }else
    {
			RunZoneTwoSM(Event);
    }
    return(ReturnEvent);
}

static ES_Event DuringZoneThree( ES_Event Event)
{
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
			//only way I can enter this zone is through zone2	
			StartZoneThreeSM(Event);
    }
    else if ( Event.EventType == ES_EXIT )
    {
     
    }else
    {
			//tape found
			//timeout
			//beacon detected : stop motor
			ReturnEvent = RunZoneThreeSM(Event);
    }
    return(ReturnEvent);
}


static ES_Event DuringZoneFour( ES_Event Event)
{
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
			//only way I can enter here is through zone 1
			// check [obstacle cross], ultrasound, then check occupied
				StartZoneFourSM(Event);
    }
    else if ( Event.EventType == ES_EXIT )
    {
			//exit to either zone1 or zone 5
     
    }else
    {
				RunZoneFourSM(Event);
    }
    return(ReturnEvent);
}


static ES_Event DuringZoneFive( ES_Event Event)
{
    ES_Event ReturnEvent = Event; 
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
			//enter from zone four
				StartZoneFiveSM(Event);
    }
    else if ( Event.EventType == ES_EXIT )
    {
			//exit to zone1
     
    }else
    {
				RunZoneFiveSM(Event);
    }
    return(ReturnEvent);
}


static void generalHardwareInit(void){
	InitShooter();
	InitDrive();
  StopMotor();
}

static void updateGameStatus(ES_Event CurrentEvent){
 
		if(CurrentEvent.EventType == LapUpdate){
			lapsRemaining = CurrentEvent.EventParam;
			printf("Lap Update, Laps Remaining: %i\r\n", lapsRemaining);
		}
		
		if(CurrentEvent.EventType == TargetComplete){
			shootingComplete = true;
			printf("Target Complete\r\n");
		}
		
		if(CurrentEvent.EventType == ObstacleComplete){
			obstacleComplete = true;
			printf("Obstacle Complete\r\n");
		}
			 
}

bool TargetCompletionStatus(void){
	return shootingComplete;
}
bool ObstacleCompletionStatus(void){
	return obstacleComplete;
}
int getLapsRemaining(void){
	return lapsRemaining;
}

