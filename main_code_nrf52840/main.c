
#include "custom_log.h"
#include "custom_timer.h"
#include "custom_ble_manager.h"


/**@brief Function for application main entry.
 */
int main(void)
{

    //Init Log for debugging:
    logInit();

    //Init and Start Timers:
    timerInit();
    timerDetectionSystem_Start();
    secondsStart();
    hundredMillisStart();

    timers_init();

    //Initialize Peripherals:
    bool erase_bonds;
    buttons_leds_init(&erase_bonds);
    //power_management_init();

    //Initialize BLE:
    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
    peer_manager_init();

    //Start execution:
    application_timers_start();
    advertising_start(erase_bonds);

    //Enter main loop:
    while(1)
    {
        //idle_state_handle();

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
                        NRF_LOG_INFO("secondsGetTime(): %d", secondsGetTime());

                        //Increase counter:
                        counter++;
                        break;
                    }
                    case 1 :
                    {
                        NRF_LOG_INFO("secondsGetTime(): %d", secondsGetTime());

                        //Increase counter:
                        counter++;
                        break;
                    }
                    case 2 :
                    {
                        NRF_LOG_INFO("secondsGetTime(): %d", secondsGetTime());

                        //Increase counter:
                        counter++;
                        break;
                    }
                    case 3 :
                    {
                        NRF_LOG_INFO("secondsGetTime(): %d", secondsGetTime());

                        //Increase counter:
                        counter++;
                        break;
                    }
                    case 4 :
                    {
                        NRF_LOG_INFO("secondsGetTime(): %d", secondsGetTime());

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