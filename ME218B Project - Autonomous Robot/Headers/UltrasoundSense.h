#ifndef UltrasoundSense_H
#define UltrasoundSense_H

#include <stdint.h>

void initUltrasoundSense(void);
void ultrasoundSenseResponse(void);
void maskUltrasoundInterrupt(bool maskStatus);
#endif /* UltrasoundSense_H */
