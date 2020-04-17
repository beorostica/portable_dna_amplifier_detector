
#ifndef CUSTOM_PID_CONTROLLER_H
#define CUSTOM_PID_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

void pwmInit(void);

void pwmSetDutyCycle(uint8_t value);


#endif /* CUSTOM_PID_CONTROLLER_H */