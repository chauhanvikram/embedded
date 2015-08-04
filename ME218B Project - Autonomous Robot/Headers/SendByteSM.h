/**
* This is the header file for SendByteSM.c.
*/

#ifndef SendByteSM_H
#define SendByteSM_H


#define GAME_STATUS_COM_INDEX 0
#define KART_ONE_MATCH_STATUS_BYTE 3
#define KART_TWO_MATCH_STATUS_BYTE 4
#define KART_THREE_MATCH_STATUS_BYTE 5
#define NUM_LAPS_LEFT_MASK 0x07
#define NUM_LAPS_LEFT_SHIFT 0 
#define RUN_STATUS_MASK 0x18
#define RUN_STATUS_SHIFT 3
#define TARGET_STATUS_MASK 0x80
#define TARGET_STATUS_SHIFT 7
#define OBSTACLE_STATUS_STATUS_MASK 0x40//01000000
#define OBSTACLE_STATUS_STATUS_SHIFT 6

#define KART_ONE_POS_COM_INDEX 1
#define KART_TWO_POS_COM_INDEX 2
#define KART_THREE_POS_COM_INDEX 3
//#define POS_X_M_INDEX 1
//#define POS_X_L_INDEX 2
//#define POS_Y_M_INDEX 3
//#define POS_Y_L_INDEX 4
//#define ORIENT_M_INDEX 5
//#define ORIENT_L_INDEX 6
#define POS_X_M_INDEX 2
#define POS_X_L_INDEX 3
#define POS_Y_M_INDEX 4
#define POS_Y_L_INDEX 5
#define ORIENT_M_INDEX 6
#define ORIENT_L_INDEX 7



/* State definitions for use with the query function */
typedef enum { WaitingByte, WaitingForEOT, WaitingForTimeout } SendByteState_t;

/* Public Function Prototypes */
ES_Event RunSendByteSM(ES_Event CurrentEvent);
void StartSendByteSM(ES_Event CurrentEvent);
SendByteState_t QuerySendByteSM(void);
uint8_t getDataByte(uint8_t com, uint8_t byte);

#endif /* SendByteSM_H */

