/****************************************************************************
 ZoneFive header file for Hierarchical Sate Machines AKA StateCharts
 02/08/12 adjsutments for use with the Events and Services Framework Gen2
 3/17/09  Fixed prototpyes to use Event_t
 ****************************************************************************/

#ifndef ZoneFiveSM_H
#define ZoneFiveSM_H

// typedefs for the states
// State definitions for use with the query function
typedef enum {CLIMBING_OBSTACLE, DESCENDING_OBSTACLE} ZoneFiveState_t ;
// Public Function Prototypes
ES_Event RunZoneFiveSM( ES_Event CurrentEvent );
void StartZoneFiveSM ( ES_Event CurrentEvent );
ZoneFiveState_t QueryZoneFiveSM ( void );

#endif /*ZoneFive_H */

