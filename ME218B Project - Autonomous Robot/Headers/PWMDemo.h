#ifndef PWMDEMO_H
#define PWMDEMO_H

#include <stdint.h>

void InitPWMDrive(void);
void InitPWMShooter(void);
void InitPWMGates(void);
//void SetPWMDuty( int NewDutyA, int NewDutyB );
void SetPWMDutyDrive( float NewDutyA, float NewDutyB );
void SetPWMDutyShooter( float NewDuty);

void SetPWMPulseWidthBackGate(uint32_t NewWidth );
void SetPWMPulseWidthFrontGate(uint32_t NewWidth );

void openGateFront(void);
void closeGateFront(void);
void openGateBack(void);
void closeGateBack(void);
#endif
