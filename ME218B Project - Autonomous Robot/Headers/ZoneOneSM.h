/****************************************************************************
 ZoneOne header file for Hierarchical Sate Machines AKA StateCharts
 02/08/12 adjsutments for use with the Events and Services Framework Gen2
 3/17/09  Fixed prototpyes to use Event_t
 ****************************************************************************/

#ifndef ZoneOneSM_H
#define ZoneOneSM_H

// typedefs for the states
// State definitions for use with the query function
typedef enum { FINDING_TAPE_ZONE_ONE, REALIGN_ZONE_ONE, FROM_ZONE_FIVE,FOLLOW_TAPE_ZONE_ONE, FORWARD_TO_ALIGN_ZONE_ONE } ZoneOneState_t ;
// Public Function Prototypes
ES_Event RunZoneOneSM( ES_Event CurrentEvent );
void StartZoneOneSM ( ES_Event CurrentEvent );
ZoneOneState_t QueryZoneOneSM ( void );

#endif /*ZoneOne_H */

