/**
* SendCommand.c
* This is the HSM for DRS communication that handles the byte sequence.
*/

#include "ES_Configure.h"
#include "ES_Framework.h"

#include "SendCommandSM.h"
#include "UpdateSM.h"
#include "SendByteSM.h"

#define ENTRY_STATE WaitingCommand

/* Prototypes */
static ES_Event DuringWaitingCommand(ES_Event Event);
static ES_Event DuringSendingByteOne(ES_Event Event);
static ES_Event DuringSendingByteTwo(ES_Event Event);
static ES_Event DuringSendingByteThree(ES_Event Event);
static ES_Event DuringSendingByteFour(ES_Event Event);
static ES_Event DuringSendingByteFive(ES_Event Event);
static ES_Event DuringSendingByteSix(ES_Event Event);
static ES_Event DuringSendingByteSeven(ES_Event Event);
static ES_Event DuringSendingByteEight(ES_Event Event);

/* Module-level variables */
static SendCommandState_t CurrentState;
static uint8_t currentCmd;

/**
* Runs the SendCommand state machine.
* @param CurrentEvent: the event to process
* @return an event to return 
*/
ES_Event RunSendCommandSM(ES_Event CurrentEvent) {
	bool MakeTransition = false; // are we making a state transition?
	SendCommandState_t NextState = CurrentState;
	ES_Event EntryEventKind = { ES_ENTRY, 0 }; // default to normal entry to new state
	ES_Event ReturnEvent = { ES_NO_EVENT, 0 }; // assume no error

    switch (CurrentState) {
		case WaitingCommand: {
			//printf("\r\n B");
			CurrentEvent = DuringWaitingCommand(CurrentEvent);
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				if (CurrentEvent.EventType == SendCmd) { // post SendByte with param = param of SendCmd
					currentCmd = CurrentEvent.EventParam; // store the command that needs to be sent
					NextState = SendingByteOne;
					MakeTransition = true;
				}
			}
		}
		break;
		case SendingByteOne: {
			//printf("\r\n C");
			CurrentEvent = DuringSendingByteOne(CurrentEvent);
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				if ((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == DRS_TIMER)) { // post SendByte with param = 0x00
					//printf("Got timeout\r\n"); //**********************************************************************************************************************
					NextState = SendingByteTwo;
					MakeTransition = true;
					ReturnEvent.EventType = ES_NO_EVENT; // consume the ES_TIMEOUT event
				}
			}
		}
		break;
		case SendingByteTwo: {
			//printf("\r\n D");
			CurrentEvent = DuringSendingByteTwo(CurrentEvent);
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				if ((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == DRS_TIMER)) { // post SendByte with param = 0x00
					NextState = SendingByteThree;
					MakeTransition = true;
					ReturnEvent.EventType = ES_NO_EVENT; // consume the ES_TIMEOUT event
				}
			}
		}
		break;
		case SendingByteThree: {
			//printf("\r\n E");
			CurrentEvent = DuringSendingByteThree(CurrentEvent);
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				if ((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == DRS_TIMER)) { // post SendByte with param = 0x00
					NextState = SendingByteFour;
					MakeTransition = true;
					ReturnEvent.EventType = ES_NO_EVENT; // consume the ES_TIMEOUT event
				}
			}
		}
		break;
		case SendingByteFour: {
			//printf("\r\n F");
			CurrentEvent = DuringSendingByteFour(CurrentEvent);
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				if ((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == DRS_TIMER)) { // post SendByte with param = 0x00
					NextState = SendingByteFive;
					MakeTransition = true;
					ReturnEvent.EventType = ES_NO_EVENT; // consume the ES_TIMEOUT event
				}
			}
		}
		break;
		case SendingByteFive: {
			//printf("\r\n G");
			CurrentEvent = DuringSendingByteFive(CurrentEvent);
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				if ((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == DRS_TIMER)) { // post SendByte with param = 0x00
					NextState = SendingByteSix;
					MakeTransition = true;
					ReturnEvent.EventType = ES_NO_EVENT; // consume the ES_TIMEOUT event
				}
			}
		}
		break;
		case SendingByteSix: {
			//printf("\r\n H");
			CurrentEvent = DuringSendingByteSix(CurrentEvent);
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				if ((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == DRS_TIMER)) { // post SendByte with param = 0x00
					NextState = SendingByteSeven;
					MakeTransition = true;
					ReturnEvent.EventType = ES_NO_EVENT; // consume the ES_TIMEOUT event
				}
			}
		}
		break;
		case SendingByteSeven: {
			//printf("\r\n I");
			CurrentEvent = DuringSendingByteSeven(CurrentEvent);
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				if ((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == DRS_TIMER)) { // post SendByte with param = 0x00
					NextState = SendingByteEight;
					MakeTransition = true;
					ReturnEvent.EventType = ES_NO_EVENT; // consume the ES_TIMEOUT event
				}
			}
		}
		break;
		case SendingByteEight: {
			//printf("\r\n J");
			CurrentEvent = DuringSendingByteEight(CurrentEvent);
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				if ((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == DRS_TIMER)) {
					//NextState = WaitingCommand;
					//MakeTransition = true;
					//printf("\r\n A");
					MakeTransition = false;
					ReturnEvent = CurrentEvent;
					CurrentState = WaitingCommand;
				}
			}
		}
		break;
	}

    /* If we are making a state transition */
    if (MakeTransition == true) {
		/* Execute exit function for current state */
		CurrentEvent.EventType = ES_EXIT;
		RunSendCommandSM(CurrentEvent);

		CurrentState = NextState; // modify state variable

       /* Execute entry function for new state */
       /* This defaults to ES_ENTRY */
		RunSendCommandSM(EntryEventKind);
	}
   
	/* In the absence of an error, the top level state machine should
	always return ES_NO_EVENT, which we initialized at the top of function */
	return ReturnEvent;
}

/**
* Does any required initialization for this state machine
* @param CurrentEvent: the current event
*/
void StartSendCommandSM(ES_Event CurrentEvent) {
	if (ES_ENTRY_HISTORY != CurrentEvent.EventType) {
		CurrentState = ENTRY_STATE;
	}
	/* Call the entry function (if any) for the ENTRY_STATE */
	RunSendCommandSM(CurrentEvent);
}

/**
* Returns the current state of the state machine.
* @return the current state of the state machine
*/
SendCommandState_t QuerySendCommandSM(void) {
	return CurrentState;
}

/**
* Initializes and runs lower-level state machine(s) for WaitingCommand
* @param Event: the current event to pass down
* @return the event that has returned from the lower levels
*/
static ES_Event DuringWaitingCommand(ES_Event Event) {
    ES_Event ReturnEvent = Event; // assume no re-mapping or comsumption

    /* process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events */
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {

    } else if (Event.EventType == ES_EXIT) {

    } else { // do the "during" function for this state

    }

    return ReturnEvent;
}

/**
* Initializes and runs lower-level state machine(s) for SendingByteOne
* @param Event: the current event to pass down
* @return the event that has returned from the lower levels
*/
static ES_Event DuringSendingByteOne(ES_Event Event) {
    ES_Event ReturnEvent = Event; // assume no re-mapping or consumption

    /* process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events */
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		StartSendByteSM(Event);

		/* Post the first byte - i.e., the command */
		ES_Event ThisEvent;
		ThisEvent.EventType = SendByte;
		ThisEvent.EventParam = currentCmd;
		PostUpdateSM(ThisEvent);

    } else if (Event.EventType == ES_EXIT) {

    } else { // do the "during" function for this state
    	ReturnEvent = RunSendByteSM(Event); // let the lower levels handle the event first
			//if (ReturnEvent.EventType == ES_TIMEOUT) printf("Came back up\r\n"); //************************************************************
			//else if (ReturnEvent.EventType == ES_NO_EVENT) printf("Didn't come back up\r\n"); //************************************************
    }

    return ReturnEvent;
}

/**
* Initializes and runs lower-level state machine(s) for SendingByteTwo
* @param Event: the current event to pass down
* @return the event that has returned from the lower levels
*/
static ES_Event DuringSendingByteTwo(ES_Event Event) {
    ES_Event ReturnEvent = Event; // assume no re-mapping or consumption

    /* process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events */
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		StartSendByteSM(Event);

		/* Post the second byte - i.e., 0x00 */
		ES_Event ThisEvent;
		ThisEvent.EventType = SendByte;
		ThisEvent.EventParam = 0x00;
		PostUpdateSM(ThisEvent);

    } else if (Event.EventType == ES_EXIT) {

    } else { // do the "during" function for this state
    	ReturnEvent = RunSendByteSM(Event); // let the lower levels handle the event first
    }

    return ReturnEvent;
}

/**
* Initializes and runs lower-level state machine(s) for SendingByteThree
* @param Event: the current event to pass down
* @return the event that has returned from the lower levels
*/
static ES_Event DuringSendingByteThree(ES_Event Event) {
    ES_Event ReturnEvent = Event; // assume no re-mapping or consumption

    /* process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events */
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		StartSendByteSM(Event);

		/* Post the third byte - i.e., 0x00 */
		ES_Event ThisEvent;
		ThisEvent.EventType = SendByte;
		ThisEvent.EventParam = 0x00;
		PostUpdateSM(ThisEvent);

    } else if (Event.EventType == ES_EXIT) {

    } else { // do the "during" function for this state
    	ReturnEvent = RunSendByteSM(Event); // let the lower levels handle the event first
    }

    return ReturnEvent;
}

/**
* Initializes and runs lower-level state machine(s) for SendingByteFour
* @param Event: the current event to pass down
* @return the event that has returned from the lower levels
*/
static ES_Event DuringSendingByteFour(ES_Event Event) {
    ES_Event ReturnEvent = Event; // assume no re-mapping or consumption

    /* process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events */
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		StartSendByteSM(Event);

		/* Post the fourth byte - i.e., 0x00 */
		ES_Event ThisEvent;
		ThisEvent.EventType = SendByte;
		ThisEvent.EventParam = 0x00;
		PostUpdateSM(ThisEvent);

    } else if (Event.EventType == ES_EXIT) {

    } else { // do the "during" function for this state
    	ReturnEvent = RunSendByteSM(Event); // let the lower levels handle the event first
    }

    return ReturnEvent;
}

/**
* Initializes and runs lower-level state machine(s) for SendingByteFive
* @param Event: the current event to pass down
* @return the event that has returned from the lower levels
*/
static ES_Event DuringSendingByteFive(ES_Event Event) {
    ES_Event ReturnEvent = Event; // assume no re-mapping or consumption

    /* process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events */
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		StartSendByteSM(Event);

		/* Post the fifth byte - i.e., 0x00 */
		ES_Event ThisEvent;
		ThisEvent.EventType = SendByte;
		ThisEvent.EventParam = 0x00;
		PostUpdateSM(ThisEvent);

    } else if (Event.EventType == ES_EXIT) {

    } else { // do the "during" function for this state
    	ReturnEvent = RunSendByteSM(Event); // let the lower levels handle the event first
    }

    return ReturnEvent;
}

/**
* Initializes and runs lower-level state machine(s) for SendingByteSix
* @param Event: the current event to pass down
* @return the event that has returned from the lower levels
*/
static ES_Event DuringSendingByteSix(ES_Event Event) {
    ES_Event ReturnEvent = Event; // assume no re-mapping or consumption

    /* process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events */
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		StartSendByteSM(Event);

		/* Post the sixth byte - i.e., 0x00 */
		ES_Event ThisEvent;
		ThisEvent.EventType = SendByte;
		ThisEvent.EventParam = 0x00;
		PostUpdateSM(ThisEvent);

    } else if (Event.EventType == ES_EXIT) {

    } else { // do the "during" function for this state
    	ReturnEvent = RunSendByteSM(Event); // let the lower levels handle the event first
    }

    return ReturnEvent;
}

/**
* Initializes and runs lower-level state machine(s) for SendingByteSeven
* @param Event: the current event to pass down
* @return the event that has returned from the lower levels
*/
static ES_Event DuringSendingByteSeven(ES_Event Event) {
    ES_Event ReturnEvent = Event; // assume no re-mapping or consumption

    /* process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events */
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		StartSendByteSM(Event);

		/* Post the seventh byte - i.e., 0x00 */
		ES_Event ThisEvent;
		ThisEvent.EventType = SendByte;
		ThisEvent.EventParam = 0x00;
		PostUpdateSM(ThisEvent);

    } else if (Event.EventType == ES_EXIT) {

    } else { // do the "during" function for this state
    	ReturnEvent = RunSendByteSM(Event); // let the lower levels handle the event first
    }

    return ReturnEvent;
}

/**
* Initializes and runs lower-level state machine(s) for SendingByteEight
* @param Event: the current event to pass down
* @return the event that has returned from the lower levels
*/
static ES_Event DuringSendingByteEight(ES_Event Event) {
    ES_Event ReturnEvent = Event; // assume no re-mapping or consumption

    /* process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events */
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
		StartSendByteSM(Event);

		/* Post the eighth byte - i.e., 0x00 */
		ES_Event ThisEvent;
		ThisEvent.EventType = SendByte;
		ThisEvent.EventParam = 0x00;
		PostUpdateSM(ThisEvent);

    } else if (Event.EventType == ES_EXIT) {

    } else { // do the "during" function for this state
    	ReturnEvent = RunSendByteSM(Event); // let the lower levels handle the event first
    }

    return ReturnEvent;
}
