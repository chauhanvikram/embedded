#ifndef SpeedControl_H
#define SpeedControl_H

#include <stdint.h>

void initSpeedControl(void);
void speedControlResponse(void);
void maskSpeedControlInterrupt(bool masked);
//if true use x, if false use y
//void setTargetCoordinate(bool trackX, uint16_t coordinate);
//void setNominalMotorDuty(uint8_t nomMotorA, uint8_t nomMotorB);
void setSpeedSetpoint(uint8_t speed);
void setControlDirection(bool reverse);

#endif /* SpeedControl_H */
