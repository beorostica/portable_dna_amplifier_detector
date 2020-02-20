
#include "custom_detection_system_struct_data.h"

static detection_system_data dsData;

void detectionSystem_saveStructData_before(uint8_t index, uint8_t mosfetValue, uint16_t sensorValue)
{
    //Save the state of the output and read the sensor before the detection:
    dsData.mosfetActuator_before[index] = mosfetValue;
    dsData.lightSensor_before[index]    = sensorValue;
}

void detectionSystem_saveStructData_after(uint8_t index, uint8_t mosfetValue, uint16_t sensorValue, uint16_t timeValue)
{
    //Save the time and Save the state of the output and read the sensor after the detection:
    dsData.time[index]                 = timeValue;
    dsData.mosfetActuator_after[index] = mosfetValue;
    dsData.lightSensor_after[index]    = sensorValue;
}

detection_system_data detectionSystem_getStructData(void)
{
    return dsData;
}

static detection_system_single_data dsSingleData;

detection_system_single_data detectionSystem_getStructSingleData(detection_system_data data, uint8_t index)
{
    
    dsSingleData.index                 = (uint16_t) index;
    dsSingleData.time                  = data.time[index];
    dsSingleData.mosfetActuator_before = (uint16_t) data.mosfetActuator_before[index];
    dsSingleData.mosfetActuator_after  = (uint16_t) data.mosfetActuator_after[index];
    dsSingleData.lightSensor_before    = data.lightSensor_before[index];
    dsSingleData.lightSensor_after     = data.lightSensor_after[index];
    
    return dsSingleData;
}