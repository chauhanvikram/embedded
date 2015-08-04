#ifndef BeaconSense_H
#define BeaconSense_H

#include <stdint.h>

void initBeaconSense(void);
void BeaconSenseResponse(void);
void maskBeaconInterrupt(bool maskStatus);
#endif /* BeaconSense_H */
