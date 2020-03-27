
#ifndef CUSTOM_STATUS_STRUCT_DATA_H
#define CUSTOM_STATUS_STRUCT_DATA_H

#include <stdbool.h>
#include <stdint.h>

typedef struct 
{
    bool      commandFromPhone;  
    bool      isMeasuring; 
    bool      isDataOnFlash;
    uint8_t   fileName_year;
    uint8_t   fileName_month;
    uint8_t   fileName_day;
    uint8_t   fileName_hrs;
    uint8_t   fileName_mins;
    uint8_t   fileName_secs;
} device_status_data;


void deviceStatus_saveStructData_init(void);

void deviceStatus_saveStructData_commandFromPhone(bool commandFromPhone);

void deviceStatus_saveStructData_isMeasuring(bool isMeasuring);

void deviceStatus_saveStructData_isDataOnFlash(bool isDataOnFlash);

void deviceStatus_saveStructData_fileName(uint8_t fileName_year, uint8_t fileName_month, uint8_t fileName_day, uint8_t fileName_hrs, uint8_t fileName_mins, uint8_t fileName_secs);


device_status_data deviceStatus_getStructData(void);

bool deviceStatus_getStructData_commandFromPhone(void);

bool deviceStatus_getStructData_isMeasuring(void);

bool deviceStatus_getStructData_isDataOnFlash(void);


#endif /* CUSTOM_STATUS_STRUCT_DATA_H */