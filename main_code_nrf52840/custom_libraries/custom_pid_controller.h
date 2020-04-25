
#ifndef CUSTOM_PID_CONTROLLER_H
#define CUSTOM_PID_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

void pidInit(void);

uint16_t pidGetAdcValue(void);

uint8_t pidGetPwmAction(uint16_t adcValue, uint16_t adcReference);

void pidSetPwmAction(uint8_t value);


#endif /* CUSTOM_PID_CONTROLLER_H */