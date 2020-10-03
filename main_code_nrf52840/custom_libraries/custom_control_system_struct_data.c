
#include "custom_control_system_struct_data.h"

static control_system_data csData;

void controlSystem_saveStructData(uint16_t time, uint16_t refAdc, uint16_t yAdc, uint16_t uPwm, uint16_t refAdcHotlid, uint16_t yAdcHotlid, uint16_t uPwmHotlid, uint16_t yAdcTamb)
{
    csData.time = time;
    csData.refAdc = refAdc;
    csData.yAdc = yAdc;
    csData.uPwm = uPwm;
    csData.refAdcHotlid = refAdcHotlid;
    csData.yAdcHotlid = yAdcHotlid;
    csData.uPwmHotlid = uPwmHotlid;
    csData.yAdcTamb = yAdcTamb;
}

control_system_data controlSystem_getStructData(void)
{
    return csData;
}