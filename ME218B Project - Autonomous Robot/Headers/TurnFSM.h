/****************************************************************************
 
  Header file for Turning Flat Sate Machine 
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef FSMTurning_H
#define FSMTurning_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum { SAMPLING, TURNING,WAITING_TURNING_FSM} TurningState_t ;


// Public Function Prototypes

bool InitTurningFSM ( uint8_t Priority );
bool PostTurningFSM( ES_Event ThisEvent );
ES_Event RunTurningFSM( ES_Event ThisEvent );
TurningState_t QueryTurningSM ( void );
void setTargetOrientation(uint16_t orientation);

#endif /* FSMTurning_H */

