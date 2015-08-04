#ifndef DRIVE_H
#define DRIVE_H


/************************************CONVERSION_CONSTANTS********************************/

//#define EDGES_PER_DEGREE 0.2
#define EDGES_PER_DEGREE 0.284
//#define EDGES_PER_DEGREE 0.18 // hack
#define EDGES_PER_INCH 4.07

/*************************************DISTANCES***************************************/

#define EDGE_DISTANCE_TO_OBSTACLE_CENTER 20 //edges
//#define BACK_UP_FROM_OBSTACLE_DISTANCE 12 //inches
#define BACK_UP_FROM_OBSTACLE_DISTANCE 4//inches
//#define EDGE_DISTANCE_TO_SHOOTING_ZONE_CENTER 73 //edges
#define EDGE_DISTANCE_TO_SHOOTING_ZONE_CENTER 56 //edges center of zone two, this works 64
//#define CENTER_OF_ROT_TO_TAPE_SENSORS 2.5 //inches
#define CENTER_OF_ROT_TO_TAPE_SENSORS 3.5 //inches
#define SHOOTING_DISTANCE_FROM_BOUNDARY_OF_ZONE_THREE 1 //inches

/**************************************SPEEDS*********************************************/
#define TAPE_FINDING_SPEED 25 //rpm
#define REVERSE_BREAKING_SPEED 25 //rpm
#define OUT_OF_ZONE_THREE_SPEED TAPE_FINDING_SPEED //rpm
#define INTO_ZONE_THREE_SPEED 50//rpm
#define CLIMBING_SPEED 150 //rpm
//#define CLIMBING_SPEED 30 //rpm
#define INTO_RAMP_SPEED 150 //rpm
//#define INTO_RAMP_SPEED 60 //rpm
#define DESCENDING_SPEED 0 //rpm
#define DRIVE_DUTY 40 //duty
#define REV_DRIVE_DUTY 30 //duty
#define TURN_DUTY 40 //duty
//#define BACK_UP_FROM_OBSTACLE_SPEED 25 //rpm
#define BACK_UP_FROM_OBSTACLE_SPEED 20 //rpm
#define NOMINAL_FOLLOW_TAPE_DUTY 45//duty
//#define NOMINAL_FOLLOW_TAPE_DUTY 100//duty
#define FOLLOW_TAPE_CLIMBING_DUTY 60 //duty
#define FOLLOW_TAPE_DESCENDING_DUTY 25 //duty
#define BEACON_TURNING_DUTY 40

#include <stdint.h>

void AlignWithBeacon(void);
void StopMotor(void);
void TurnKart(float degrees, bool alignWithBeacon);
void FollowTape(bool follow, uint16_t motorADuty, uint16_t motorBDuty);
void InitDrive(void);
void DriveDistance(float numInches,bool reverse);
void DriveForward(uint8_t duty);
//void enableOrientationControl(bool control, uint8_t duty, bool trackX, uint16_t coordinate);
void countEdges(uint32_t edgeCount);
void DriveReverse(uint8_t duty);
void enableSpeedControl(bool control, uint8_t RPM_Speed, bool reverse);

#endif
