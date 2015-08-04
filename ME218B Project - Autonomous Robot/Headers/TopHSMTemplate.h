/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts

 ****************************************************************************/

#ifndef TopHSMTemplate_H
#define TopHSMTemplate_H

// State definitions for use with the query function
typedef enum { WAITING, PROCESSING } ComState_t ;

// Public Function Prototypes

ES_Event RunMasterSM( ES_Event CurrentEvent );
void StartMasterSM ( ES_Event CurrentEvent );
bool PostMasterSM( ES_Event ThisEvent );
bool InitMasterSM ( uint8_t Priority );

#endif /*TopHSMTemplate_H */

