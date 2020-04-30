
//Include Modules:
#include "custom_log.h"
#include "custom_timer.h"
#include "custom_detection_system_struct_data.h"
#include "custom_qspi.h"
#include "boards.h"
#include "custom_twi.h"
#include "custom_ble_manager.h"
#include "custom_device_status_struct_data.h"
#include "custom_pid_controller.h"
#include "custom_control_system_struct_data.h"
#include "custom_battery_system_struct_data.h"


//Main Function:
int main(void)
{

    //Init Log for debugging:
    logInit();

    //Print Message:
    NRF_LOG_INFO("");
    NRF_LOG_INFO("*********************");
    NRF_LOG_INFO("**** Setup Start ****");
    NRF_LOG_INFO("*********************");
    NRF_LOG_INFO("");


    //Init Timers:
    timerInit();

    //Initialize other Peripherals:
    qspiInit();                       //Init QSPI for external flash (qspiInit() must be after the timers are started, otherwise the qspi throws error on the first burn (but after reset works ok)).
    bsp_board_init(BSP_INIT_LEDS);    //Init digital output
    twiInit();                        //Init TWI for ADS1015

    //Initialize BLE:
    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
    
    //Start Advertising:
    advertising_start();

    //Set initial status data and STAT characteristic of the device:
    deviceStatus_saveStructData_init();
    bleCusStatSendData(deviceStatus_getStructData());

    //Configure the PID controller peripherals:
    pidInit();

    /////////////////////////////////////////////////////////
    //Configure battery babysitter:
    bq27441_begin();

    //Start Battery System Timer:
    timerBatterySystem_Start();
    /////////////////////////////////////////////////////////

    //Print Message:
    NRF_LOG_INFO("");
    NRF_LOG_INFO("**********************");
    NRF_LOG_INFO("***** Loop Start *****");
    NRF_LOG_INFO("**********************");
    NRF_LOG_INFO("");

    //Enter main loop:
    while(1)
    {
        
        //If the measuring flag is true:
        if(deviceStatus_getStructData_isMeasuring())
        {

            ////////////////////////////////////////////////////////////////
            /// Temp Control Task //////////////////////////////////////////
            ////////////////////////////////////////////////////////////////
            static uint32_t timeSecondsPast = 0;
            static uint16_t adcReference = 768;
            if(timerControlSystem_GetFlag())
            {
                timerControlSystem_ClearFlag();
                
                // Measuring, Processing and Manipulating for PID controller:
                uint16_t adcValue = pidGetAdcValue();
                uint8_t pwmValue = pidGetPwmAction(adcValue, adcReference);
                pidSetPwmAction(pwmValue);

                // Save and get the data in the static control_system_data struct:
                controlSystem_saveStructData((uint16_t) secondsGetTime(), adcReference, adcValue, (uint16_t) pwmValue);
                control_system_data csData = controlSystem_getStructData();
                //NRF_LOG_INFO("Count: %d. PWM: %d. REF: %d. ADC: %d.", csData.time, csData.uPwm, csData.refAdc, csData.yAdc);

                // Save data in external flash after a certain time:
                if (timerControlSystem_SaveExternalFlash_GetFlag()) 
                {
                    timerControlSystem_SaveExternalFlash_ClearFlag();

                    //(1) First. Save on flash for "Cont":
                    qspiControlSystem_PushSampleInExternalFlash(csData);
                }

                // Change the reference signal:
                if (secondsGetTime() - timeSecondsPast > 60){
                    timeSecondsPast = secondsGetTime();
                    if (adcReference == 768) {
                        adcReference = 256;
                    } else {
                        adcReference = 768;
                    }
                }

            }

            ////////////////////////////////////////////////////////////////
            /// Detection System Task //////////////////////////////////////
            ////////////////////////////////////////////////////////////////
            static uint8_t counter = 0;
            if(timerDetectionSystem_GetFlag())
            {
                if(secondsGetFlag())
                {
                    secondsClearFlag();
                
                    static uint8_t mosfetActuator;
                    static uint16_t lightSensor;
                    static uint16_t time;
                
                    switch(counter) 
                    {
                        case 0 :
                        {
                            //Save struct data before and Change the mosfet state for detection:
                            mosfetActuator = (uint8_t) bsp_board_led_state_get(counter);
                            lightSensor    = ads1015Read(counter);
                            NRF_LOG_INFO("lightSensor[%d]: %d", counter, lightSensor);
                            detectionSystem_saveStructData_before(counter, mosfetActuator, lightSensor);
                            bsp_board_led_on(counter);
                            
                            //Increase counter:
                            counter++;
                            break;
                        }
                        case 1 :
                        {
                            //Save struct data after and Change the mosfet state for non-detection:
                            mosfetActuator = (uint8_t) bsp_board_led_state_get(counter-1);
                            lightSensor    = ads1015Read(counter-1);
                            time           = (uint16_t) secondsGetTime();
                            NRF_LOG_INFO("lightSensor[%d]: %d", (counter-1), lightSensor);
                            detectionSystem_saveStructData_after((counter-1), mosfetActuator, lightSensor, time);
                            bsp_board_led_off(counter-1);
                
                            //Save struct data before and Change the mosfet state for detection:
                            mosfetActuator = (uint8_t) bsp_board_led_state_get(counter);
                            lightSensor    = ads1015Read(counter);
                            NRF_LOG_INFO("lightSensor[%d]: %d", counter, lightSensor);
                            detectionSystem_saveStructData_before(counter, mosfetActuator, lightSensor);
                            bsp_board_led_on(counter);
                
                            //Increase counter:
                            counter++;
                            break;
                        }
                        case 2 :
                        {
                            //Save struct data after and Change the mosfet state for non-detection:
                            mosfetActuator = (uint8_t) bsp_board_led_state_get(counter-1);
                            lightSensor    = ads1015Read(counter-1);
                            time           = (uint16_t) secondsGetTime();
                            NRF_LOG_INFO("lightSensor[%d]: %d", (counter-1), lightSensor);
                            detectionSystem_saveStructData_after((counter-1), mosfetActuator, lightSensor, time);
                            bsp_board_led_off(counter-1);
                
                            //Save struct data before and Change the mosfet state for detection:
                            mosfetActuator = (uint8_t) bsp_board_led_state_get(counter);
                            lightSensor    = ads1015Read(counter);
                            NRF_LOG_INFO("lightSensor[%d]: %d", counter, lightSensor);
                            detectionSystem_saveStructData_before(counter, mosfetActuator, lightSensor);
                            bsp_board_led_on(counter);
                
                            //Increase counter:
                            counter++;
                            break;
                        }
                        case 3 :
                        {
                            //Save struct data after and Change the mosfet state for non-detection:
                            mosfetActuator = (uint8_t) bsp_board_led_state_get(counter-1);
                            lightSensor    = ads1015Read(counter-1);
                            time           = (uint16_t) secondsGetTime();
                            NRF_LOG_INFO("lightSensor[%d]: %d", (counter-1), lightSensor);
                            detectionSystem_saveStructData_after((counter-1), mosfetActuator, lightSensor, time);
                            bsp_board_led_off(counter-1);
                
                            //Save struct data before and Change the mosfet state for detection:
                            mosfetActuator = (uint8_t) bsp_board_led_state_get(counter);
                            lightSensor    = ads1015Read(counter);
                            NRF_LOG_INFO("lightSensor[%d]: %d", counter, lightSensor);
                            detectionSystem_saveStructData_before(counter, mosfetActuator, lightSensor);
                            bsp_board_led_on(counter);
                
                            //Increase counter:
                            counter++;
                            break;
                        }
                        case 4 :
                        {
                            //Save struct data after and Change the mosfet state for non-detection:
                            mosfetActuator = (uint8_t) bsp_board_led_state_get(counter-1);
                            lightSensor    = ads1015Read(counter-1);
                            time           = (uint16_t) secondsGetTime();
                            NRF_LOG_INFO("lightSensor[%d]: %d", (counter-1), lightSensor);
                            detectionSystem_saveStructData_after((counter-1), mosfetActuator, lightSensor, time);
                            bsp_board_led_off(counter-1);
                
                            //Get the detection system struct data:
                            detection_system_data dsData = detectionSystem_getStructData();
                            NRF_LOG_INFO("time = %d. mosfetBefore = %d. sensorBefore = %d. mosfetAfter = %d. sensorAfter = %d", dsData.time[0], dsData.mosfetActuator_before[0], dsData.lightSensor_before[0], dsData.mosfetActuator_after[0], dsData.lightSensor_after[0]);
                            NRF_LOG_INFO("time = %d. mosfetBefore = %d. sensorBefore = %d. mosfetAfter = %d. sensorAfter = %d", dsData.time[1], dsData.mosfetActuator_before[1], dsData.lightSensor_before[1], dsData.mosfetActuator_after[1], dsData.lightSensor_after[1]);
                            NRF_LOG_INFO("time = %d. mosfetBefore = %d. sensorBefore = %d. mosfetAfter = %d. sensorAfter = %d", dsData.time[2], dsData.mosfetActuator_before[2], dsData.lightSensor_before[2], dsData.mosfetActuator_after[2], dsData.lightSensor_after[2]);
                            NRF_LOG_INFO("time = %d. mosfetBefore = %d. sensorBefore = %d. mosfetAfter = %d. sensorAfter = %d", dsData.time[3], dsData.mosfetActuator_before[3], dsData.lightSensor_before[3], dsData.mosfetActuator_after[3], dsData.lightSensor_after[3]);
                            
            
                            //(1) First. Save on flash for "Sens":
                            qspiDetectionSystem_PushSampleInExternalFlash(dsData);
                            
                            //If the time-out occurs or the command from the phone app is for stopping the detection system task, then stop it:
                            if((time > deviceStatus_getStructData_timeDuration_secs()) || (!deviceStatus_getStructData_commandFromPhone())){
                                NRF_LOG_INFO("MAIN: Detection System Task Stoped. time: %d.", time);

                                //Stop the detection system and the control system timers:
                                timerDetectionSystem_Stop();
                                secondsStop();
                                timerControlSystem_Stop();
                                timerControlSystem_SaveExternalFlash_Stop();

                                //Update the device status data (now it's not measuring):
                                deviceStatus_saveStructData_isMeasuring(false);
                                NRF_LOG_INFO("MAIN: isMeasuring = %d.", deviceStatus_getStructData_isMeasuring());

                                //Update the device status data (now commandFromPhone is set to false even though actually there isn't a command from phone when time-out expires):
                                deviceStatus_saveStructData_commandFromPhone(false);
                                NRF_LOG_INFO("MAIN: commandFromPhone = %d.", deviceStatus_getStructData_commandFromPhone());

                                //Update device status and notify STAT characteristic:;
                                bleCusStatSendData(deviceStatus_getStructData());
                                NRF_LOG_INFO("MAIN: Send notification of the STAT characteristic. commandFromPhone = %d. isMeasuring = %d. isDataOnFlash = %d", deviceStatus_getStructData_commandFromPhone(), deviceStatus_getStructData_isMeasuring(), deviceStatus_getStructData_isDataOnFlash());
                                                            
                            }
            
                            //Reset counter and timer flag:
                            counter = 0;
                            timerDetectionSystem_ClearFlag();
                            break;
                        }
                    }
                
                }
            }

            ////////////////////////////////////////////////////////////////
            /// Battery System Task (when measuring) ///////////////////////
            ////////////////////////////////////////////////////////////////
            if(timerBatterySystem_GetFlag())
            {
                timerBatterySystem_ClearFlag();

                //Read from babysitter:
                uint16_t time           = 0;
                uint16_t soc            = bq27441_getSoc();
                uint16_t capacityRemain = bq27441_getCapacityRemain();
                uint16_t capacityFull   = bq27441_getCapacityFull();
                uint8_t soh             = bq27441_getSoh();
                uint16_t voltage        = bq27441_getVoltage();
                int16_t current         = bq27441_getCurrent();
                int16_t power           = bq27441_getPower();

                //Save data in the static battery data struct:
                batterySystem_saveStructData(time, soc, capacityRemain, capacityFull, (uint16_t)soh, voltage, (uint16_t)current, (uint16_t)power);
  
                //Get the data from the static battery data struct:
                battery_system_data bsData = batterySystem_getStructData();

                //Print values for debugging:
                //NRF_LOG_INFO("soc: %d. capRem: %d. capFull: %d. voltage: %d. current: %d. power: %d.", bsData.soc, bsData.capacityRemain, bsData.capacityFull, bsData.voltage, bsData.current, bsData.power);

                //(1) First. Save on flash for "Batt":
                qspiBatterySystem_PushSampleInExternalFlash(bsData);
            }

        }
        //But if it's not measuring:
        else
        {
            ///////////////////////////////////////////////////////////////////////
            /// Battery System Task (when phone connected and not measuring) //////
            ///////////////////////////////////////////////////////////////////////
            //Only if there is a phone connected with Batt notification enabled, then send battery data via ble:
            if(bleGetCusBattNotificationFlag() && timerBatterySystem_GetFlag())
            {
                timerBatterySystem_ClearFlag();

                //Read from babysitter:
                uint16_t time           = 0;
                uint16_t soc            = bq27441_getSoc();
                uint16_t capacityRemain = bq27441_getCapacityRemain();
                uint16_t capacityFull   = bq27441_getCapacityFull();
                uint8_t soh             = bq27441_getSoh();
                uint16_t voltage        = bq27441_getVoltage();
                int16_t current         = bq27441_getCurrent();
                int16_t power           = bq27441_getPower();

                //Save data in the static battery data struct:
                batterySystem_saveStructData(time, soc, capacityRemain, capacityFull, (uint16_t)soh, voltage, (uint16_t)current, (uint16_t)power);
  
                //Get the data from the static battery data struct:
                battery_system_data bsData = batterySystem_getStructData();

                //Print values for debugging:
                NRF_LOG_INFO("soc: %d. capRem: %d. capFull: %d. voltage: %d. current: %d. power: %d.", bsData.soc, bsData.capacityRemain, bsData.capacityFull, bsData.voltage, bsData.current, bsData.power);

                //Just send battery ble data, but don't save on flash:
                if(bleGetCusBattNotificationFlag())
                {
                    bleCusBattSendData(bsData);
                    NRF_LOG_INFO("BLE BATT SERVICE: send. Read from main(). time: %d", bsData.time);
                }
            }
        }

        ////////////////////////////////////////////////////////////////
        /// BLE data transmission Task /////////////////////////////////
        ////////////////////////////////////////////////////////////////
        //If the data on flash flag is true:
        if (deviceStatus_getStructData_isDataOnFlash()) 
        {
            // Do something (try to send BLE data) after 0.1 seconds:
            if(hundredMillisGetFlag())
            {
                hundredMillisClearFlag();

                //If the nRF52840 is connected and the notifications for "Cont" are enabled and there is data on flash for "Cont", then try to send BT data:
                if(bleGetCusContNotificationFlag() && deviceStatus_getStructData_isContDataOnFlash())
                {
                    //(2) Second. Read from flash and send via BLE for "Cont":
                    qspiControlSystem_ReadExternalFlashAndSendBleDataIfPossible();
                }

                //If the nRF52840 is connected and the notifications for "Sens" are enabled and there is data on flash for "Sens", then try to send BT data:
                if(bleGetCusSensNotificationFlag() && deviceStatus_getStructData_isSensDataOnFlash()){
                    //(2) Second. Read from flash and send via BLE for "Sens":
                    qspiDetectionSystem_ReadExternalFlashAndSendBleDataIfPossible();
                }

                //If the nRF52840 is connected and the notifications for "Batt" are enabled and there is data on flash for "Batt", then try to send BT data:
                if(bleGetCusBattNotificationFlag() && deviceStatus_getStructData_isBattDataOnFlash()){
                    //(2) Second. Read from flash and send via BLE for "Batt":
                    qspiBatterySystem_ReadExternalFlashAndSendBleDataIfPossible();
                }

            }
        }

       
    }
}


/**
 * @}
 */