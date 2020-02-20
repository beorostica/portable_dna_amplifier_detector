
#ifndef CUSTOM_BLE_MANAGER_H
#define CUSTOM_BLE_MANAGER_H


#include <stdbool.h>


void ble_stack_init(void);
void gap_params_init(void);
void gatt_init(void);
void advertising_init(void);
void services_init(void);
void conn_params_init(void);
void advertising_start(void);


#endif /* CUSTOM_BLE_MANAGER_H */
