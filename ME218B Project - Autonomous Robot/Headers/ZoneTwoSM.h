/****************************************************************************
 ZoneTwo header file for Hierarchical Sate Machines AKA StateCharts
 02/08/12 adjsutments for use with the Events and Services Framework Gen2
 3/17/09  Fixed prototpyes to use Event_t
 ****************************************************************************/

#ifndef ZoneTwoSM_H
#define ZoneTwoSM_H

// typedefs for the states
// State definitions for use with the query function
typedef enum { FOLLOW_TAPE_ZONE_TWO, ENTER_SHOOT, REALIGN_ZONE_TWO, FINDING_TAPE_ZONE_TWO, FORWARD_TO_ALIGN_ZONE_TWO } ZoneTwoState_t ;
// Public Function Prototypes
ES_Event RunZoneTwoSM( ES_Event CurrentEvent );
void StartZoneTwoSM ( ES_Event CurrentEvent );
ZoneTwoState_t QueryZoneTwoSM ( void );

#endif /*ZoneTwo_H */

