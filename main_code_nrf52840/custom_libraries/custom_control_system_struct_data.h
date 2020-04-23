
#ifndef CUSTOM_CONTROL_SYSTEM_STRUCT_DATA_H
#define CUSTOM_CONTROL_SYSTEM_STRUCT_DATA_H


#include <stdint.h>

typedef struct 
{ 
    uint16_t  time;
    uint16_t  refAdc;
    uint16_t  yAdc;
    uint16_t  uPwm;
} control_system_data;


void controlSystem_saveStructData(uint16_t time, uint16_t refAdc, uint16_t yAdc, uint16_t uPwm);

control_system_data controlSystem_getStructData(void);


#endif /* CUSTOM_CONTROL_SYSTEM_STRUCT_DATA_H */