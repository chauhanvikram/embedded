#ifndef EncoderRight_H
#define EncoderRight_H

#include <stdint.h>

#define ENCODER_PPR 64 // pulses per revolution for the encoder

void initEncoderRight(void);
void encoderRightResponse(void);
uint32_t getPeriodRight(void);
void maskRightEncoder(bool masked);
void startPosControlRight(uint32_t edgeCount);
uint32_t getEdgeCountRight(void);
#endif /* EncoderRight_H */
