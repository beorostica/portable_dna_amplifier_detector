
#ifndef CUS_CONT_H
#define CUS_CONT_H


#include <stdint.h>
#include "ble_srv_common.h"

#include "custom_control_system_struct_data.h"


#define CUSTOM_CONT_SERVICE_UUID_BASE         {0xCE, 0x9B, 0xC0, 0x56, 0xDB, 0x16, 0x61, 0xCB, \
                                               0x51, 0x53, 0xC1, 0x11, 0xDA, 0xBE, 0x75, 0x04}

#define CUSTOM_CONT_SERVICE_UUID               0x1400
#define CUSTOM_CONT_VALUE_CHAR_UUID            0x1401


/**@brief   Macro for defining a cus_cont instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_CUS_CONT_DEF(_name)                                                                    \
static cus_cont_t _name;                                                                           \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                \
                     BLE_HRS_BLE_OBSERVER_PRIO,                                                    \
                     cus_cont_on_ble_evt, &_name)


/**@brief Forward declaration of the cus_cont_t type.*/
typedef struct cus_cont_s cus_cont_t;

/**@brief Custom Service event type. */
typedef enum
{
    CUS_CONT_EVT_NOTIFICATION_ENABLED,                             /**< Custom value notification enabled event. */
    CUS_CONT_EVT_NOTIFICATION_DISABLED,                            /**< Custom value notification disabled event. */
    CUS_CONT_EVT_DISCONNECTED,
    CUS_CONT_EVT_CONNECTED
} cus_cont_evt_type_t;

/**@brief Custom Service event. */
typedef struct
{
    cus_cont_evt_type_t evt_type;                                  /**< Type of event. */
} cus_cont_evt_t;

/**@brief Custom Service event handler type. */
typedef void (*cus_cont_evt_handler_t) (cus_cont_t * p_cus, cus_cont_evt_t * p_evt);

/**@brief Custom Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    cus_cont_evt_handler_t        evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
    uint8_t                       initial_custom_value[sizeof(control_system_data)];         /**< Initial custom value */
    ble_srv_cccd_security_mode_t  custom_value_char_attr_md;     /**< Initial security level for Custom characteristics attribute */
} cus_cont_init_t;

/**@brief Custom Service structure. This contains various status information for the service. */
struct cus_cont_s
{
    cus_cont_evt_handler_t        evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
    uint16_t                      service_handle;                 /**< Handle of Custom Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      custom_value_handles;           /**< Handles related to the Custom Value characteristic. */
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
uint32_t cus_cont_ble_init(cus_cont_t * p_cus, const cus_cont_init_t * p_cus_init);


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Battery Service.
 *
 * @note 
 *
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 * @param[in]   p_context  Custom Service structure.
 */
void cus_cont_on_ble_evt( ble_evt_t const * p_ble_evt, void * p_context);


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
uint32_t cus_cont_custom_value_update(cus_cont_t * p_cus, uint8_t * custom_value);


#endif /* CUS_CONT_H */