/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts

 ****************************************************************************/

#ifndef StrategySM_H
#define StrategySM_H

// State definitions for use with the query function
typedef enum { WAITING_STATE, ZONE_ONE, ZONE_TWO, ZONE_THREE, ZONE_FOUR, ZONE_FIVE } StrategyState_t ;

// Public Function Prototypes

ES_Event RunStrategySM( ES_Event CurrentEvent );
void StartStrategySM ( ES_Event CurrentEvent );
bool PostStrategySM( ES_Event ThisEvent );
bool InitStrategySM ( uint8_t Priority );

bool TargetCompletionStatus(void);
bool ObstacleCompletionStatus(void);
int getLapsRemaining(void);

#endif /*StrategySM */

