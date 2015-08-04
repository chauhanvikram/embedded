/**
* This is the header file for UpdateSM.c.
*/

#ifndef UpdateSM_H
#define UpdateSM_H

/* State definitions for use with the query function */
typedef enum { Waiting, SendingCmdOne, 
				SendingCmdTwo, SendingCmdThree,
				SendingCmdFour } UpdateState_t;

/* Public Function Prototypes */
ES_Event RunUpdateSM(ES_Event CurrentEvent);
void StartUpdateSM(ES_Event CurrentEvent);
bool PostUpdateSM(ES_Event ThisEvent);
bool InitUpdateSM(uint8_t Priority);

#endif /* UpdateSM_H */
