
#include "cus_batt.h"

#include "custom_log.h"


static uint32_t custom_value_char_add(cus_batt_t * p_cus, const cus_batt_init_t * p_cus_init);
static void on_connect(cus_batt_t * p_cus, ble_evt_t const * p_ble_evt);
static void on_disconnect(cus_batt_t * p_cus, ble_evt_t const * p_ble_evt);
static void on_write(cus_batt_t * p_cus, ble_evt_t const * p_ble_evt);
static void on_tx_complete(cus_batt_t * p_cus);


/**@brief Function for adding the Custom Value characteristic.
 *
 * @param[in]   p_cus        Custom Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t custom_value_char_add(cus_batt_t * p_cus, const cus_batt_init_t * p_cus_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    ///////////////////////////////////////////////////////////////////////////
    /// For Notification //////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on Cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;
    ///////////////////////////////////////////////////////////////////////////

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 0;
    char_md.char_props.write  = 0;
    char_md.char_props.notify = 1; 
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; 
    char_md.p_sccd_md         = NULL;
		
    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = CUSTOM_BATT_VALUE_CHAR_UUID;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(battery_system_data);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(battery_system_data);

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->custom_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}


/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(cus_batt_t * p_cus, ble_evt_t const * p_ble_evt)
{
    p_cus->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

    //We can now invoke this event handler:
    cus_batt_evt_t evt;
    evt.evt_type = CUS_BATT_EVT_CONNECTED;
    p_cus->evt_handler(p_cus, &evt);
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(cus_batt_t * p_cus, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_cus->conn_handle = BLE_CONN_HANDLE_INVALID;
}


/**@brief Function for handling the Write event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(cus_batt_t * p_cus, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    // Custom Value Characteristic Written to.
    if (p_evt_write->handle == p_cus->custom_value_handles.value_handle)
    {
        // Put specific task here (toogle LED_4).
        //nrf_gpio_pin_toggle(LED_4);
    }

    // Check if the Custom value CCCD is written to and that the value is the appropriate length, i.e 2 bytes.
    if ((p_evt_write->handle == p_cus->custom_value_handles.cccd_handle) && (p_evt_write->len == 2))
    {
        // CCCD written, call application event handler
        if (p_cus->evt_handler != NULL)
        {
            cus_batt_evt_t evt;

            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                evt.evt_type = CUS_BATT_EVT_NOTIFICATION_ENABLED;
            }
            else
            {
                evt.evt_type = CUS_BATT_EVT_NOTIFICATION_DISABLED;
            }
            // Call the application event handler.
            p_cus->evt_handler(p_cus, &evt);
        }
    }
}


/**@brief Function for handling the TX_COMPLETE event.
 *
 * @param   p_cus   Custom Service structure.
 */
static void on_tx_complete(cus_batt_t * p_cus)
{
    if (p_cus->busy)
    {
        p_cus->busy = false;
    }
}


/**@brief Function for initializing the Custom Service.
 *
 * @param[out]  p_cus       Custom Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_cus_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t cus_batt_ble_init(cus_batt_t * p_cus, const cus_batt_init_t * p_cus_init)
{
    if (p_cus == NULL || p_cus_init == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_cus->evt_handler               = p_cus_init->evt_handler;
    p_cus->conn_handle               = BLE_CONN_HANDLE_INVALID;

    // Add Custom Service UUID
    ble_uuid128_t base_uuid = {CUSTOM_BATT_SERVICE_UUID_BASE};
    err_code =  sd_ble_uuid_vs_add(&base_uuid, &p_cus->uuid_type);
    VERIFY_SUCCESS(err_code);
    
    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = CUSTOM_BATT_SERVICE_UUID;

    // Add the Custom Service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_cus->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Custom Value characteristic
    return custom_value_char_add(p_cus, p_cus_init);
}

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Battery Service.
 *
 * @note 
 *
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 * @param[in]   p_context  Custom Service structure.
 */
void cus_batt_on_ble_evt( ble_evt_t const * p_ble_evt, void * p_context)
{
   cus_batt_t * p_cus = (cus_batt_t *) p_context;

   if (p_cus == NULL || p_ble_evt == NULL)
   {
       return;
   }
   
   switch (p_ble_evt->header.evt_id)
   {
       case BLE_GAP_EVT_CONNECTED:
           on_connect(p_cus, p_ble_evt);
           break;

       case BLE_GAP_EVT_DISCONNECTED:
           on_disconnect(p_cus, p_ble_evt);
           break;

       case BLE_GATTS_EVT_WRITE:
           on_write(p_cus, p_ble_evt);
           break;

       case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            on_tx_complete(p_cus);
            break;

       default:
           // No implementation needed.
           break;
   }
}


/**@brief Function for updating the custom value.
 *
 * @details The application calls this function when the cutom value should be updated. If
 *          notification has been enabled, the custom value characteristic is sent to the client.
 *
 * @note 
 *       
 * @param[in]   p_cus          Custom Service structure.
 * @param[in]   Custom value 
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t cus_batt_custom_value_update(cus_batt_t * p_cus, uint8_t * custom_value)
{
     
    if (p_cus == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code = NRF_SUCCESS;


    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(battery_system_data);
    gatts_value.offset  = 0;
    gatts_value.p_value = custom_value;

    // Update database.
    err_code = sd_ble_gatts_value_set(p_cus->conn_handle,
                                      p_cus->custom_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if (p_cus->conn_handle != BLE_CONN_HANDLE_INVALID) 
    {
        ble_gatts_hvx_params_t hvx_params;
    
        memset(&hvx_params, 0, sizeof(hvx_params));
    
        hvx_params.handle = p_cus->custom_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;
    
        //Try to send data once:
        err_code = sd_ble_gatts_hvx(p_cus->conn_handle, &hvx_params);
    
        //If the "nrf resources buffer" is busy, we will say its success but busy:
        if (err_code == NRF_ERROR_RESOURCES){
    
            err_code = NRF_SUCCESS;
            p_cus->busy = true;
            NRF_LOG_INFO("BLE BATT SERVICE: sd_ble_gatts_hvx BUSY, waiting ...");
    
            //We wait for BLE_GATTS_EVT_HVN_TX_COMPLETE:
            while(p_cus->busy);
    
            //Send again:
            err_code = sd_ble_gatts_hvx(p_cus->conn_handle, &hvx_params);
            NRF_LOG_INFO("BLE BATT SERVICE: sd_ble_gatts_hvx FREE, send again.");
            
        }   
             
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}
