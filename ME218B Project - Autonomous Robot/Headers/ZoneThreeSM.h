/****************************************************************************
 ZoneThree header file for Hierarchical Sate Machines AKA StateCharts
 02/08/12 adjsutments for use with the Events and Services Framework Gen2
 3/17/09  Fixed prototpyes to use Event_t
 ****************************************************************************/

#ifndef ZoneThreeSM_H
#define ZoneThreeSM_H

// typedefs for the states
// State definitions for use with the query function
typedef enum { ALIGN_WITH_BEACON, WAIT_FOR_SHOOT_MOTOR, SHOOTING, TURN_TOWARDS_ZONE_TWO, DRIVE_TO_SHOOTING_DISTANCE } ZoneThreeState_t ;
// Public Function Prototypes
ES_Event RunZoneThreeSM( ES_Event CurrentEvent );
void StartZoneThreeSM ( ES_Event CurrentEvent );
ZoneThreeState_t QueryZoneThreeSM ( void );

#endif /*ZoneThree_H */

