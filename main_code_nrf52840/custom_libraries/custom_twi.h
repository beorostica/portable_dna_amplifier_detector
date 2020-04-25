
#ifndef CUSTOM_TWI_H
#define CUSTOM_TWI_H


#include <stdint.h>
#include <stdbool.h>

void twiInit (void);

uint16_t ads1015Read(uint8_t channel);

bool bq27441_begin(void);
//bool bq27441_setCapacity(uint16_t capacity);
uint16_t bq27441_getSoc(void);
uint16_t bq27441_getVoltage(void);
int16_t bq27441_getCurrent(void);
uint16_t bq27441_getCapacityRemain(void);
uint16_t bq27441_getCapacityFull(void);
int16_t bq27441_getPower(void);
uint8_t bq27441_getSoh(void);


#endif /* CUSTOM_TWI_H */