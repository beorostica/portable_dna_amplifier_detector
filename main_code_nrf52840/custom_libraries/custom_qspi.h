
#ifndef QSPI_CUSTOM_H
#define QSPI_CUSTOM_H


#include <stdint.h>
#include "custom_detection_system_struct_data.h"

void qspiInit(void);

void qspiPushSampleInExternalFlash(detection_system_data sample);

void qspiReadExternalFlashAndSendBleDataIfPossible(void);


#endif /* QSPI_CUSTOM_H */
