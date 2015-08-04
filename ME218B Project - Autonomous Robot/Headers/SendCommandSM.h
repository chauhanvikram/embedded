/**
* This is the header file for SendCommandSM.c.
*/

#ifndef SendCommandSM_H
#define SendCommandSM_H

/* State definitions for use with the query function */
typedef enum { WaitingCommand, SendingByteOne, SendingByteTwo,
							SendingByteThree, SendingByteFour, SendingByteFive,
							SendingByteSix, SendingByteSeven, SendingByteEight } SendCommandState_t;

/* Public Function Prototypes */
ES_Event RunSendCommandSM(ES_Event CurrentEvent);
void StartSendCommandSM(ES_Event CurrentEvent);
SendCommandState_t QuerySendCommandSM(void);

#endif /* SendCommandSM_H */

