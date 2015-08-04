/****************************************************************************
 Module
   ComServiceSM.c

 Revision
   2.0.1

 Description
   This is a template for the top level Hierarchical state machine

 Notes

TODO_FOR_COM_SERVICE

1) Test With DRS 



****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"

#include "UpdateSM.h"


#include "ComServiceSM.h"
#include "termio.h"
#include "inc/hw_memmap.h" 
#include "inc/hw_types.h" 
#include "inc/hw_sysctl.h" 
#include "inc/hw_gpio.h" 
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <BITDEFS.h>
#include "SendByteSM.h"
#include "StrategySM.h"
#include "TurnFSM.h"
#include "Shooter.h"

/*----------------------------- Module Defines ----------------------------*/

#define WAIT_TIME 150
#define KART_ONE_BIT BIT0HI //LED 1 is connected to PB0
#define KART_TWO_BIT BIT1HI //LED 2 is connected to PB1
#define KART_THREE_BIT BIT2HI //LED 3 is connected to PB2

#define KART_PORT (HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA + ALL_BITS)))

//#define KART_ONE 1 // pink/green
//#define KART_TWO 0 // orange/yellow
//#define KART_THREE 0 // blue/yellow



typedef struct{
	uint8_t numLaps;
	uint8_t targetStatus;
	uint8_t obstacleStatus;
	uint8_t zone;
	uint8_t kartNumber;
	uint8_t game_status_byte;
	uint8_t position_command_index;
	uint16_t currentXPosition;
	uint16_t currentYPosition;
	int16_t currentOrientation;
}Kart;


/*---------------------------- Module Functions ---------------------------*/
static ES_Event DuringWaitingState( ES_Event Event);
static ES_Event DuringProcessingState( ES_Event Event);

static uint8_t maskAndShiftByte(uint8_t byte, uint32_t mask, uint8_t shift);

static void initializeKartStatus(void);
static void updateOurKartStatus(void);
static void updateKartZone(void);
static void initializePins(void);
static void posDataUpdate(Kart* thisKart);
static int determineZone(uint16_t Xpos, uint16_t Ypos, Kart* kart);


/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, though if the top level state machine
// is just a single state container for orthogonal regions, you could get
// away without it

static int kart_one = 0;
static int kart_two = 0;
static int kart_three = 0;
static bool listenToMostlyCrossed = false;
static bool listenToCenterOfObstacle = false;
static bool listenToPostObstacleZoneOne = false;
static uint8_t RunStatus = 0x00;
static Kart our_kart;
static Kart kart_a;
static Kart kart_b;

static ComState_t CurrentState;
// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

//determine the size of the playing field

//-1 corresponds to X-positive in the East direction & Y-positive in the South direction

//********************************************** DYNAMIC ZONE CONSTANTS ***************************************************************/


static uint16_t zone_three_x_max = ZONE_3_X_MAX;
static uint16_t zone_three_x_min = ZONE_3_X_MIN;
static uint16_t zone_three_y_max = ZONE_3_Y_MAX;
static uint16_t zone_three_y_min = ZONE_3_Y_MIN;

static uint16_t zone_two_x_max = ZONE_2_X_MAX;
static uint16_t zone_two_y_min = ZONE_2_Y_MIN;
static uint16_t zone_two_y_max = ZONE_2_Y_MAX;

static uint16_t zone_four_x_max = ZONE_4_X_MAX;
static uint16_t zone_four_x_min = ZONE_4_X_MIN;
static uint16_t zone_four_y_min = ZONE_4_Y_MIN;

static uint16_t zone_five_y_min = ZONE_5_Y_MIN;
static uint16_t zone_five_y_max = ZONE_5_Y_MAX;
static uint16_t	zone_five_x_min = ZONE_5_X_MIN;


/*------------------------------ Module Code ------------------------------*/

static void initGameStatusLED(void);

void setZonesForAlign(int currentZone, int targetZone,bool reset){
	if(reset){
	  zone_three_x_max = ZONE_3_X_MAX;
		zone_three_x_min = ZONE_3_X_MIN;
		zone_three_y_max = ZONE_3_Y_MAX;
		zone_three_y_min = ZONE_3_Y_MIN;

		zone_two_x_max = ZONE_2_X_MAX;
		zone_two_y_min = ZONE_2_Y_MIN;
		zone_two_y_max = ZONE_2_Y_MAX;

		zone_four_x_max = ZONE_4_X_MAX;
		zone_four_x_min = ZONE_4_X_MIN;
		zone_four_y_min = ZONE_4_Y_MIN;

		zone_five_y_min = ZONE_5_Y_MIN;
		zone_five_y_max = ZONE_5_Y_MAX;
		zone_five_x_min = ZONE_5_X_MIN;
		return;
	}
		
			
	if(targetZone == 3 && currentZone == 2){
			zone_two_x_max = COURT_X_MAX;
			zone_two_y_min = COURT_Y_MIN;
			zone_two_y_max = COURT_Y_MAX;
			
		zone_three_x_max = 0;
		zone_three_x_min = 0;
		zone_three_y_max = 0;
		zone_three_y_min = 0;

		zone_four_x_max = 0;
		zone_four_x_min = 0;
		zone_four_y_min = 0;

		zone_five_y_min = 0;
		zone_five_y_max = 0;
		zone_five_x_min = 0;
		

			return;
	}
	if(targetZone == 2 && currentZone == 3){
			zone_three_x_max = COURT_X_MAX;
			zone_three_x_min = COURT_X_MIN;
			zone_three_y_max = COURT_Y_MAX;
			zone_three_y_min = COURT_Y_MIN;
		

		zone_two_x_max = 0;
		zone_two_y_min = 0;
		zone_two_y_max =0;

		zone_four_x_max = 0;
		zone_four_x_min =0;
		zone_four_y_min = 0;

		zone_five_y_min = 0;
		zone_five_y_max = 0;
		zone_five_x_min = 0;
		
		
			return;
	}
	if(targetZone == 1 && currentZone == 5){
		
		zone_three_x_max = 0;
		zone_three_x_min = 0;
		zone_three_y_max = 0;
		zone_three_y_min = 0;
		

		zone_two_x_max = 0;
		zone_two_y_min = 0;
		zone_two_y_max =0;

		zone_four_x_max = 0;
		zone_four_x_min =0;
		zone_four_y_min = 0;

		//zone_five_y_min = 48;
		zone_five_y_min = 53;
			return;
	}
	if(targetZone == 5 && currentZone == 1){
			zone_three_x_max = 0;
			zone_three_x_min = 0;
			zone_three_y_max = 0;
			zone_three_y_min = 0;
		

		zone_two_x_max = 0;
		zone_two_y_min = 0;
		zone_two_y_max =0;

		zone_four_x_max = 0;
		zone_four_x_min =0;
		zone_four_y_min = 0;

		zone_five_y_min = 0;
		zone_five_y_max = 0;
		zone_five_x_min = 0;
		
			return;
	}
	if(targetZone == 1 && currentZone == 4){
			zone_four_x_max = COURT_X_MAX;
      zone_four_x_min = COURT_X_MIN;
      zone_four_y_min = COURT_Y_MIN;
		
			zone_three_x_max = 0;
			zone_three_x_min = 0;
			zone_three_y_max = 0;
			zone_three_y_min = 0;

		zone_two_x_max = 0;
		zone_two_y_min = 0;
		zone_two_y_max =0;

		zone_five_y_min = 0;
		zone_five_y_max = 0;
		zone_five_x_min = 0;
		

			return;
	}
	
}


uint8_t getKartAZone(void){
	return kart_a.zone;
}
uint8_t getKartBZone(void){
	return kart_b.zone;
}												
uint16_t getOurKartY(void){
	return our_kart.currentYPosition;
}
uint16_t getOurKartX(void){
	return our_kart.currentXPosition;
}

uint8_t getOurZone(void){
	return our_kart.zone;
}

uint16_t getOurKartAbsoluteOrientation(void){
	int orientation = our_kart.currentOrientation;
	if(orientation < 0)orientation += 360;
	return orientation;
}

void setComeListenToPostFive(bool listen){
	listenToPostObstacleZoneOne = listen;
}

void setMiddleOfObstacle(bool listen){
	listenToCenterOfObstacle = listen;
}

void setListenToCrossOverPoint(bool listen){
	listenToMostlyCrossed = listen;
	
}

/****************************************************************************
 Function
     InitComServiceSM

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
bool InitComServiceSM ( uint8_t Priority )
{
	
  ES_Event ThisEvent;
  MyPriority = Priority;  // save our priority
  ThisEvent.EventType = ES_ENTRY;
	

	
  // Start the ComService State machine
  StartComServiceSM( ThisEvent );
  return true;
}

/****************************************************************************
 Function
     PostComServiceSM

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
bool PostComServiceSM( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunComServiceSM

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
ES_Event RunComServiceSM( ES_Event CurrentEvent )
{
   bool MakeTransition = false;/* are we making a state transition? */
   ComState_t NextState = CurrentState;
   ES_Event EntryEventKind = { ES_ENTRY, 0 };// default to normal entry to new state
   ES_Event ReturnEvent = { ES_NO_EVENT, 0 }; // assume no error

    switch ( CurrentState )
   {
       case WAITING : 
         CurrentEvent = DuringWaitingState(CurrentEvent);

         if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
         {
            switch (CurrentEvent.EventType)
            {
               case ES_TIMEOUT : 
                  if(CurrentEvent.EventParam == COM_TIMER) {
								
                  NextState = PROCESSING;
                  MakeTransition = true; 
									}
               break;
            }
         }
       break;
				 
			case PROCESSING:
						 CurrentEvent = DuringProcessingState(CurrentEvent);
						 if ( CurrentEvent.EventType != ES_NO_EVENT ) //If an event is active
						 {
								switch (CurrentEvent.EventType)
								{
									 case DoneProcessing :
											NextState = WAITING;
											MakeTransition = true;	
									 break;
								 }
						 }
			break;
    }
    //   If we are making a  state transition
    if (MakeTransition == true)
    {
       //   Execute exit function for current state
       CurrentEvent.EventType = ES_EXIT;
       RunComServiceSM(CurrentEvent);
       CurrentState = NextState;
       RunComServiceSM(EntryEventKind);
     }

   return(ReturnEvent);
}
/****************************************************************************
 Function
     StartComServiceSM

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
void StartComServiceSM ( ES_Event CurrentEvent )
{	
	// if there is more than 1 state to the top level machine you will need 
  // to initialize the state variable
  CurrentState = WAITING;
	initializePins();
	initGameStatusLED();
	initializeKartStatus();
  printf("We are Kart %u\r\n", our_kart.kartNumber);
	
  // now we need to let the Run function init the lower level state machines
  // use LocalEvent to keep the compiler from complaining about unused var
  RunComServiceSM(CurrentEvent);
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
				ES_Timer_InitTimer(COM_TIMER, WAIT_TIME);
			
				ES_Event Event;
				Event.EventType = BotUpdate;
				PostUpdateSM(Event);

    }
    else if ( Event.EventType == ES_EXIT )
    {

      
    }else
    {
      
    }

    return(ReturnEvent);
}

static ES_Event DuringProcessingState( ES_Event Event)
{
    ES_Event ReturnEvent = Event; // assme no re-mapping or comsumption
    if ( (Event.EventType == ES_ENTRY) ||
         (Event.EventType == ES_ENTRY_HISTORY) )
    {
				updateOurKartStatus();
				//printf("\rOur Kart %i Game Status:\n\r Target Status = 0x%#08x \n\r Obstacle Status = 0x%#08x\n\r Run Status = 0x%#08x\n\r", our_kart.kartNumber,
					//our_kart.targetStatus, our_kart.obstacleStatus, RunStatus);
				updateKartZone();
				//printf("\rOur Current Location is X = %i, Y = %i, O = %i, z = %i\n\r",our_kart.currentXPosition,
					//our_kart.currentYPosition,our_kart.currentOrientation,our_kart.zone);
			
				//printf("\rKart A Location is X = %i, Y = %i, O = %i\n\r",kart_a.currentXPosition,
					//kart_a.currentYPosition,kart_a.currentOrientation);
			
				//printf("\rKart B Location is X = %i, Y = %i, O = %i\n\r",kart_b.currentXPosition,
					//kart_b.currentYPosition,kart_b.currentOrientation);
				ES_Event Done;
				Done.EventType = DoneProcessing;
				PostComServiceSM(Done);
    }
    else if ( Event.EventType == ES_EXIT )
    {
    }else
    {
			
    }

    return(ReturnEvent);
}

/**
* Returns byte in receiveData[][].
* @param com command (row) in receiveData
* @param byte byte (column) in receiveData
* @return byte from receiveData[][]
*/
	

static uint8_t maskAndShiftByte(uint8_t byte, uint32_t mask, uint8_t shift){
	return (byte &= mask)>>shift;
}

static void updateKartZone(void){
	posDataUpdate(&our_kart);
	//printf("Xpos: %i Ypos: %i Zone: %i \r\n", our_kart.currentXPosition, our_kart.currentYPosition, our_kart.zone);
	posDataUpdate(&kart_a);
	posDataUpdate(&kart_b);
}


static void posDataUpdate(Kart* thisKart){
	thisKart->currentXPosition = 0;
	thisKart->currentYPosition = 0;
	thisKart->currentOrientation = 0;
	uint16_t zoneNumber = 0;
	uint16_t XPosition = 0x0000;
	uint16_t YPosition = 0x0000;
	int16_t orientation = 0x0000;
	XPosition|= getDataByte(thisKart->position_command_index, POS_X_M_INDEX)<<8;
	XPosition|= getDataByte(thisKart->position_command_index, POS_X_L_INDEX);
	
	YPosition|= getDataByte(thisKart->position_command_index, POS_Y_M_INDEX)<<8;
	YPosition|= getDataByte(thisKart->position_command_index, POS_Y_L_INDEX);
	orientation|= getDataByte(thisKart->position_command_index, ORIENT_M_INDEX)<<8;
	orientation|= getDataByte(thisKart->position_command_index, ORIENT_L_INDEX);
	
	thisKart->currentXPosition = XPosition;
	thisKart->currentYPosition = YPosition;
	thisKart->currentOrientation = orientation;
	
	zoneNumber = determineZone(XPosition,YPosition, thisKart);
	
	if(thisKart->kartNumber == our_kart.kartNumber){
		if(zoneNumber != thisKart->zone){
			ES_Event Event;
			Event.EventType = ZoneUpdate;
			Event.EventParam = zoneNumber;
			PostStrategySM(Event);
			PostTurningFSM(Event);
			thisKart->zone = zoneNumber;
		}
	}
}

static int determineZone(uint16_t Xpos, uint16_t Ypos, Kart* kart){

	if(Xpos >= ZONE_2_X_MIN && Xpos  < zone_two_x_max && Ypos >= zone_two_y_min  && Ypos < zone_two_y_max){
		//printf("Zone 2\r\n");
		return 2;
	}
	if(Xpos >= zone_three_x_min && Xpos  < zone_three_x_max && Ypos >= zone_three_y_min && Ypos < zone_three_y_max) {
		//printf("Zone 3\r\n");
		return 3;
	}
	if(Xpos >= zone_four_x_min && Xpos  < zone_four_x_max && Ypos >= zone_four_y_min && Ypos < ZONE_4_Y_MAX) {
		
		if(Xpos >= CENTER_OF_OBSTACLE_POINT_X && kart->kartNumber == our_kart.kartNumber &&
				listenToCenterOfObstacle){ //&&getKartAZone()!=5 &&  getKartBZone() !=5){
						ES_Event Event;
						Event.EventType = CenterOfObstacle;
					  PostStrategySM(Event);
						listenToCenterOfObstacle = false;
			}
		//printf("Zone 4\r\n"); 
		return 4;
	}
	if(Xpos >= zone_five_x_min && Xpos  < ZONE_5_X_MAX && Ypos >= zone_five_y_min && Ypos < zone_five_y_max){
			
			if(Ypos <= OBSTACLE_PAST_CROSSOVER_POINT_Y && kart->kartNumber == our_kart.kartNumber &&
				listenToMostlyCrossed){
						ES_Event Event;
						Event.EventType = ObstacleMostlyCrossed;
					  PostStrategySM(Event);
						listenToMostlyCrossed = false;
			}
			//printf("Zone 5\r\n"); 
			return 5;
	}
	
	else if(listenToPostObstacleZoneOne && Ypos <= ZONE_ONE_FROM_ZONE_FIVE_STOP_Y && kart->kartNumber == our_kart.kartNumber){
						ES_Event Event;
						Event.EventType = ZoneOneStop;
					  PostStrategySM(Event);
						listenToPostObstacleZoneOne = false;
			}
	//printf("Zone 1\r\n"); 
	return 1;
}


static void updateOurKartStatus(void){
	uint8_t gameStatusByte = getDataByte(GAME_STATUS_COM_INDEX,our_kart.game_status_byte);
	
	//Update Number of Laps
		uint8_t numLapsLeft = maskAndShiftByte (gameStatusByte, NUM_LAPS_LEFT_MASK, NUM_LAPS_LEFT_SHIFT);
		if(our_kart.numLaps != numLapsLeft){
				ES_Event ThisEvent;
				ThisEvent.EventType = LapUpdate;
				ThisEvent.EventParam = numLapsLeft;
				PostStrategySM(ThisEvent);
				our_kart.numLaps = numLapsLeft;
			}
		//Update Obstacle Status only do this until the obstacle has been made
		 if(our_kart.obstacleStatus == 0){
				uint8_t obstacleStatus = maskAndShiftByte(gameStatusByte, OBSTACLE_STATUS_STATUS_MASK, OBSTACLE_STATUS_STATUS_SHIFT);
				if(obstacleStatus!=our_kart.obstacleStatus){
					ES_Event ThisEvent;
					ThisEvent.EventType = ObstacleComplete;
					PostStrategySM(ThisEvent);
					our_kart.obstacleStatus = 1;
				}
			}
		//Update TargetStatus
		if(our_kart.targetStatus ==0){
			uint8_t targetStatus = maskAndShiftByte (gameStatusByte, TARGET_STATUS_MASK, TARGET_STATUS_SHIFT);	
			//printf("\r0x%#08x = Game_Status_Byte,  0x%#08x = targetStatus\n", gameStatusByte, targetStatus);
			if(targetStatus!=our_kart.targetStatus){
				ES_Event ThisEvent;
				ThisEvent.EventType = TargetComplete;
				PostStrategySM(ThisEvent);
				our_kart.targetStatus = 1;
			}
		} 
		//Update Run Status
			ES_Event ThisEvent;
			uint8_t status = maskAndShiftByte (gameStatusByte, RUN_STATUS_MASK, RUN_STATUS_SHIFT);
			if(status!= RunStatus){
				RunStatus = status;
				if(status == 0x01){
					ThisEvent.EventType = FlagDropped;
			    PostStrategySM(ThisEvent);	
					HWREG(GPIO_PORTB_BASE +(GPIO_O_DATA + ALL_BITS))|= (BIT3HI);
				}
				else if(status ==0x02){
					ThisEvent.EventType = CautionFlag;
				
				 	PostStrategySM(ThisEvent);
					PostTurningFSM(ThisEvent);
					HWREG(GPIO_PORTB_BASE +(GPIO_O_DATA + ALL_BITS))&= ~(BIT3HI);
				}
				else if(status == 0x03){
					ThisEvent.EventType = RaceOver;
			
					PostStrategySM(ThisEvent);
					PostTurningFSM(ThisEvent);
					HWREG(GPIO_PORTB_BASE +(GPIO_O_DATA + ALL_BITS))&= ~(BIT3HI);
				}
			}
			
}

//decide which Kart is ours and assign names KartA and KartB to opponent Karts
//this happens once per race
static void initializeKartStatus(void){
	
	kart_one = KART_PORT &=KART_ONE_BIT;
	kart_two = KART_PORT &=KART_TWO_BIT;
	kart_three = KART_PORT &= KART_THREE_BIT; 
	
	our_kart.numLaps = 5;
	our_kart.obstacleStatus = 0;
	our_kart.targetStatus = 0;
	our_kart.zone = 1;
	our_kart.currentXPosition = 0x0000;
	our_kart.currentYPosition = 0x0000;
	our_kart.currentOrientation = 0x0000;
	
	kart_a.zone = 1;
	kart_a.currentXPosition = 0x0000;
	kart_a.currentYPosition = 0x0000;
	kart_a.currentOrientation = 0x0000;
	
	kart_b.zone = 1;
	kart_b.currentXPosition = 0x0000;
	kart_b.currentYPosition = 0x0000;
	kart_b.currentOrientation = 0x0000;

	if (kart_one != 0){
		our_kart.kartNumber = 1;
		our_kart.position_command_index = KART_ONE_POS_COM_INDEX;
		our_kart.game_status_byte = KART_ONE_MATCH_STATUS_BYTE;
		kart_a.kartNumber = 2;
		kart_b.kartNumber = 3;
	}
	else if (kart_two != 0){
		our_kart.kartNumber = 2;
		our_kart.position_command_index = KART_TWO_POS_COM_INDEX;
		our_kart.game_status_byte = KART_TWO_MATCH_STATUS_BYTE;
		kart_a.kartNumber = 1;
		kart_a.position_command_index = KART_ONE_POS_COM_INDEX;
		kart_b.kartNumber = 3;	
		kart_b.position_command_index = KART_THREE_POS_COM_INDEX;
		}
	else if (kart_three != 0) {
	  our_kart.kartNumber = 3;
		our_kart.position_command_index = KART_THREE_POS_COM_INDEX;
		our_kart.game_status_byte = KART_THREE_MATCH_STATUS_BYTE;
		kart_a.kartNumber = 1;
		kart_a.position_command_index = KART_ONE_POS_COM_INDEX;
		kart_b.kartNumber = 2;	
		kart_b.position_command_index = KART_TWO_POS_COM_INDEX;
		}
	else printf("Kart Hasn't Been Initialized\r\n");
}
	
	//Initialize the Ports B0-2 on the Tiva as a digital input pins
	//to indicate which kart is ours at the start of the race
static void initializePins(void){
	HWREG(SYSCTL_RCGCGPIO) |= (SYSCTL_RCGCGPIO_R1);
	//puts("\r\nPort B activated");
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != SYSCTL_PRGPIO_R1);
	/* set as a digital ports */
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (KART_ONE_BIT|KART_TWO_BIT|KART_THREE_BIT);
  /* set as input ports*/
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) &= ~(KART_ONE_BIT|KART_TWO_BIT|KART_THREE_BIT);
}

static void initGameStatusLED(void){
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != SYSCTL_PRGPIO_R1);
	HWREG(GPIO_PORTB_BASE+ GPIO_O_DEN)|= (BIT3HI);
	HWREG(GPIO_PORTB_BASE+ GPIO_O_DIR)|= (BIT3HI);
	HWREG(GPIO_PORTB_BASE +(GPIO_O_DATA + ALL_BITS))&= ~(BIT3HI);
}

