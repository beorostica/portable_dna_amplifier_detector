
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


////////////////////////////////////////////////////////////////
/// For periodicity of the Temp Controller System //////////////
////////////////////////////////////////////////////////////////

void timerControlSystem_Start(void);

void timerControlSystem_Stop(void);

bool timerControlSystem_GetFlag(void);

void timerControlSystem_ClearFlag(void);

////////////////////////////////////////////////////////////////////
/// For periodicity of the Control System Save in external flash ///
////////////////////////////////////////////////////////////////////

void timerControlSystem_SaveExternalFlash_Start(void);

void timerControlSystem_SaveExternalFlash_Stop(void);

bool timerControlSystem_SaveExternalFlash_GetFlag(void);

void timerControlSystem_SaveExternalFlash_ClearFlag(void);


////////////////////////////////////////////////////////
/// For periodicity of the Battery System //////////////
////////////////////////////////////////////////////////

void timerBatterySystem_Start(void);

void timerBatterySystem_Stop(void);

bool timerBatterySystem_GetFlag(void);

void timerBatterySystem_ClearFlag(void);


#endif /* CUSTOM_TIMER_H */