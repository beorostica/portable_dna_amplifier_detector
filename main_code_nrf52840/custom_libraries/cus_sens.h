
#ifndef CUS_SENS_H
#define CUS_SENS_H


#include <stdint.h>
#include "ble_srv_common.h"

#include "custom_detection_system_struct_data.h"


#define CUSTOM_SENS_SERVICE_UUID_BASE         {0xBD, 0x8A, 0xBF, 0x45, 0xCA, 0x05, 0x50, 0xBA, \
                                               0x40, 0x42, 0xB0, 0x00, 0xC9, 0xAD, 0x64, 0xF3}

#define CUSTOM_SENS_SERVICE_UUID               0x1400
#define CUSTOM_SENS_VALUE_CHAR_UUID            0x1401


/**@brief   Macro for defining a cus_sens instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_CUS_SENS_DEF(_name)                                                                    \
static cus_sens_t _name;                                                                           \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                \
                     BLE_HRS_BLE_OBSERVER_PRIO,                                                    \
                     cus_sens_on_ble_evt, &_name)


/**@brief Forward declaration of the cus_sens_t type.*/
typedef struct cus_sens_s cus_sens_t;

/**@brief Custom Service event type. */
typedef enum
{
    CUS_SENS_EVT_NOTIFICATION_ENABLED,                             /**< Custom value notification enabled event. */
    CUS_SENS_EVT_NOTIFICATION_DISABLED,                            /**< Custom value notification disabled event. */
    CUS_SENS_EVT_DISCONNECTED,
    CUS_SENS_EVT_CONNECTED
} cus_sens_evt_type_t;

/**@brief Custom Service event. */
typedef struct
{
    cus_sens_evt_type_t evt_type;                                  /**< Type of event. */
} cus_sens_evt_t;

/**@brief Custom Service event handler type. */
typedef void (*cus_sens_evt_handler_t) (cus_sens_t * p_cus, cus_sens_evt_t * p_evt);

/**@brief Custom Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    cus_sens_evt_handler_t        evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
    uint8_t                       initial_custom_value[sizeof(detection_system_single_data)];         /**< Initial custom value */
    ble_srv_cccd_security_mode_t  custom_value_char_attr_md;     /**< Initial security level for Custom characteristics attribute */
} cus_sens_init_t;

/**@brief Custom Service structure. This contains various status information for the service. */
struct cus_sens_s
{
    cus_sens_evt_handler_t        evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
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
uint32_t cus_sens_ble_init(cus_sens_t * p_cus, const cus_sens_init_t * p_cus_init);


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Battery Service.
 *
 * @note 
 *
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 * @param[in]   p_context  Custom Service structure.
 */
void cus_sens_on_ble_evt( ble_evt_t const * p_ble_evt, void * p_context);


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
uint32_t cus_sens_custom_value_update(cus_sens_t * p_cus, uint8_t * custom_value);


#endif /* CUS_SENS_H */