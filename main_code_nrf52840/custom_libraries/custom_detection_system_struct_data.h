
#ifndef CUSTOM_DETECTION_SYSTEM_STRUCT_DATA_H
#define CUSTOM_DETECTION_SYSTEM_STRUCT_DATA_H


#include <stdint.h>

typedef struct 
{
    uint16_t  time[4];
    uint8_t   mosfetActuator_before[4];
    uint16_t  lightSensor_before[4];
    uint8_t   mosfetActuator_after[4];
    uint16_t  lightSensor_after[4];
} detection_system_data;

typedef struct 
{
    uint16_t  index;  
    uint16_t  time; 
    uint16_t  mosfetActuator_before;
    uint16_t  mosfetActuator_after;
    uint16_t  lightSensor_before;
    uint16_t  lightSensor_after;
} detection_system_single_data;


void detectionSystem_saveStructData_before(uint8_t index, uint8_t mosfetValue, uint16_t sensorValue);

void detectionSystem_saveStructData_after(uint8_t index, uint8_t mosfetValue, uint16_t sensorValue, uint16_t timeValue);

detection_system_data detectionSystem_getStructData(void);

detection_system_single_data detectionSystem_getStructSingleData(detection_system_data data, uint8_t index);


#endif /* CUSTOM_DETECTION_SYSTEM_STRUCT_DATA_H */