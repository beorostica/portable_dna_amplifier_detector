
//Include Modules:
#include "custom_log.h"
#include "custom_timer.h"
#include "custom_detection_system_struct_data.h"
#include "custom_ble_manager.h"

#include <stdlib.h>

//Main Function:
int main(void)
{

    //Init Log for debugging:
    logInit();

    //Init and Start Timers:
    timerInit();
    timerDetectionSystem_Start();
    secondsStart();
    hundredMillisStart();

    //Initialize BLE:
    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();

    //Start execution:
    advertising_start();

    //Print Initial Message:
    NRF_LOG_INFO("*********************");
    NRF_LOG_INFO("*** Project Start ***");
    NRF_LOG_INFO("*********************");

    srand(0);

    //Enter main loop:
    while(1)
    {
        ////////////////////////////////////////////////////////////////
        /// Detection System Task //////////////////////////////////////
        ////////////////////////////////////////////////////////////////
        static uint8_t counter = 0;
        
        if(timerDetectionSystem_GetFlag())
        {
            if(secondsGetFlag())
            {
                secondsClearFlag();

                switch(counter) 
                {
                    case 0 :
                    {
                        //Save struct data before and Change the mosfet state for detection:
                        detectionSystem_saveStructData_before(counter, 0, (uint16_t) rand());
                        
                        //Increase counter:
                        counter++;
                        break;
                    }
                    case 1 :
                    {
                        //Save struct data after and Change the mosfet state for non-detection:
                        detectionSystem_saveStructData_after((counter-1), 1, (uint16_t) rand(), (uint16_t) secondsGetTime());

                        //Save struct data before and Change the mosfet state for detection:
                        detectionSystem_saveStructData_before(counter, 0, (uint16_t) rand());

                        //Increase counter:
                        counter++;
                        break;
                    }
                    case 2 :
                    {
                        //Save struct data after and Change the mosfet state for non-detection:
                        detectionSystem_saveStructData_after((counter-1), 1, (uint16_t) rand(), (uint16_t) secondsGetTime());

                        //Save struct data before and Change the mosfet state for detection:
                        detectionSystem_saveStructData_before(counter, 0, (uint16_t) rand());

                        //Increase counter:
                        counter++;
                        break;
                    }
                    case 3 :
                    {
                        //Save struct data after and Change the mosfet state for non-detection:
                        detectionSystem_saveStructData_after((counter-1), 1, (uint16_t) rand(), (uint16_t) secondsGetTime());

                        //Save struct data before and Change the mosfet state for detection:
                        detectionSystem_saveStructData_before(counter, 0, (uint16_t) rand());

                        //Increase counter:
                        counter++;
                        break;
                    }
                    case 4 :
                    {
                        //Save struct data after and Change the mosfet state for non-detection:
                        detectionSystem_saveStructData_after((counter-1), 1, (uint16_t) rand(), (uint16_t) secondsGetTime());

                        //Get the detection system struct data:
                        detection_system_data dsData = detectionSystem_getStructData();
                        NRF_LOG_INFO("time = %d. mosfetBefore = %d. sensorBefore = %d. mosfetAfter = %d. sensorAfter = %d", dsData.time[0], dsData.mosfetActuator_before[0], dsData.lightSensor_before[0], dsData.mosfetActuator_after[0], dsData.lightSensor_after[0]);
                        NRF_LOG_INFO("time = %d. mosfetBefore = %d. sensorBefore = %d. mosfetAfter = %d. sensorAfter = %d", dsData.time[1], dsData.mosfetActuator_before[1], dsData.lightSensor_before[1], dsData.mosfetActuator_after[1], dsData.lightSensor_after[1]);
                        NRF_LOG_INFO("time = %d. mosfetBefore = %d. sensorBefore = %d. mosfetAfter = %d. sensorAfter = %d", dsData.time[2], dsData.mosfetActuator_before[2], dsData.lightSensor_before[2], dsData.mosfetActuator_after[2], dsData.lightSensor_after[2]);
                        NRF_LOG_INFO("time = %d. mosfetBefore = %d. sensorBefore = %d. mosfetAfter = %d. sensorAfter = %d", dsData.time[3], dsData.mosfetActuator_before[3], dsData.lightSensor_before[3], dsData.mosfetActuator_after[3], dsData.lightSensor_after[3]);

                        //Send BT data if notifications are anabled by Phone:
                        if(bleGetNotificationFlag())
                            bleSendData(detectionSystem_getStructSingleData(dsData, 0));

                        //Reset counter and timer flag:
                        counter = 0;
                        timerDetectionSystem_ClearFlag();
                        break;
                    }
                }
                

            }
        }

        //Print time every 0.1[s]:
        if(hundredMillisGetFlag())
        {
            hundredMillisClearFlag();
            
            NRF_LOG_INFO("hundredMillisGetTime(): %d", hundredMillisGetTime());
        }
        

    }
}


/**
 * @}
 */