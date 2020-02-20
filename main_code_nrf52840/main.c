
#include "custom_ble_manager.h"


/**@brief Function for application main entry.
 */
int main(void)
{
    bool erase_bonds;

    //Initialize Peripherals:
    log_init();
    timers_init();
    buttons_leds_init(&erase_bonds);
    power_management_init();

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
        idle_state_handle();
    }
}


/**
 * @}
 */