#ifndef EncoderLeft_H
#define EncoderLeft_H

#include <stdint.h>

#define ENCODER_PPR 64 // pulses per revolution for the encoder

void initEncoderLeft(void);
void encoderLeftResponse(void);
uint32_t getPeriodLeft(void);
void maskLeftEncoder(bool masked);
void startPosControlLeft(uint32_t edgeCount);
uint32_t getEdgeCountLeft(void);

#endif /* EncoderLeft_H */
