/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts

 ****************************************************************************/

#ifndef ComServiceSM_H
#define ComServiceSM_H

#define COURT_Y_MIN 5
#define COURT_Y_MAX 190
#define COURT_X_MIN 70
#define COURT_X_MAX 249

//constants to define Zone 2
/*
#define ZONE_2_X_MIN 70
#define ZONE_2_X_MAX 117-4
//#define ZONE_2_X_MAX 130
*/
#define ZONE_2_X_MIN COURT_X_MIN
#define ZONE_2_X_MAX 120


//#define ZONE_2_Y_MIN 48
#define ZONE_2_Y_MIN 53
//#define ZONE_2_Y_MAX 134
#define ZONE_2_Y_MAX 117

//constants to define Zone 3
#define ZONE_3_X_MIN ZONE_2_X_MAX
#define ZONE_3_X_MAX 162

#define ZONE_3_Y_MIN ZONE_2_Y_MIN
#define ZONE_3_Y_MAX ZONE_2_Y_MAX

//constants to define Zone 4
#define ZONE_4_X_MIN 150
//#define ZONE_4_X_MAX 206
//#define ZONE_4_X_MAX 230
#define ZONE_4_X_MAX 212

//#define ZONE_4_Y_MIN ZONE_3_Y_MAX
#define ZONE_4_Y_MAX COURT_Y_MAX
//#define ZONE_4_Y_MIN 125
#define ZONE_4_Y_MIN 115

//constants to define Zone 5
#define ZONE_5_X_MIN ZONE_3_X_MAX
//#define ZONE_5_X_MAX 206
#define ZONE_5_X_MAX 192

//#define ZONE_5_Y_MIN ZONE_3_Y_MIN
#define ZONE_5_Y_MAX ZONE_4_Y_MIN
//#define ZONE_5_Y_MIN 55
#define ZONE_5_Y_MIN 60 

//#define OBSTACLE_PAST_CROSSOVER_POINT_Y 77 //was 70
#define OBSTACLE_PAST_CROSSOVER_POINT_Y 84
//#define CENTER_OF_OBSTACLE_POINT_X 165
//#define CENTER_OF_OBSTACLE_POINT_X 179
#define CENTER_OF_OBSTACLE_POINT_X 174
#define ZONE_ONE_FROM_ZONE_FIVE_STOP_Y 20

#define OFFSET_TO_CENTER_OF_ROT 2

// State definitions for use with the query function
typedef enum { WAITING, PROCESSING } ComState_t ;

// Public Function Prototypes

ES_Event RunComServiceSM( ES_Event CurrentEvent );
void StartComServiceSM ( ES_Event CurrentEvent );
bool PostComServiceSM( ES_Event ThisEvent );
bool InitComServiceSM ( uint8_t Priority );

void setComeListenToPostFive(bool listen);
void setMiddleOfObstacle(bool listen);
void setListenToCrossOverPoint(bool listen);

void setZonesForAlign(int currentZone, int targetZone,bool reset); //if reset is true set back to default values

uint16_t getOurKartY(void);
uint16_t getOurKartX(void);
uint8_t getKartAZone(void);
uint8_t getKartBZone(void);
uint16_t getOurKartAbsoluteOrientation(void);
uint8_t getOurZone(void);

#endif /*ComServiceSM_H */

