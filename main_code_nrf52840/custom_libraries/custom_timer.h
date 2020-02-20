
#ifndef CUSTOM_TIMER_H
#define CUSTOM_TIMER_H


#include <stdint.h>
#include <stdbool.h>

/////////////////////////////////////////////////////
/// To create All Timers ////////////////////////////
/////////////////////////////////////////////////////

void timerInit(void);


/////////////////////////////////////////////////////
/// To Detect If a tenth of a Second has past ///////
/// Also, to get the time value               ///////
/////////////////////////////////////////////////////

void hundredMillisStart(void);

bool hundredMillisGetFlag(void);

void hundredMillisClearFlag(void);

uint32_t hundredMillisGetTime(void);

void hundredMillisStop(void);


///////////////////////////////////////////////
/// To Detect If a Second has past ////////////
/// Also, to get the time value    ////////////
///////////////////////////////////////////////

void secondsStart(void);

bool secondsGetFlag(void);

void secondsClearFlag(void);

uint32_t secondsGetTime(void);

void secondsStop(void);


////////////////////////////////////////////////////////////////
/// To check when the Detector System Must be Executeed ////////
////////////////////////////////////////////////////////////////

void timerDetectionSystem_Start(void);

bool timerDetectionSystem_GetFlag(void);

void timerDetectionSystem_ClearFlag(void);

void timerDetectionSystem_Stop(void);


#endif /* CUSTOM_TIMER_H */