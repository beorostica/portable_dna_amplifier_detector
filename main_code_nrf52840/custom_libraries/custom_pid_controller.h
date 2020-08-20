
#ifndef CUSTOM_PID_CONTROLLER_H
#define CUSTOM_PID_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

typedef struct 
{ 
    uint16_t  adcHeater;
    uint16_t  adcHotlid;
    uint16_t  adcTamb;
} adc_data_t;

void pidInit(void);

adc_data_t pidGetAdcValues(void);

uint8_t pid1GetPwmAction(uint16_t adcValue, uint16_t adcReference);

uint8_t pid2GetPwmAction(uint16_t adcValue, uint16_t adcReference);

void pid1SetPwmAction(uint8_t value);

void pid2SetPwmAction(uint8_t value);


#endif /* CUSTOM_PID_CONTROLLER_H */