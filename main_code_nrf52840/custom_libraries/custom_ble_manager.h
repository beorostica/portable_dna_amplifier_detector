
#ifndef CUSTOM_BLE_MANAGER_H
#define CUSTOM_BLE_MANAGER_H


#include <stdbool.h>

#include "custom_detection_system_struct_data.h"


void ble_stack_init(void);
void gap_params_init(void);
void gatt_init(void);
void advertising_init(void);
void services_init(void);
void conn_params_init(void);
void advertising_start(void);

bool bleGetCusNotificationFlag(void);
void bleCusSendData(detection_system_single_data data);

bool bleGetCusStatNotificationFlag(void);
void bleCusStatSendData(detection_system_single_data data);

bool bleGetCusSensNotificationFlag(void);
void bleCusSensSendData(detection_system_single_data data);

#endif /* CUSTOM_BLE_MANAGER_H */
