
#ifndef CUSTOM_PID_CONTROLLER_H
#define CUSTOM_PID_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

void pwmInit(void);

void pwmSetDutyCycle(uint8_t value);

void adcInit(void);

uint16_t adcGetValue(void);

uint8_t pidGetAction(uint16_t adcValue, uint16_t adcReference);


#endif /* CUSTOM_PID_CONTROLLER_H */