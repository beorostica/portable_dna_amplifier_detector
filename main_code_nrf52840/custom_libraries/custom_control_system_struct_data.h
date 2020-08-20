
#ifndef CUSTOM_CONTROL_SYSTEM_STRUCT_DATA_H
#define CUSTOM_CONTROL_SYSTEM_STRUCT_DATA_H


#include <stdint.h>

typedef struct 
{ 
    uint16_t  time;
    uint16_t  refAdc;
    uint16_t  yAdc;
    uint16_t  uPwm;
    uint16_t  refAdcHotlid;
    uint16_t  yAdcHotlid;
    uint16_t  uPwmHotlid;
    uint16_t  yAdcTamb;
} control_system_data;


void controlSystem_saveStructData(uint16_t time, uint16_t refAdc, uint16_t yAdc, uint16_t uPwm, uint16_t refAdcHotlid, uint16_t yAdcHotlid, uint16_t uPwmHotlid, uint16_t yAdcTamb);

control_system_data controlSystem_getStructData(void);


#endif /* CUSTOM_CONTROL_SYSTEM_STRUCT_DATA_H */