
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
