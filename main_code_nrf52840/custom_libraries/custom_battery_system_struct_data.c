
#include "custom_battery_system_struct_data.h"

static battery_system_data bsData;

void batterySystem_saveStructData(uint16_t time, uint16_t soc, uint16_t capacityRemain, uint16_t capacityFull, uint16_t soh, uint16_t voltage, uint16_t current, uint16_t  power)
{
    bsData.time = time;
    bsData.soc = soc;
    bsData.capacityRemain = capacityRemain;
    bsData.capacityFull = capacityFull;
    bsData.soh = soh;
    bsData.voltage = voltage;
    bsData.current = current;
    bsData.power = power;
}

battery_system_data batterySystem_getStructData(void)
{
    return bsData;
}