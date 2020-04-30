
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
    uint8_t   timeDuration_hrs;
    uint8_t   timeDuration_mins;
    uint8_t   timeDuration_secs;
    uint8_t   tempReference;
    bool      isSensDataOnFlash;
    bool      isContDataOnFlash;
    bool      isBattDataOnFlash;
} device_status_data;


void deviceStatus_saveStructData_init(void);

void deviceStatus_saveStructData_commandFromPhone(bool commandFromPhone);

void deviceStatus_saveStructData_isMeasuring(bool isMeasuring);

void deviceStatus_saveStructData_isDataOnFlash(bool isDataOnFlash);

void deviceStatus_saveStructData_fileName(uint8_t fileName_year, uint8_t fileName_month, uint8_t fileName_day, uint8_t fileName_hrs, uint8_t fileName_mins, uint8_t fileName_secs);

void deviceStatus_saveStructData_timeDuration(uint8_t timeDuration_hrs, uint8_t timeDuration_mins, uint8_t timeDuration_secs);

void deviceStatus_saveStructData_tempReference(uint8_t tempReference);

void deviceStatus_saveStructData_isSensDataOnFlash(bool isSensDataOnFlash);

void deviceStatus_saveStructData_isContDataOnFlash(bool isContDataOnFlash);

void deviceStatus_saveStructData_isBattDataOnFlash(bool isBattDataOnFlash);


device_status_data deviceStatus_getStructData(void);

bool deviceStatus_getStructData_commandFromPhone(void);

bool deviceStatus_getStructData_isMeasuring(void);

bool deviceStatus_getStructData_isDataOnFlash(void);

uint16_t deviceStatus_getStructData_timeDuration_secs(void);

uint8_t deviceStatus_getStructData_tempReference(void);

bool deviceStatus_getStructData_isSensDataOnFlash(void);

bool deviceStatus_getStructData_isContDataOnFlash(void);

bool deviceStatus_getStructData_isBattDataOnFlash(void);


#endif /* CUSTOM_STATUS_STRUCT_DATA_H */