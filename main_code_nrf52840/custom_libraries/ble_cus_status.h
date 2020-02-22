
#ifndef BLE_CUS_STATUS_H
#define BLE_CUS_STATUS_H


#include <stdint.h>
#include "ble_srv_common.h"

#include "custom_detection_system_struct_data.h"


#define CUSTOM_SERVICE_STATUS_UUID_BASE         {0xAB, 0x79, 0xAE, 0x34, 0xB9, 0xF4, 0x4F, 0xA9, \
                                                 0x3F, 0x31, 0xAF, 0xFF, 0xB8, 0x9C, 0x53, 0xE2}

#define CUSTOM_SERVICE_STATUS_UUID               0x1400
#define CUSTOM_STATUS_VALUE_CHAR_UUID            0x1401


/**@brief   Macro for defining a ble_cus instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_CUS_STATUS_DEF(_name)                                                                   \
static ble_cus_status_t _name;                                                                      \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_HRS_BLE_OBSERVER_PRIO,                                                     \
                     ble_cus_status_on_ble_evt, &_name)


/**@brief Forward declaration of the ble_cus_status_t type.*/
typedef struct ble_cus_status_s ble_cus_status_t;



/**@brief Custom Service event type. */
typedef enum
{
    BLE_CUS_STATUS_EVT_NOTIFICATION_ENABLED,                             /**< Custom value notification enabled event. */
    BLE_CUS_STATUS_EVT_NOTIFICATION_DISABLED,                            /**< Custom value notification disabled event. */
    BLE_CUS_STATUS_EVT_DISCONNECTED,
    BLE_CUS_STATUS_EVT_CONNECTED
} ble_cus_status_evt_type_t;

/**@brief Custom Service event. */
typedef struct
{
    ble_cus_status_evt_type_t evt_type;                                  /**< Type of event. */
} ble_cus_status_evt_t;

/**@brief Custom Service event handler type. */
typedef void (*ble_cus_status_evt_handler_t) (ble_cus_status_t * p_cus, ble_cus_status_evt_t * p_evt);



/**@brief Custom Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_cus_status_evt_handler_t  evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
    uint8_t                       initial_custom_status_value[sizeof(detection_system_single_data)];         /**< Initial custom value */
    ble_srv_cccd_security_mode_t  custom_status_value_char_attr_md;     /**< Initial security level for Custom characteristics attribute */
} ble_cus_status_init_t;

/**@brief Custom Service structure. This contains various status information for the service. */
struct ble_cus_status_s
{
    ble_cus_status_evt_handler_t  evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
    uint16_t                      service_handle;                 /**< Handle of Custom Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      custom_status_value_handles;    /**< Handles related to the Custom Value characteristic. */
    uint16_t                      conn_handle;                    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint8_t                       uuid_type;
    bool                          busy;                           //!< Busy flag. Indicates that the hvx function returned busy and that there is still data to be transfered. */
};




/**@brief Function for initializing the Custom Service.
 *
 * @param[out]  p_cus       Custom Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_cus_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_cus_status_init(ble_cus_status_t * p_cus, const ble_cus_status_init_t * p_cus_init);


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Battery Service.
 *
 * @note 
 *
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 * @param[in]   p_context  Custom Service structure.
 */
void ble_cus_status_on_ble_evt( ble_evt_t const * p_ble_evt, void * p_context);


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
uint32_t ble_cus_status_custom_value_update(ble_cus_status_t * p_cus, uint8_t * custom_value);


#endif /* BLE_CUS_STATUS_H */