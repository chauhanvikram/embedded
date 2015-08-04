/****************************************************************************
 ZoneFour header file for Hierarchical Sate Machines AKA StateCharts
 02/08/12 adjsutments for use with the Events and Services Framework Gen2
 3/17/09  Fixed prototpyes to use Event_t
 ****************************************************************************/

#ifndef ZoneFourSM_H
#define ZoneFourSM_H

// typedefs for the states
// State definitions for use with the query function
typedef enum { FOLLOW_TAPE_ZONE_FOUR, BACK_AWAY_FROM_OBSTACLE , ALIGN_WITH_OBSTACLE, ALIGN_WITH_OBSTACLE_ROUND_TWO } ZoneFourState_t ;
// Public Function Prototypes
ES_Event RunZoneFourSM( ES_Event CurrentEvent );
void StartZoneFourSM ( ES_Event CurrentEvent );
ZoneFourState_t QueryZoneFourSM ( void );

#endif /*ZoneFour_H */

