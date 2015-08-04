#ifndef _FollowTape_H_
#define _FollowTape_H_

#include <stdint.h>
#include "ES_Types.h"

#define NUM_TAPE_SENSORS 2


void InitFollowTape( void );
void FollowTapeResponse( void );
void followTapeInterruptMask(bool interruptMaskStatus);
int getTapeSensor1Val(void);
int getTapeSensor2Val(void);
void setNominalTapeFollowDuty(uint16_t nom_A, uint16_t nom_B);

#endif //_IntSample_H_
