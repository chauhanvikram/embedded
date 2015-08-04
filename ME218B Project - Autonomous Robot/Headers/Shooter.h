#ifndef SHOOTER_H
#define SHOOTER_H

#include <stdint.h>


#define MOTOR_WAIT_TIME 7000
#define SHOOT_TIME 5000
#define BACK_GATE_WAIT_TIME 1500

void InitShooter(void);
void StopFlyWheel(void);
void StartFlyWheel(void);

//if direction is > 0 forward, if < 0 reverse
void ActivateFrontGate(int direction);
void ActivateBackGate(int direction);

#endif
