
#include "custom_ble_manager.h"
#include "cus_stat.h"
#include "cus_sens.h"
#include "cus_cont.h"
#include "cus_batt.h"

#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "app_timer.h"

#include "custom_timer.h"
#include "custom_log.h"


#define DEVICE_NAME                     "DNAamplifier"                     /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME               "NordicSemiconductor"                   /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                800                                     //800 /**<0.5[s]>**/. //300 /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */

#define APP_ADV_DURATION                0                                       //0 /**< Never Timeout >**/. //18000 /**< The advertising duration (180 seconds) in units of 10 milliseconds. */
#define APP_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                       /**< A tag identifying the SoftDevice BLE configuration. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)        /**< Minimum acceptable connection interval (0.1 seconds). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(200, UNIT_1_25_MS)        /**< Maximum acceptable connection interval (0.2 second). */
#define SLAVE_LATENCY                   0                                       /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                   /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                  /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                       /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

NRF_BLE_GATT_DEF(m_gatt);                                                       /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                         /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                             /**< Advertising module instance. */

/* YOUR_JOB: Declare all services structure your application is using
 *  BLE_XYZ_DEF(m_xyz);
 */
BLE_CUS_STAT_DEF(m_cus_stat);
BLE_CUS_SENS_DEF(m_cus_sens);
BLE_CUS_CONT_DEF(m_cus_cont);
BLE_CUS_BATT_DEF(m_cus_batt);


static bool isCusStatNotificationEnabled = false;
static bool isCusSensNotificationEnabled = false;
static bool isCusContNotificationEnabled = false;
static bool isCusBattNotificationEnabled = false;

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                        /**< Handle of the current connection. */

// YOUR_JOB: Use UUIDs for service(s) used in your application.
static ble_uuid_t m_adv_uuids[] =                                               /**< Universally unique service identifiers. */
{
    {CUSTOM_SENS_SERVICE_UUID, BLE_UUID_TYPE_VENDOR_BEGIN}
};


//Events or handlers:
static void nrf_qwr_error_handler(uint32_t nrf_error);                      //For handling Queued Write Module errors
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt);              //For handling the Connection Parameters Module
static void conn_params_error_handler(uint32_t nrf_error);                  //For handling a Connection Parameters error
static void on_adv_evt(ble_adv_evt_t ble_adv_evt);                          //For handling advertising events
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context); //For handling BLE events
static void on_cus_stat_evt(cus_stat_t * p_cus_service, cus_stat_evt_t * p_evt);   //For handling custom service events
static void on_cus_sens_evt(cus_sens_t * p_cus_service, cus_sens_evt_t * p_evt);   //For handling custom service events
static void on_cus_cont_evt(cus_cont_t * p_cus_service, cus_cont_evt_t * p_evt);   //For handling custom service events
static void on_cus_batt_evt(cus_batt_t * p_cus_service, cus_batt_evt_t * p_evt);   //For handling custom service events


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("BLE_MANAGER: Fast advertising.");
            break;

        case BLE_ADV_EVT_IDLE:
            break;

        default:
            break;
    }
}


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_DISCONNECTED:
            isCusStatNotificationEnabled = false;
            isCusSensNotificationEnabled = false;
            isCusContNotificationEnabled = false;
            isCusBattNotificationEnabled = false;
            NRF_LOG_INFO("BLE_MANAGER: isCusStatNotificationEnabled: false");
            NRF_LOG_INFO("BLE_MANAGER: isCusSensNotificationEnabled: false");
            NRF_LOG_INFO("BLE_MANAGER: isCusContNotificationEnabled: false");
            NRF_LOG_INFO("BLE_MANAGER: isCusBattNotificationEnabled: false");
            NRF_LOG_INFO("BLE_MANAGER: Disconnected.");
            break;

        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("BLE_MANAGER: Connected.");
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("BLE_MANAGER: PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            isCusStatNotificationEnabled = false;
            isCusSensNotificationEnabled = false;
            isCusContNotificationEnabled = false;
            isCusBattNotificationEnabled = false;
            NRF_LOG_INFO("BLE_MANAGER: isCusStatNotificationEnabled: false");
            NRF_LOG_INFO("BLE_MANAGER: isCusSensNotificationEnabled: false");
            NRF_LOG_INFO("BLE_MANAGER: isCusContNotificationEnabled: false");
            NRF_LOG_INFO("BLE_MANAGER: isCusBattNotificationEnabled: false");
            NRF_LOG_DEBUG("BLE_MANAGER: GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            isCusStatNotificationEnabled = false;
            isCusSensNotificationEnabled = false;
            isCusContNotificationEnabled = false;
            isCusBattNotificationEnabled = false;
            NRF_LOG_INFO("BLE_MANAGER: isCusStatNotificationEnabled: false");
            NRF_LOG_INFO("BLE_MANAGER: isCusSensNotificationEnabled: false");
            NRF_LOG_INFO("BLE_MANAGER: isCusContNotificationEnabled: false");
            NRF_LOG_INFO("BLE_MANAGER: isCusBattNotificationEnabled: false");
            NRF_LOG_DEBUG("BLE_MANAGER: GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for handling the YYY Service events.
 * YOUR_JOB implement a service handler function depending on the event the service you are using can generate
 *
 * @details This function will be called for all YY Service events which are passed to
 *          the application.
 *
 * @param[in]   p_yy_service   YY Service structure.
 * @param[in]   p_evt          Event received from the YY Service.
 *
 *
static void on_yys_evt(ble_yy_service_t     * p_yy_service,
                       ble_yy_service_evt_t * p_evt)
{
    switch (p_evt->evt_type)
    {
        case BLE_YY_NAME_EVT_WRITE:
            APPL_LOG("[APPL]: charact written with value %s. ", p_evt->params.char_xx.value.p_str);
            break;

        default:
            // No implementation needed.
            break;
    }
}
*/


/**@brief Function for handling the Custom Service Service events.
 *
 * @details This function will be called for all Custom Service events which are passed to
 *          the application.
 *
 * @param[in]   p_cus_service  Custom Service structure.
 * @param[in]   p_evt          Event received from the Custom Service.
 *
 */
static void on_cus_stat_evt(cus_stat_t * p_cus_service, cus_stat_evt_t * p_evt)
{
    uint32_t err_code;
    static ble_gatts_value_t gatts_value;
    static uint8_t data_buffer[sizeof(device_status_data)];
        
    switch(p_evt->evt_type)
    {
        case CUS_STAT_EVT_NOTIFICATION_ENABLED:
            isCusStatNotificationEnabled = true;
            NRF_LOG_INFO("BLE_MANAGER: isCusStatNotificationEnabled: true");
            break;

        case CUS_STAT_EVT_NOTIFICATION_DISABLED:
            isCusStatNotificationEnabled = false;
            NRF_LOG_INFO("BLE_MANAGER: isCusStatNotificationEnabled: false");
            break;

        case CUS_STAT_EVT_CONNECTED :
            break;

        case CUS_STAT_EVT_DISCONNECTED:
            isCusStatNotificationEnabled = false;
            NRF_LOG_INFO("BLE_MANAGER: isCusStatNotificationEnabled: false");
            break;

        case CUS_STAT_EVT_WRITE:

            //Allocate memory for "gatts_value" variable:
            memset(&gatts_value, 0, sizeof(gatts_value));
            gatts_value.offset  = 0;
            gatts_value.len     = sizeof(device_status_data);
            gatts_value.p_value = (uint8_t*) &data_buffer;

            //If the STAT characteristic notification is enabled, then the STAT characteristic changed by the NordicSoftdevice is valid:
            if(bleGetCusStatNotificationFlag())
            {               
                //Get the last STAT characteristic written by the phone app and store on "data_buffer":
                err_code = sd_ble_gatts_value_get(p_cus_service->conn_handle, p_cus_service->custom_value_handles.value_handle, &gatts_value);
                APP_ERROR_CHECK(err_code);

                //Store the commandFromPhoneAux = command of the STAT characteristic written by the phone:
                bool commandFromPhoneAux = (bool) data_buffer[0];
                
                //The command of the STAT characteristic is analysed only if is different from the command in device status data:
                if(commandFromPhoneAux != deviceStatus_getStructData_commandFromPhone())
                {                    
                    //If the command from phone app is true:
                    if(commandFromPhoneAux){
                        //And if there is not previous data stored on flash, then start detection system task: 
                        if(!deviceStatus_getStructData_isDataOnFlash())
                        {
                            NRF_LOG_INFO("BLE_MANAGER: Write to STAT characteristic is gonna be stored in the device status data.");
                     
                            //Update the status data (now the nrf52 is commanded to start the detection task):
                            deviceStatus_saveStructData_commandFromPhone(true);
                            NRF_LOG_INFO("BLE_MANAGER: commandFromPhone = %d.", deviceStatus_getStructData_commandFromPhone());

                            //Start the detection system and temp control system timers:
                            NRF_LOG_INFO("BLE_MANAGER: Detection System and Temp Control System Start.");
                            timerDetectionSystem_Start();
                            secondsStart();
                            timerControlSystem_Start();
                            timerControlSystem_SaveExternalFlash_Start();
                            hundredMillisStart();
                            
                            //Update the device status data (now it's measuring and there is data on flash):
                            deviceStatus_saveStructData_isMeasuring(true);
                            NRF_LOG_INFO("BLE_MANAGER: isMeasuring = %d.", deviceStatus_getStructData_isMeasuring());
                            deviceStatus_saveStructData_isDataOnFlash(true);
                            deviceStatus_saveStructData_fileName(data_buffer[3], data_buffer[4], data_buffer[5], data_buffer[6], data_buffer[7], data_buffer[8]);
                            deviceStatus_saveStructData_timeDuration(data_buffer[9], data_buffer[10], data_buffer[11]);
                            deviceStatus_saveStructData_tempReference(data_buffer[12]);
                            deviceStatus_saveStructData_isSensDataOnFlash(true);
                            deviceStatus_saveStructData_isContDataOnFlash(true);
                            deviceStatus_saveStructData_isBattDataOnFlash(true);
                            NRF_LOG_INFO("BLE_MANAGER: isDataOnFlash = %d.", deviceStatus_getStructData_isDataOnFlash());
                            NRF_LOG_INFO("BLE_MANAGER: fileName_year = %d.", deviceStatus_getStructData().fileName_year);
                            NRF_LOG_INFO("BLE_MANAGER: fileName_month = %d.", deviceStatus_getStructData().fileName_month);
                            NRF_LOG_INFO("BLE_MANAGER: fileName_day = %d.", deviceStatus_getStructData().fileName_day);
                            NRF_LOG_INFO("BLE_MANAGER: fileName_hrs = %d.", deviceStatus_getStructData().fileName_hrs);
                            NRF_LOG_INFO("BLE_MANAGER: fileName_mins = %d.", deviceStatus_getStructData().fileName_mins);
                            NRF_LOG_INFO("BLE_MANAGER: fileName_secs = %d.", deviceStatus_getStructData().fileName_secs);
                            NRF_LOG_INFO("BLE_MANAGER: timeDuration_hrs = %d.", deviceStatus_getStructData().timeDuration_hrs);
                            NRF_LOG_INFO("BLE_MANAGER: timeDuration_mins = %d.", deviceStatus_getStructData().timeDuration_mins);
                            NRF_LOG_INFO("BLE_MANAGER: timeDuration_secs = %d.", deviceStatus_getStructData().timeDuration_secs);
                            NRF_LOG_INFO("BLE_MANAGER: tempReference = %d.", deviceStatus_getStructData().tempReference);
                            NRF_LOG_INFO("BLE_MANAGER: isSensDataOnFlash = %d.", deviceStatus_getStructData_isSensDataOnFlash());
                            NRF_LOG_INFO("BLE_MANAGER: isContDataOnFlash = %d.", deviceStatus_getStructData_isContDataOnFlash());
                            NRF_LOG_INFO("BLE_MANAGER: isBattDataOnFlash = %d.", deviceStatus_getStructData_isBattDataOnFlash());

                            //Change the "data_buffer" to update STAT characteristic:
                            data_buffer[0] = deviceStatus_getStructData_commandFromPhone();
                            data_buffer[1] = deviceStatus_getStructData_isMeasuring();
                            data_buffer[2] = deviceStatus_getStructData_isDataOnFlash();
                            data_buffer[3] = deviceStatus_getStructData().fileName_year;
                            data_buffer[4] = deviceStatus_getStructData().fileName_month;
                            data_buffer[5] = deviceStatus_getStructData().fileName_day;
                            data_buffer[6] = deviceStatus_getStructData().fileName_hrs;
                            data_buffer[7] = deviceStatus_getStructData().fileName_mins;
                            data_buffer[8] = deviceStatus_getStructData().fileName_secs;
                            data_buffer[9]  = deviceStatus_getStructData().timeDuration_hrs;
                            data_buffer[10] = deviceStatus_getStructData().timeDuration_mins;
                            data_buffer[11] = deviceStatus_getStructData().timeDuration_secs;
                            data_buffer[12] = deviceStatus_getStructData().tempReference;
                            data_buffer[13] = deviceStatus_getStructData_isSensDataOnFlash();
                            data_buffer[14] = deviceStatus_getStructData_isContDataOnFlash();
                            data_buffer[15] = deviceStatus_getStructData_isBattDataOnFlash();

                            //Send a notification back to the phone app of the STAT characteristic written (stored on "data_buffer"):
                            uint32_t err_code = cus_stat_custom_value_update(p_cus_service, data_buffer);
                            APP_ERROR_CHECK(err_code);
                            NRF_LOG_INFO("BLE_MANAGER: Send notification of the STAT characteristic. commandFromPhone = %d. isMeasuring = %d. isDataOnFlash = %d", deviceStatus_getStructData_commandFromPhone(), deviceStatus_getStructData_isMeasuring(), deviceStatus_getStructData_isDataOnFlash());
                        }
                        //But if there is previous data stored on flash, then print log info:
                        else
                        {
                            NRF_LOG_INFO("BLE_MANAGER: The nRF52 has previous data stored on flash, so you have to wait until nRF52 reads and sends all the data on flash.");
                            NRF_LOG_INFO("BLE_MANAGER: Write to STAT characteristic will not be stored in the device status data.");
                        }
                    }
                    //If the command from phone app is false:
                    else
                    {
                        //And if there is previous data stored on flash, then the main will stop to read and send data on flash soon:
                        if(deviceStatus_getStructData_isDataOnFlash())
                        {
                            NRF_LOG_INFO("BLE_MANAGER: Write to STAT characteristic is gonna be stored in the device status data.");
                     
                            //Update the status data (now the nrf52 is commanded to stop the detection task):
                            deviceStatus_saveStructData_commandFromPhone(false);
                            NRF_LOG_INFO("BLE_MANAGER: commandFromPhone = %d.", deviceStatus_getStructData_commandFromPhone());

                            //Change the "data_buffer" to update STAT characteristic:
                            data_buffer[0] = deviceStatus_getStructData_commandFromPhone();
                            data_buffer[1] = deviceStatus_getStructData_isMeasuring();

                            //Send a notification back to the phone app of the STAT characteristic written (stored on "data_buffer"):
                            uint32_t err_code = cus_stat_custom_value_update(p_cus_service, data_buffer);
                            APP_ERROR_CHECK(err_code);
                            NRF_LOG_INFO("BLE_MANAGER: Send notification of the STAT characteristic. commandFromPhone = %d. isMeasuring = %d. isDataOnFlash = %d", deviceStatus_getStructData_commandFromPhone(), deviceStatus_getStructData_isMeasuring(), deviceStatus_getStructData_isDataOnFlash());
                            NRF_LOG_INFO("BLE_MANAGER: Wait until all the data stored on flash is send via BLE.");
                        }
                        //But if there is not previous data stored on flash, then print log info:
                        else
                        {
                            NRF_LOG_INFO("BLE_MANAGER: The nRF52 doesn't have data to send (system is not measuring), so the command from phone was not necessary");
                            NRF_LOG_INFO("BLE_MANAGER: Write to STAT characteristic will not be stored in the device status data.");
                        }
                    }
                }
                //If the command of the STAT characteristic sent by the phone is the same as the device status command, then print log info:
                else
                {
                    NRF_LOG_INFO("BLE_MANAGER: The ble command sent by the phone is the same as the stored. commandFromPhone = %d", deviceStatus_getStructData_commandFromPhone());
                }

            }
            //If the STAT characteristic notification is disabled, then the STAT characteristic changed by the NordicSoftdevice is not valid:
            else
            {
                NRF_LOG_INFO("BLE_MANAGER: Write to STAT characteristic is not valid when notification is disabled.");

                //Update and notify the STAT characteristic (must be the status data stored):
                device_status_data dataCusStat = deviceStatus_getStructData();
                data_buffer[0] = dataCusStat.commandFromPhone;
                data_buffer[1] = dataCusStat.isMeasuring;
                data_buffer[2] = dataCusStat.isDataOnFlash;
                data_buffer[3] = dataCusStat.fileName_year;
                data_buffer[4] = dataCusStat.fileName_month;
                data_buffer[5] = dataCusStat.fileName_day;
                data_buffer[6] = dataCusStat.fileName_hrs;
                data_buffer[7] = dataCusStat.fileName_mins;
                data_buffer[8] = dataCusStat.fileName_secs;
                data_buffer[9]  = dataCusStat.timeDuration_hrs;
                data_buffer[10] = dataCusStat.timeDuration_mins;
                data_buffer[11] = dataCusStat.timeDuration_secs;
                data_buffer[12] = dataCusStat.tempReference;
                data_buffer[13] = dataCusStat.isSensDataOnFlash;
                data_buffer[14] = dataCusStat.isContDataOnFlash;
                data_buffer[15] = dataCusStat.isContDataOnFlash;
                err_code = sd_ble_gatts_value_set(p_cus_service->conn_handle, p_cus_service->custom_value_handles.value_handle, &gatts_value);
                APP_ERROR_CHECK(err_code);
                NRF_LOG_INFO("BLE_MANAGER: The STAT characteristic is the same as the device status data.");
            }

            break;

        default:
              // No implementation needed.
              break;
    }
}


/**@brief Function for handling the Custom Service Service events.
 *
 * @details This function will be called for all Custom Service events which are passed to
 *          the application.
 *
 * @param[in]   p_cus_service  Custom Service structure.
 * @param[in]   p_evt          Event received from the Custom Service.
 *
 */
static void on_cus_sens_evt(cus_sens_t * p_cus_service, cus_sens_evt_t * p_evt)
{
    uint32_t err_code;
    switch(p_evt->evt_type)
    {
        case CUS_SENS_EVT_NOTIFICATION_ENABLED:
            isCusSensNotificationEnabled = true;
            NRF_LOG_INFO("BLE_MANAGER: isCusSensNotificationEnabled: true");
            break;

        case CUS_SENS_EVT_NOTIFICATION_DISABLED:
            isCusSensNotificationEnabled = false;
            NRF_LOG_INFO("BLE_MANAGER: isCusSensNotificationEnabled: false");
            break;

        case CUS_SENS_EVT_CONNECTED :
            break;

        case CUS_SENS_EVT_DISCONNECTED:
            isCusSensNotificationEnabled = false;
            NRF_LOG_INFO("BLE_MANAGER: isCusSensNotificationEnabled: false");
            break;

        default:
              // No implementation needed.
              break;
    }
}

/**@brief Function for handling the Custom Service Service events.
 *
 * @details This function will be called for all Custom Service events which are passed to
 *          the application.
 *
 * @param[in]   p_cus_service  Custom Service structure.
 * @param[in]   p_evt          Event received from the Custom Service.
 *
 */
static void on_cus_cont_evt(cus_cont_t * p_cus_service, cus_cont_evt_t * p_evt)
{
    uint32_t err_code;
    switch(p_evt->evt_type)
    {
        case CUS_CONT_EVT_NOTIFICATION_ENABLED:
            isCusContNotificationEnabled = true;
            NRF_LOG_INFO("BLE_MANAGER: isCusContNotificationEnabled: true");
            break;

        case CUS_CONT_EVT_NOTIFICATION_DISABLED:
            isCusContNotificationEnabled = false;
            NRF_LOG_INFO("BLE_MANAGER: isCusContNotificationEnabled: false");
            break;

        case CUS_CONT_EVT_CONNECTED :
            break;

        case CUS_CONT_EVT_DISCONNECTED:
            isCusContNotificationEnabled = false;
            NRF_LOG_INFO("BLE_MANAGER: isCusContNotificationEnabled: false");
            break;

        default:
              // No implementation needed.
              break;
    }
}


/**@brief Function for handling the Custom Service Service events.
 *
 * @details This function will be called for all Custom Service events which are passed to
 *          the application.
 *
 * @param[in]   p_cus_service  Custom Service structure.
 * @param[in]   p_evt          Event received from the Custom Service.
 *
 */
static void on_cus_batt_evt(cus_batt_t * p_cus_service, cus_batt_evt_t * p_evt)
{
    uint32_t err_code;
    switch(p_evt->evt_type)
    {
        case CUS_BATT_EVT_NOTIFICATION_ENABLED:
            isCusBattNotificationEnabled = true;
            NRF_LOG_INFO("BLE_MANAGER: isCusBattNotificationEnabled: true");
            break;

        case CUS_BATT_EVT_NOTIFICATION_DISABLED:
            isCusBattNotificationEnabled = false;
            NRF_LOG_INFO("BLE_MANAGER: isCusBattNotificationEnabled: false");
            break;

        case CUS_BATT_EVT_CONNECTED :
            break;

        case CUS_BATT_EVT_DISCONNECTED:
            isCusBattNotificationEnabled = false;
            NRF_LOG_INFO("BLE_MANAGER: isCusBattNotificationEnabled: false");
            break;

        default:
              // No implementation needed.
              break;
    }
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
void gap_params_init(void)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    /* YOUR_JOB: Use an appearance value matching the application's use case.
       err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_);
       APP_ERROR_CHECK(err_code); */

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the GATT module.
 */
void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
 */
void advertising_init(void)
{
    ret_code_t             err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance      = true;
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.advdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;

    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}


/**@brief Function for initializing services that will be used by the application.
 */
void services_init(void)
{
    ret_code_t         err_code;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    /* YOUR_JOB: Add code to initialize the services used by the application.
       ble_xxs_init_t                     xxs_init;
       ble_yys_init_t                     yys_init;

       // Initialize XXX Service.
       memset(&xxs_init, 0, sizeof(xxs_init));

       xxs_init.evt_handler                = NULL;
       xxs_init.is_xxx_notify_supported    = true;
       xxs_init.ble_xx_initial_value.level = 100;

       err_code = ble_bas_init(&m_xxs, &xxs_init);
       APP_ERROR_CHECK(err_code);

       // Initialize YYY Service.
       memset(&yys_init, 0, sizeof(yys_init));
       yys_init.evt_handler                  = on_yys_evt;
       yys_init.ble_yy_initial_value.counter = 0;

       err_code = ble_yy_service_init(&yys_init, &yy_init);
       APP_ERROR_CHECK(err_code);
     */

    //////////////////////////////////////////////////////////////
    //CUS STAT service variable:
    cus_stat_init_t  cus_stat_init;

    //Initialize CUS Service init structure to zero.
    memset(&cus_stat_init, 0, sizeof(cus_stat_init));

    //Sets the write and read permissions to the characteristic value attribute to open, i.e. the peer is allowed to write/read the value without encrypting the link first.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cus_stat_init.custom_value_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cus_stat_init.custom_value_char_attr_md.write_perm);

    // Set the cus event handler
    cus_stat_init.evt_handler = on_cus_stat_evt;

    err_code = cus_stat_ble_init(&m_cus_stat, &cus_stat_init);
    APP_ERROR_CHECK(err_code);

    //////////////////////////////////////////////////////////////
    //CUS SENS service variable:
    cus_sens_init_t  cus_sens_init;

    //Initialize CUS Service init structure to zero.
    memset(&cus_sens_init, 0, sizeof(cus_sens_init));

    //The write and read permissions to the characteristic value attribute are disabled:
    //BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cus_sens_init.custom_value_char_attr_md.read_perm);
    //BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cus_sens_init.custom_value_char_attr_md.write_perm);

    // Set the cus event handler
    cus_sens_init.evt_handler = on_cus_sens_evt;

    err_code = cus_sens_ble_init(&m_cus_sens, &cus_sens_init);
    APP_ERROR_CHECK(err_code);

    //////////////////////////////////////////////////////////////
    //CUS CONT service variable:
    cus_cont_init_t  cus_cont_init;

    //Initialize CUS Service init structure to zero.
    memset(&cus_cont_init, 0, sizeof(cus_cont_init));

    // Set the cus event handler
    cus_cont_init.evt_handler = on_cus_cont_evt;

    err_code = cus_cont_ble_init(&m_cus_cont, &cus_cont_init);
    APP_ERROR_CHECK(err_code);

    //////////////////////////////////////////////////////////////
    //CUS BATT service variable:
    cus_batt_init_t  cus_batt_init;

    //Initialize CUS Service init structure to zero.
    memset(&cus_batt_init, 0, sizeof(cus_batt_init));

    // Set the cus event handler
    cus_batt_init.evt_handler = on_cus_batt_evt;

    err_code = cus_batt_ble_init(&m_cus_batt, &cus_batt_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Connection Parameters module.
 */
void conn_params_init(void)
{
    ret_code_t             err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting advertising.
 */
void advertising_start(void)
{
    ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}




/**@brief Function for checking notification status.
 */
bool bleGetCusStatNotificationFlag(void)
{
    return isCusStatNotificationEnabled;
}

/**@brief Function for checking notification status.
 */
bool bleGetCusSensNotificationFlag(void)
{
    return isCusSensNotificationEnabled;
}

/**@brief Function for checking notification status.
 */
bool bleGetCusContNotificationFlag(void)
{
    return isCusContNotificationEnabled;
}

/**@brief Function for checking notification status.
 */
bool bleGetCusBattNotificationFlag(void)
{
    return isCusBattNotificationEnabled;
}

/**@brief Function for sending ble data.
 */
void bleCusStatSendData(device_status_data data)
{
    uint8_t *ptrData = (uint8_t*) &data;
    uint32_t err_code = cus_stat_custom_value_update(&m_cus_stat, ptrData);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for sending ble data.
 */
void bleCusSensSendData(detection_system_single_data data)
{
    uint8_t *ptrData = (uint8_t*) &data;
    uint32_t err_code = cus_sens_custom_value_update(&m_cus_sens, ptrData);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for sending ble data.
 */
void bleCusContSendData(control_system_data data)
{
    uint8_t *ptrData = (uint8_t*) &data;
    uint32_t err_code = cus_cont_custom_value_update(&m_cus_cont, ptrData);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for sending ble data.
 */
void bleCusBattSendData(battery_system_data data)
{
    uint8_t *ptrData = (uint8_t*) &data;
    uint32_t err_code = cus_batt_custom_value_update(&m_cus_batt, ptrData);
    APP_ERROR_CHECK(err_code);
}