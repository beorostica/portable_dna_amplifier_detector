
#include "custom_twi.h"
#include "custom_ads1015.h"

#include "boards.h"
#include "nrf_drv_twi.h"


// Indicates if operation on TWI has ended.
static volatile bool m_xfer_done = false;

// TWI instance.
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(0);

/**
 * @brief TWI events handler.
 */
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    switch (p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
            m_xfer_done = true;
            break;
        default:
            break;
    }
}

/**
 * @brief TWI initialization.
 */
void twiInit(void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_config = {
       .scl                = ARDUINO_SCL_PIN,
       .sda                = ARDUINO_SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

/**
 * @brief ADS1015 read function.
 */
uint16_t ads1015Read(uint8_t channel)
{
    //Classic error variable for debbugging:
    ret_code_t err_code;
    
    // Start with default values
    uint16_t config =
        ADS1015_REG_CONFIG_CQUE_NONE |    // Disable the comparator (default val)
        ADS1015_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
        ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
        ADS1015_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
        ADS1015_REG_CONFIG_DR_1600SPS |   // 1600 samples per second (default)
        ADS1015_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)
    
    // Set PGA/voltage range
    adsGain_t m_gain = GAIN_ONE;
    config |= m_gain;
    
    // Set single-ended input channel
    switch (channel) {
      case (0):
          config |= ADS1015_REG_CONFIG_MUX_SINGLE_0;
          break;
      case (1):
          config |= ADS1015_REG_CONFIG_MUX_SINGLE_1;
          break;
      case (2):
          config |= ADS1015_REG_CONFIG_MUX_SINGLE_2;
          break;
      case (3):
          config |= ADS1015_REG_CONFIG_MUX_SINGLE_3;
          break;
    }
    
    //Set 'start single-conversion' bit
    config |= ADS1015_REG_CONFIG_OS_SINGLE;

    ///////////////////////////////////////////////////////////////
    /// Write configuration to ADC1015 ////////////////////////////
    ///////////////////////////////////////////////////////////////

    //Send: addr, reg_pointer_read, data_config[1], data_config[0]
    m_xfer_done = false;
    uint8_t reg_pointer_write = ADS1015_REG_POINTER_CONFIG;
    uint8_t data_config_1 = (config >> 8);
    uint8_t data_config_0 = (config & 0xFF);
    uint8_t m_bytes_to_send[3] = {reg_pointer_write, data_config_1, data_config_0};
    err_code = nrf_drv_twi_tx(&m_twi, ADS1015_ADDRESS, m_bytes_to_send, sizeof(m_bytes_to_send), false);
    APP_ERROR_CHECK(err_code);
    while(!m_xfer_done);
    m_xfer_done = false;

    ///////////////////////////////////////////////////////////////
    /// Read data from ADC1015 ////////////////////////////////////
    ///////////////////////////////////////////////////////////////
    
    //Send: addr, register_pointer_read
    uint8_t register_pointer_read = ADS1015_REG_POINTER_CONVERT;
    err_code = nrf_drv_twi_tx(&m_twi, ADS1015_ADDRESS, &register_pointer_read, sizeof(register_pointer_read), false);
    APP_ERROR_CHECK(err_code);
    while(!m_xfer_done);
    m_xfer_done = false;

    //Send: addr. Receive: data[1], data[0]
    static uint8_t m_data[2];
    err_code = nrf_drv_twi_rx(&m_twi, ADS1015_ADDRESS, m_data, sizeof(m_data));
    APP_ERROR_CHECK(err_code);

    ///////////////////////////////////////////////////////////////
    /// Value interpretation of ADC1015 data //////////////////////
    ///////////////////////////////////////////////////////////////

    uint16_t adcValue = (m_data[0] << 4)|(m_data[1] >> 4);
    return adcValue;

}