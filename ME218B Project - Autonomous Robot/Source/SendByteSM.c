/**
* SendByte.c
* This is the HSM for DRS communication that handles writing and reading each byte.
*/

#include "ES_Configure.h"
#include "ES_Framework.h"

#include "SendByteSM.h"
#include "SPI.h"

#define ENTRY_STATE WaitingByte
#define WAIT_TIME 2 // time to wait between successive transmissions [ms]
#define NUM_COMMANDS 4 // number of commands
#define NUM_BYTES 8 // number of bytes per command

/* Prototypes */
static ES_Event DuringWaitingByte(ES_Event Event);
static ES_Event DuringWaitingForEOT(ES_Event Event);
static ES_Event DuringWaitingForTimeout(ES_Event Event);
static void storeData(void);

/* Module-level variables */
static SendByteState_t CurrentState;
static uint8_t receiveData[NUM_COMMANDS][NUM_BYTES];
static uint8_t comIndex = 0; // command index
static uint8_t byteIndex = 0; // byte index

/**
* Runs the SendByte state machine.
* @param CurrentEvent: the event to process
* @return an event to return 
*/
ES_Event RunSendByteSM(ES_Event CurrentEvent) {
	bool MakeTransition = false; // are we making a state transition?
	SendByteState_t NextState = CurrentState;
	ES_Event EntryEventKind = { ES_ENTRY, 0 }; // default to normal entry to new state
	ES_Event ReturnEvent = { ES_NO_EVENT, 0 }; // assume no error

    switch (CurrentState) {
		case WaitingByte: {
			CurrentEvent = DuringWaitingByte(CurrentEvent);
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				if (CurrentEvent.EventType == SendByte) {
					sendData(CurrentEvent.EventParam); // write the byte to the SSI data register
					//printf("%X\r\n",CurrentEvent.EventParam); // DEBUG *****************************************************
					NextState = WaitingForEOT;
					MakeTransition = true;
				}
			}
		}
		break;
		case WaitingForEOT: {
			//printf("\r\n XX");
			CurrentEvent = DuringWaitingForEOT(CurrentEvent);
			//printf("\r\n 1");
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				if (CurrentEvent.EventType == EndOfTransmit) {
					storeData(); // store received data
					NextState = WaitingForTimeout;
					MakeTransition = true;
					ES_Timer_InitTimer(DRS_TIMER, WAIT_TIME); // start DRS timer
				}
			}
		}
		break;
		case WaitingForTimeout: {
			//printf("\r\n XXX");
			CurrentEvent = DuringWaitingForTimeout(CurrentEvent);
			if (CurrentEvent.EventType != ES_NO_EVENT) {
				if ((CurrentEvent.EventType == ES_TIMEOUT) && (CurrentEvent.EventParam == DRS_TIMER)) {
					//printf("Got timeout\r\n"); //******************************************************************************************************************
					//NextState = WaitingByte;
					//MakeTransition = true;
					MakeTransition = false;
					ReturnEvent = CurrentEvent;
					CurrentState = WaitingByte;
				}
			}
		}
		break;
	}

    /* If we are making a state transition */
    if (MakeTransition == true) {
		/* Execute exit function for current state */
		CurrentEvent.EventType = ES_EXIT;
		RunSendByteSM(CurrentEvent);

		CurrentState = NextState; // modify state variable

       /* Execute entry function for new state */
       /* This defaults to ES_ENTRY */
		RunSendByteSM(EntryEventKind);
	}
   
	return ReturnEvent;
}

/**
* Does any required initialization for this state machine
* @param CurrentEvent: the current event
*/
void StartSendByteSM(ES_Event CurrentEvent) {
	if (ES_ENTRY_HISTORY != CurrentEvent.EventType) {
		CurrentState = ENTRY_STATE;
	}
	/* Call the entry function (if any) for the ENTRY_STATE */
	RunSendByteSM(CurrentEvent);
}

/**
* Returns the current state of the state machine.
* @return the current state of the state machine
*/
SendByteState_t QuerySendByteSM(void) {
	return CurrentState;
}

/**
* Returns byte in receiveData[][].
* @param com command (row) in receiveData
* @param byte byte (column) in receiveData
* @return byte from receiveData[][]
*/
uint8_t getDataByte(uint8_t com, uint8_t byte) {
	return receiveData[com][byte];
}

/**
* Stores and posts data received from the DRS.
*/
static void storeData(void) {
	receiveData[comIndex][byteIndex] = getReceiveData(); // store data from slave
	//printf("%X\r\n", getReceiveData()); //*************************************************************************************

	byteIndex++; // increment byteIndex
	if (byteIndex == 8) {
		byteIndex = 0; // reset byteIndex to 0
		comIndex = (comIndex + 1) % 4; // increment comIndex (modulo 4)
	}
}

/**
* Initializes and runs lower-level state machine(s) for Waiting
* @param Event: the current event to pass down
* @return the event that has returned from the lower levels
*/
static ES_Event DuringWaitingByte(ES_Event Event) {
    ES_Event ReturnEvent = Event; // assume no re-mapping or comsumption

    /* process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events */
		if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {
			
    } else if (Event.EventType == ES_EXIT) {

    } else { // do the "during" function for this state

    }

    return ReturnEvent;
}

/**
* Initializes and runs lower-level state machine(s) for WaitingForEOT
* @param Event: the current event to pass down
* @return the event that has returned from the lower levels
*/
static ES_Event DuringWaitingForEOT(ES_Event Event) {
		//printf("\r\n 1");
    ES_Event ReturnEvent = Event; // assume no re-mapping or comsumption

    /* process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events */
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {

    } else if (Event.EventType == ES_EXIT) {

    } else { // do the "during" function for this state

    }

    return ReturnEvent;
}

/**
* Initializes and runs lower-level state machine(s) for WaitingForTimeout
* @param Event: the current event to pass down
* @return the event that has returned from the lower levels
*/
static ES_Event DuringWaitingForTimeout(ES_Event Event) {
    ES_Event ReturnEvent = Event; // assume no re-mapping or comsumption

    /* process ES_ENTRY, ES_ENTRY_HISTORY & ES_EXIT events */
	if ((Event.EventType == ES_ENTRY) || (Event.EventType == ES_ENTRY_HISTORY)) {

    } else if (Event.EventType == ES_EXIT) {

    } else { // do the "during" function for this state

    }

    return ReturnEvent;
}
