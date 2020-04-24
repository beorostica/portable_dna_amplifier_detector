
#ifndef QSPI_CUSTOM_H
#define QSPI_CUSTOM_H


#include <stdint.h>
#include "custom_detection_system_struct_data.h"
#include "custom_control_system_struct_data.h"

void qspiInit(void);

void qspiPushSampleInExternalFlash(detection_system_data sample);

void qspiReadExternalFlashAndSendBleDataIfPossible(void);

void qspiControlSystem_PushSampleInExternalFlash(control_system_data sample);

void qspiControlSystem_ReadExternalFlashAndSendBleDataIfPossible(void);


#endif /* QSPI_CUSTOM_H */
