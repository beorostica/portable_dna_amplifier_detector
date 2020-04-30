
#ifndef CUSTOM_BATTERY_SYSTEM_STRUCT_DATA_H
#define CUSTOM_BATTERY_SYSTEM_STRUCT_DATA_H


#include <stdint.h>

typedef struct 
{ 
    uint16_t  time;
    uint16_t  soc;
    uint16_t  capacityRemain;
    uint16_t  capacityFull;
    uint16_t  soh;
    uint16_t  voltage;
    uint16_t  current;
    uint16_t  power;
} battery_system_data;


void batterySystem_saveStructData(uint16_t time, uint16_t soc, uint16_t capacityRemain, uint16_t capacityFull, uint16_t soh, uint16_t voltage, uint16_t current, uint16_t  power);

battery_system_data batterySystem_getStructData(void);


#endif /* CUSTOM_BATTERY_SYSTEM_STRUCT_DATA_H */