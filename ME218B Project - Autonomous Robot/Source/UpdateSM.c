/**
* UpdateSM.c
* This is the top-level HSM for DRS communication.
*/

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "UpdateSM.h"
#include "SendCommandSM.h"
#include "SPI.h"

#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "bitdefs.h"

#define CMD_1 0x3F // game status
#define CMD_2 0xC3 // KART 1 information
#define CMD_3 0x5A // KART 2 information
#define CMD_4 0x7E // KART 3 information

/* Prototypes */
static ES_Event DuringWaiting(ES_Event Event);
static ES_Event DuringSendingCmdOne(ES_Event Event);
static ES_Event DuringSendingCmdTwo(ES_Event Event);
static ES_Event DuringSendingCmdThree(ES_Event Event);
static ES_Event DuringSendingCmdFour(ES_Event Event);

/* Module-level variables */
static UpdateState_t CurrentState;
static uint8_t MyPriority;

/**
* Saves away the priority, and starts the top level state machine.
* @param Priority: the priorty of this service
* @return flase if error in initialization; true otherwise
*/
bool InitUpdateSM(uint8_t Priority) {
	ES_Event ThisEvent;
	MyPriority = Priority;  // save our priority
	ThisEvent.EventType = ES_ENTRY;
	initSPI(); // initialize SPI hardware
	
	/* Start the Master State machine */
	StartUpdateSM(ThisEvent);

	return true;
}

/**
* Posts an event to this state machine's queue
* @param ThisEvent: the event to post to the queue
* @return false if post operation failed; true otherwise
*/
bool PostUpdateSM(ES_Event ThisEvent) {
	return ES_PostToService(MyPriority, ThisEvent);
}

/**
* Runs the top-level state machine.
* @param CurrentEvent: the event to process
* @return an event to return 
*/
ES_Event RunUpdateSM(ES_Event CurrentEvent) {
	bool MakeTransition = false; // are we making a state transition?
	UpdateState_t NextState = CurrentState;
	ES_Event EntryEventKind = { ES_ENTRY, 0 }; // default to normal entry to new state
	ES_Event ReturnEvent = { ES_NO_EVENT, 0 }; // assume no error
	
	//printf("%i\r\n", CurrentState);

    switch (CurrentState) {
		case Waiting: {
			CurrentEvent = DuringWaiting(CurrentEvent);
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				if (CurrentEvent.EventType == BotUpdate) { // post SendCmd with param = 0x3F
					NextState = SendingCmdOne;
					//printf("\r\n 1");
					MakeTransition = true;
				} 
			}
		}
		break;
		case SendingCmdOne: {
			CurrentEvent = DuringSendingCmdOne(CurrentEvent);
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				if ((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == DRS_TIMER)) { // post SendCmd with param = 0xC3
					//printf("Got timeout");
					NextState = SendingCmdTwo;
					//printf("\r\n 4");
					MakeTransition = true;
				}
			}
		}
		break;
		case SendingCmdTwo: {
			CurrentEvent = DuringSendingCmdTwo(CurrentEvent);
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				if ((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == DRS_TIMER)) { // post SendCmd with param = 0x5A
					NextState = SendingCmdThree;
					//printf("\r\n 5");
					MakeTransition = true;
				}
			}
		}
		break;
		case SendingCmdThree: {
			CurrentEvent = DuringSendingCmdThree(CurrentEvent);
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				if ((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == DRS_TIMER)) { // post SendCmd with param = 0x7E
					NextState = SendingCmdFour;
					//printf("\r\n 6");
					MakeTransition = true;
				}
			}
		}
		break;
		case SendingCmdFour: {
			CurrentEvent = DuringSendingCmdFour(CurrentEvent);
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				if ((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == DRS_TIMER)) {
					NextState = Waiting;
					//printf("\r\n 7");
					MakeTransition = true;
				}
			}
		}
		break;
	}

	/* If we are making a state transition */
	if (MakeTransition == true) {
		/* Execute exit function for current state */
		CurrentEvent.EventType = ES_EXIT;
		//printf("\r\n 2");
		RunUpdateSM(CurrentEvent);
		//printf("\r\n 3");
		CurrentState = NextState; // modify state variable
		//printf("\r\n a");
		/* Execute entry function for new state */
		/* This defaults to ES_ENTRY */
		RunUpdateSM(EntryEventKind);
	}
   
	/* In the absence of an error, the top level state machine should
	always return ES_NO_EVENT, which we initialized at the top of function */
	return ReturnEvent;
}

/**
* Does any required initialization for this state machine
* @param CurrentEvent: the current event
*/
void StartUpdateSM(ES_Event CurrentEvent) {
  // if there is more than 1 state to the top level machine you will need 
  // to initialize the state variable
	CurrentState = Waiting;
  // now we need to let the Run function init the lower level state machines
  // use LocalEvent to keep the compiler from complaining about unused var
	RunUpdateSM(CurrentEvent);
	return;
}

/**
* Initializes and runs lower-level state machine(s) for Waiting
* @param Event: the current event to pass down
* @return the event that has returned from the lower levels
*/
static ES_Event DuringWaiting(ES_Event Event) {
    ES_Event ReturnEvent = Event; // assume no re-mapping or comsumption

    /* process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events */
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {

    } else if (Event.EventType == ES_EXIT) {

    } else { // do the 'during' function for this state

    }

    return ReturnEvent;
}

/**
* Initializes and runs lower-level state machine(s) for SendingCmdOne
* @param Event: the current event to pass down
* @return the event that has returned from the lower levels
*/
static ES_Event DuringSendingCmdOne(ES_Event Event) {
    ES_Event ReturnEvent = Event; // assume no re-mapping or comsumption

    /* process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events */
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		//printf("\r\n b");
		StartSendCommandSM(Event);
		//printf("\r\n c");
		FSS_PORT &= ~FSS_PIN; // Lower the Fss line
		//printf("\r\n d");
		/* Post the first command */
		ES_Event ThisEvent;
		ThisEvent.EventType = SendCmd;
		ThisEvent.EventParam = CMD_1;
		//printf("\r\n e");
		PostUpdateSM(ThisEvent);
		//printf("\r\n f");
    } else if (Event.EventType == ES_EXIT) {
			FSS_PORT |= FSS_PIN; // Raise the Fss line
    } else { // do the "during" function for this state
    	ReturnEvent = RunSendCommandSM(Event); // let the lower levels handle the event first
    }

    return ReturnEvent;
}

/**
* Initializes and runs lower-level state machine(s) for SendingCmdTwo
* @param Event: the current event to pass down
* @return the event that has returned from the lower levels
*/
static ES_Event DuringSendingCmdTwo(ES_Event Event) {
    ES_Event ReturnEvent = Event; // assume no re-mapping or comsumption

    /* process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events */
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		StartSendCommandSM(Event);
		FSS_PORT &= ~FSS_PIN; // Lower the Fss line

		/* Post the second command */
		ES_Event ThisEvent;
		ThisEvent.EventType = SendCmd;
		ThisEvent.EventParam = CMD_2;
		PostUpdateSM(ThisEvent);

    } else if (Event.EventType == ES_EXIT) {
			FSS_PORT |= FSS_PIN; // Raise the Fss line
    } else { // do the "during" function for this state
    	ReturnEvent = RunSendCommandSM(Event); // let the lower levels handle the event first
    }

    return ReturnEvent;
}

/**
* Initializes and runs lower-level state machine(s) for SendingCmdThree
* @param Event: the current event to pass down
* @return the event that has returned from the lower levels
*/
static ES_Event DuringSendingCmdThree(ES_Event Event) {
    ES_Event ReturnEvent = Event; // assume no re-mapping or comsumption

    /* process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events */
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		StartSendCommandSM(Event);
		FSS_PORT &= ~FSS_PIN; // Lower the Fss line
		
		/* Post the third command */
		ES_Event ThisEvent;
		ThisEvent.EventType = SendCmd;
		ThisEvent.EventParam = CMD_3;
		PostUpdateSM(ThisEvent);

    } else if (Event.EventType == ES_EXIT) {
			FSS_PORT |= FSS_PIN; // Raise the Fss line
    } else { // do the "during" function for this state
    	ReturnEvent = RunSendCommandSM(Event); // let the lower levels handle the event first
    }

    return ReturnEvent;
}

/**
* Initializes and runs lower-level state machine(s) for SendingCmdFour
* @param Event: the current event to pass down
* @return the event that has returned from the lower levels
*/
static ES_Event DuringSendingCmdFour(ES_Event Event) {
    ES_Event ReturnEvent = Event; // assume no re-mapping or comsumption

    /* process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events */
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		StartSendCommandSM(Event);
		FSS_PORT &= ~FSS_PIN; // Lower the Fss line

		/* Post the fourth command */
		ES_Event ThisEvent;
		ThisEvent.EventType = SendCmd;
		ThisEvent.EventParam = CMD_4;
		PostUpdateSM(ThisEvent);

    } else if (Event.EventType == ES_EXIT) {
			FSS_PORT |= FSS_PIN; // Raise the Fss line
    } else { // do the "during" function for this state
    	ReturnEvent = RunSendCommandSM(Event); // let the lower levels handle the event first
    }

    return ReturnEvent;
}
