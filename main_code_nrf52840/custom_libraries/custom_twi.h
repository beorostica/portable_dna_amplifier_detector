
#ifndef CUSTOM_TWI_H
#define CUSTOM_TWI_H


#include <stdint.h>

void twiInit (void);

uint16_t ads1015Read(uint8_t channel);


#endif /* CUSTOM_TWI_H */