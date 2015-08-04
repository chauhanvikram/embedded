/****************************************************************************
 Module
   ComServiceSM.c

 Revision
   2.0.1

 Description
   This is a template for the top level Hierarchical state machine

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 02/08/12 01:39 jec      converted from MW_MasterMachine.c
 02/06/12 22:02 jec      converted to Gen 2 Events and Services Framework
 02/13/10 11:54 jec      converted During functions to return Event_t
                         so that they match the template
 02/21/07 17:04 jec      converted to pass Event_t to Start...()
 02/20/07 21:37 jec      converted to use enumerated type for events
 02/21/05 15:03 jec      Began Coding
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "TopHSMTemplate.h"

#include "termio.h"
#include "inc/hw_memmap.h" 
#include "inc/hw_types.h" 
#include "inc/hw_sysctl.h" 
#include "inc/hw_gpio.h" 
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <BITDEFS.h>
#define ALL_BITS (0xff<<2) 

//#include "SendByteSM.h"


/*----------------------------- Module Defines ----------------------------*/
#define WAIT_TIME 1000
#define LED1_BIT BIT0HI //LED 1 is connected to PB0
#define LED2_BIT BIT1HI //LED 2 is connected to PB1
#define LED3_BIT BIT2HI //LED 3 is connected to PB2

#define LED1 (HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA + ALL_BITS))& LED1_BIT)
#define LED2 (HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA + ALL_BITS))& LED2_BIT) 
#define LED3 (HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA + ALL_BITS))& LED3_BIT) 

#define Kart_Info_Array_Length 5

#define GAME_STATUS_BYTE_INDEX 1

		//Constants associated with updating the Number of Laps Left for each Kart
#define NUM_LAPS_LEFT_INDEX 0
#define GAME_STATUS_COMMAND_INDEX 0
#define NUM_LAPS_LEFT_SHIFT 0
#define NUM_LAPS_LEFT_MASK 0x07 //00000111

	//Constants associated with updating the Obstacle Status for each Kart
#define OBSTACLE_STATUS_INDEX 3
#define OBSTACLE_STATUS_STATUS_MASK 0x40//01000000
#define OBSTACLE_STATUS_STATUS_SHIFT 6

//Constants associated with updating the Target Status for each Kart
#define TARGET_STATUS_INDEX 2
#define GAME_STATUS_BYTE_INDEX 1
#define TARGET_STATUS_MASK 0x80 //10000000
#define TARGET_STATUS_SHIFT 7

	//Constants associated with updating the Run Status for each Kart
#define RUN_STATUS_INDEX 1
#define RUN_STATUS_MASK 0x18 //00011000
#define RUN_STATUS_SHIFT 3

static uint8_t currentByte;
static uint8_t numToTranslate;



/*---------------------------- Module Functions ---------------------------*/
static ES_Event DuringWaitingState( ES_Event Event);
static ES_Event DuringProcessingState( ES_Event Event);

static ES_Event translateLapNum (uint8_t lapNum);
static uint8_t maskAndShiftByte(uint8_t byte, uint32_t mask, uint8_t shift);

//comment this out after testing
static uint8_t getDataByte(uint8_t com, uint8_t byte);

static void initializeKartStatus(void);
static void updateOurKartStatus(void);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, though if the top level state machine
// is just a single state container for orthogonal regions, you could get
// away without it

typedef struct{
	uint8_t numLaps;
	uint8_t targetStatus;
	uint8_t obstacleStatus;
	uint8_t zone;
	uint8_t kartNumber;
}kart;

static kart our_kart;
static kart kart_a;
static kart kart_b;

static uint8_t receiveData[4][8] = {{0,0xff, 0, 0x6D, 0x6D, 0x6D, 0,0}, 
																		{0,0xff, 0, 0x6D, 0x6D, 0x6D, 0,0},
																		{0,0xff, 0, 0x6D, 0x6D, 0x6D, 0,0},
																		{0,0xff, 0, 0x6D, 0x6D, 0x6D, 0,0}};

static ComState_t CurrentState;
// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitMasterSM

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
bool InitMasterSM ( uint8_t Priority )
{
	
  ES_Event ThisEvent;

  MyPriority = Priority;  // save our priority

  ThisEvent.EventType = ES_ENTRY;
  // Start the Master State machine


  StartMasterSM( ThisEvent );

  return true;
}

/****************************************************************************
 Function
     PostMasterSM

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
bool PostMasterSM( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunMasterSM

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
ES_Event RunMasterSM( ES_Event CurrentEvent )
{
   bool MakeTransition = false;/* are we making a state transition? */
   ComState_t NextState = CurrentState;
   ES_Event EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
   ES_Event ReturnEvent = { ES_NO_EVENT, 0 }; // assume no error

    switch ( CurrentState )
   {
       case WAITING :       // If current state is state one
         // Execute During function for state one. ES_ENTRY & ES_EXIT are
         // processed here allow the lowere level state machines to re-map
         // or consume the event
					
         CurrentEvent = DuringWaitingState(CurrentEvent);
         //process any events
         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case ES_TIMEOUT : //If event is event one
                  if(CurrentEvent.EventParam == COM_TIMER) {
									// Execute action function for state one : event one
                  NextState = PROCESSING;//Decide what the next state will be
                  // for internal transitions, skip changing MakeTransition
                  MakeTransition = true; //mark that we are taking a transition
                  // if transitioning to a state with history change kind of entry
                  // EntryEventKind.EventType = ES_ENTRY_HISTORY;
                  // optionally, consume or re-map this event for the upper
                  // level state machine
                  ReturnEvent.EventType = ES_NO_EVENT;
									}
               break;
                // repeat cases as required for relevant events
            }
         }
       break;
				 
			case PROCESSING:
						// Do the processing
						
							// Execute During function for state one. ES_ENTRY & ES_EXIT are
						 // processed here allow the lowere level state machines to re-map
						 // or consume the event
						 CurrentEvent = DuringProcessingState(CurrentEvent);
						 //process any events
						 if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
						 {
								switch (CurrentEvent.EventType)
								{
									 case DONE_PROCESSING : //If event is event one
											
											// Execute action function for state one : event one
											NextState = WAITING;//Decide what the next state will be
											// for internal transitions, skip changing MakeTransition
											MakeTransition = true; //mark that we are taking a transition
											// if transitioning to a state with history change kind of entry
											// EntryEventKind.EventType = ES_ENTRY_HISTORY;
											// optionally, consume or re-map this event for the upper
											// level state machine
											ReturnEvent.EventType = ES_NO_EVENT;
										
									 break;
										// repeat cases as required for relevant events
								}
						 }
			break;
      // repeat state pattern as required for other states
    }
    //   If we are making a  state transition
    if (MakeTransition == true)
    {
       //   Execute exit function for current state
       CurrentEvent.EventType = ES_EXIT;
       RunMasterSM(CurrentEvent);

       CurrentState = NextState; //Modify state variable

       // Execute entry function for new state
       // this defaults to ES_ENTRY
       RunMasterSM(EntryEventKind);
     }
   // in the absence of an error the top level state machine should
   // always return ES_NO_EVENT, which we initialized at the top of func
   return(ReturnEvent);
}
/****************************************************************************
 Function
     StartMasterSM

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
void StartMasterSM ( ES_Event CurrentEvent )
{	
	// if there is more than 1 state to the top level machine you will need 
  // to initialize the state variable
  CurrentState = WAITING;
	
	//Initialize the Ports B0-2 on the Tiva as a digital input pins
	//to indicate which kart is ours at the start of the race
	HWREG(SYSCTL_RCGCGPIO) |= (SYSCTL_RCGCGPIO_R1);
	puts("\r\nPort B activated");
	int Dummy = HWREG(SYSCTL_RCGCGPIO);
  
	/* set as a digital ports */
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (LED1_BIT|LED2_BIT|LED3_BIT);
  Dummy = HWREG(SYSCTL_RCGCGPIO); 
  
  /* set as input ports*/
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) &= ~(LED1_BIT|LED2_BIT|LED3_BIT);

	
	//decide which Kart is ours and assign names KartA and KartB to opponent Karts
	//this happens once per race
	initializeKartStatus();
  printf("\r\n We are Kart %u", our_kart.kartNumber);

  // now we need to let the Run function init the lower level state machines
  // use LocalEvent to keep the compiler from complaining about unused var
  RunMasterSM(CurrentEvent);
  return;
}


/***************************************************************************
 private functions
 ***************************************************************************/

static ES_Event DuringWaitingState( ES_Event Event)
{
    ES_Event ReturnEvent = Event; // assme no re-mapping or comsumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
        // implement any entry actions required for this state machine
				ES_Timer_InitTimer(COM_TIMER, WAIT_TIME);
        // after that start any lower level machines that run in this state
        //StartLowerLevelSM( Event );
        // repeat the StartxxxSM() functions for concurrent state machines
        // on the lower level
			
				// no lower Level SMs here
    }
    else if ( Event.EventType == ES_EXIT )
    {
        // on exit, give the lower levels a chance to clean up first
        //RunLowerLevelSM(Event);
        // repeat for any concurrently running state machines
        // now do any local exit functionality
      
    }else
    // do the 'during' function for this state
    {
        // run any lower level state machine
        // ReturnEvent = RunLowerLevelSM(Event);
      
        // repeat for any concurrent lower level machines
				printf("\r\n In WAITING");
        // do any activity that is repeated as long as we are in this state
    }
    // return either Event, if you don't want to allow the lower level machine
    // to remap the current event, or ReturnEvent if you do want to allow it.
    return(ReturnEvent);
}

static ES_Event DuringProcessingState( ES_Event Event)
{
    ES_Event ReturnEvent = Event; // assme no re-mapping or comsumption

    // process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
        // implement any entry actions required for this state machine
        
        // after that start any lower level machines that run in this state
        //StartLowerLevelSM( Event );
        // repeat the StartxxxSM() functions for concurrent state machines
        // on the lower level
				//printf("\r\n Entry Event DuringProcessing");
				// no lower Level SMs here
				ES_Event GoProcess;
				GoProcess.EventType = ES_NO_EVENT;
				PostMasterSM(GoProcess);
    }
    else if ( Event.EventType == ES_EXIT )
    {
        // on exit, give the lower levels a chance to clean up first
        //RunLowerLevelSM(Event);
        // repeat for any concurrently running state machines
        // now do any local exit functionality
				//printf("\r\n Entry Event ExitProcessing");
    }else
    // do the 'during' function for this state
    {
        // run any lower level state machine
        // ReturnEvent = RunLowerLevelSM(Event);
				printf("\r\n In PROCESSING");
        // repeat for any concurrent lower level machines
				//printf("\r\n Else DuringProcessing");
        // do any activity that is repeated as long as we are in this state
				updateOurKartStatus();
				ES_Event DoneProcessing;
				DoneProcessing.EventType = DONE_PROCESSING;
				PostMasterSM(DoneProcessing);
    }
    // return either Event, if you don't want to allow the lower level machine
    // to remap the current event, or ReturnEvent if you do want to allow it.
    return(ReturnEvent);
}


/**
* Returns byte in receiveData[][].
* @param com command (row) in receiveData
* @param byte byte (column) in receiveData
* @return byte from receiveData[][]
*/

//0x6D = 1101101
	
uint8_t getDataByte(uint8_t com, uint8_t byte) {
	return receiveData[com][byte];
}

static uint8_t maskAndShiftByte(uint8_t byte, uint32_t mask, uint8_t shift){
	return (byte &= mask)>>shift;
}


static void updateOurKartStatus(){
	currentByte = getDataByte(GAME_STATUS_COMMAND_INDEX,(our_kart.kartNumber+GAME_STATUS_BYTE_INDEX));
	
	//Update Number of Laps
		uint8_t numLapsLeft = maskAndShiftByte (currentByte, NUM_LAPS_LEFT_MASK, NUM_LAPS_LEFT_SHIFT);
		if(our_kart.numLaps != numLapsLeft){
				ES_Event ThisEvent;
				ThisEvent.EventType = LAP_UPDATE;
				ThisEvent.EventParam = numToTranslate;
				PostStrategySM(ThisEvent);
				our_kart.numLaps = numLapsLeft;
			}
		//Update Obstacle Status only do this until the obstacle has been made
		 if(our_kart.obstacleStatus == 0){
				uint8_t obstacleStatus = maskAndShiftByte(currentByte, OBSTACLE_STATUS_STATUS_MASK, OBSTACLE_STATUS_STATUS_SHIFT);
				if(obstacleStatus!=our_kart.obstacleStatus){
					ES_Event ThisEvent;
					ThisEvent.EventType = OBSTACLE_COMPLETE;
					PostStrategySM(ThisEvent);
					our_kart.obstacleStatus = 1;
				}
			}
		//Update TargetStatus
		if(our_kart.targetStatus ==0){
			uint8_t targetStatus = maskAndShiftByte (currentByte, TARGET_STATUS_MASK, TARGET_STATUS_SHIFT);	
			if(targetStatus!=our_kart.targetStatus){
				ES_Event ThisEvent;
				ThisEvent.EventType = TARGET_COMPLETE;
				PostStrategySM(ThisEvent);
				our_kart.targetStatus = 1;
			}
		} 
		
		//Update Run Status
			ES_Event ThisEvent;
			uint8_t runStatus = maskAndShiftByte (currentByte, RUN_STATUS_MASK, RUN_STATUS_SHIFT);
			if(runStatus == 0x01){
				ThisEvent.EventType = FLAG_DROPPED;
				PostStrategySM(ThisEvent);
			}
			else if(runStatus ==0x02){
				ThisEvent.EventType = CAUTION_FLAG;
				PostStrategySM(ThisEvent);
			}
			else if(runStatus == 0x03){
				ThisEvent.EventType = RACE_OVER;
				PostStrategySM(ThisEvent);
			}
			
}

static void initializeKartStatus(void){
	our_kart.numLaps = 5;
	our_kart.obstacleStatus = 0;
	our_kart.targetStatus = 0;
	our_kart.zone = 1;
	
	kart_a.numLaps = 5;
	kart_a.obstacleStatus = 0;
	kart_a.targetStatus = 0;
	kart_a.zone = 1;
	
	kart_b.numLaps = 5;
	kart_b.obstacleStatus = 0;
	kart_b.targetStatus = 0;
	kart_b.zone = 1;

	if (LED1 == 1){
		our_kart.kartNumber = 1;
		kart_a.kartNumber = 2;
		kart_b.kartNumber = 3;
	}
	else if (LED2 == 1){
		our_kart.kartNumber = 2;
		kart_a.kartNumber = 1;
		kart_b.kartNumber = 3;
		}
	else {
	  our_kart.kartNumber = 3;
		kart_a.kartNumber = 1;
		kart_b.kartNumber = 2;	
		}
}

