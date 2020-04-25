
#include "custom_device_status_struct_data.h"

static device_status_data statusData;

void deviceStatus_saveStructData_init(void)
{
    statusData.commandFromPhone = false;
    statusData.isMeasuring      = false;
    statusData.isDataOnFlash    = false;
    statusData.fileName_year  = 0;
    statusData.fileName_month = 0;
    statusData.fileName_day   = 0;
    statusData.fileName_hrs   = 0;
    statusData.fileName_mins  = 0;
    statusData.fileName_secs  = 0;
    statusData.timeDuration_hrs  = 0;
    statusData.timeDuration_mins = 1;
    statusData.timeDuration_secs = 0;
    statusData.tempReference = 37;
    statusData.isSensDataOnFlash = false;
    statusData.isContDataOnFlash = false;
}

void deviceStatus_saveStructData_commandFromPhone(bool commandFromPhone)
{
    statusData.commandFromPhone = commandFromPhone;
}

void deviceStatus_saveStructData_isMeasuring(bool isMeasuring)
{
    statusData.isMeasuring = isMeasuring;
}

void deviceStatus_saveStructData_isDataOnFlash(bool isDataOnFlash)
{
    statusData.isDataOnFlash = isDataOnFlash;
}

void deviceStatus_saveStructData_fileName(uint8_t fileName_year, uint8_t fileName_month, uint8_t fileName_day, uint8_t fileName_hrs, uint8_t fileName_mins, uint8_t fileName_secs)
{
    statusData.fileName_year  = fileName_year;
    statusData.fileName_month = fileName_month;
    statusData.fileName_day   = fileName_day;
    statusData.fileName_hrs   = fileName_hrs;
    statusData.fileName_mins  = fileName_mins;
    statusData.fileName_secs  = fileName_secs;
}

void deviceStatus_saveStructData_timeDuration(uint8_t timeDuration_hrs, uint8_t timeDuration_mins, uint8_t timeDuration_secs)
{
    if(timeDuration_hrs > 17){  timeDuration_hrs  = 17;}
    if(timeDuration_mins > 59){ timeDuration_mins = 59;}
    if(timeDuration_secs > 59){ timeDuration_secs = 59;}
    statusData.timeDuration_hrs  = timeDuration_hrs;
    statusData.timeDuration_mins = timeDuration_mins;
    statusData.timeDuration_secs = timeDuration_secs;
}

void deviceStatus_saveStructData_tempReference(uint8_t tempReference)
{
    if(tempReference > 100){ tempReference = 100;}
    if(tempReference < 5){   tempReference = 5;}
    statusData.tempReference = tempReference;
}

void deviceStatus_saveStructData_isSensDataOnFlash(bool isSensDataOnFlash)
{
    statusData.isSensDataOnFlash = isSensDataOnFlash;
}

void deviceStatus_saveStructData_isContDataOnFlash(bool isContDataOnFlash)
{
    statusData.isContDataOnFlash = isContDataOnFlash;
}


device_status_data deviceStatus_getStructData(void)
{
    return statusData;
}

bool deviceStatus_getStructData_commandFromPhone(void)
{
    return statusData.commandFromPhone;
}

bool deviceStatus_getStructData_isMeasuring(void)
{
    return statusData.isMeasuring;
}

bool deviceStatus_getStructData_isDataOnFlash(void)
{
    return statusData.isDataOnFlash;
}

uint16_t deviceStatus_getStructData_timeDuration_secs(void)
{
    uint16_t hrs  = (uint16_t) statusData.timeDuration_hrs;
    uint16_t mins = (uint16_t) statusData.timeDuration_mins;
    uint16_t secs = (uint16_t) statusData.timeDuration_secs;
    return (hrs*3600 + mins*60 + secs);
}

uint8_t deviceStatus_getStructData_tempReference(void)
{
    return statusData.tempReference;
}

bool deviceStatus_getStructData_isSensDataOnFlash(void)
{
    return statusData.isSensDataOnFlash;
}

bool deviceStatus_getStructData_isContDataOnFlash(void)
{
    return statusData.isContDataOnFlash;
}