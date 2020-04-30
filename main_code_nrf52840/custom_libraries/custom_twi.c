
#include "custom_twi.h"
#include "custom_ads1015.h"
#include "custom_bq27441.h"

#include "boards.h"
#include "nrf_drv_twi.h"
#include "custom_log.h"


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


////////////////////////////////////////////////////////////////////////////////////////
/// For ADS1015 (4 channel adc + pga) //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

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
    while(!m_xfer_done);
    m_xfer_done = false;

    ///////////////////////////////////////////////////////////////
    /// Value interpretation of ADC1015 data //////////////////////
    ///////////////////////////////////////////////////////////////

    uint16_t adcValue = (m_data[0] << 4)|(m_data[1] >> 4);
    return adcValue;

}


////////////////////////////////////////////////////////////////////////////////////////
/// For BQ27441 (fuel gauge, battery babysitter) ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

static uint16_t deviceType(void);
static uint16_t readControlWord(uint16_t function);
static uint16_t readWord(uint16_t subAddress);


bool bq27441_begin(void)
{
    uint16_t deviceID = 0;
    deviceID = deviceType(); // Read deviceType from BQ27441
    NRF_LOG_INFO("deviceID: %d.", deviceID);

    if (deviceID == BQ27441_DEVICE_ID)
    {
        return true; // If device ID is valid, return true
    }
    return false; // Otherwise return false

}

// Read the device type - should be 0x0421
static uint16_t deviceType(void)
{
    return readControlWord(BQ27441_CONTROL_DEVICE_TYPE);
}

// Read a 16-bit subcommand() from the BQ27441-G1A's control()
static uint16_t readControlWord(uint16_t function)
{
    //Classic error variable for debbugging:
    ret_code_t err_code;

    ///////////////////////
    //// i2cWriteBytes ////
    ///////////////////////
    //Send: addr, sub_address, data_config[1], data_config[0]
    m_xfer_done = false;
    uint8_t sub_address = 0x00;
    uint8_t sub_command_lsb = (function & 0x00FF);
    uint8_t sub_command_msb = (function >> 8);
    uint8_t m_bytes_to_send[3] = {sub_address, sub_command_lsb, sub_command_msb};
    err_code = nrf_drv_twi_tx(&m_twi, BQ72441_I2C_ADDRESS, m_bytes_to_send, sizeof(m_bytes_to_send), false);
    APP_ERROR_CHECK(err_code);
    while(!m_xfer_done);
    m_xfer_done = false;

    ///////////////////////
    //// i2cReadBytes /////
    ///////////////////////
    //Send: addr, sub_address_read
    uint8_t sub_address_read = 0x00;
    err_code = nrf_drv_twi_tx(&m_twi, BQ72441_I2C_ADDRESS, &sub_address_read, sizeof(sub_address_read), false);
    APP_ERROR_CHECK(err_code);
    while(!m_xfer_done);
    m_xfer_done = false;

    //Send: addr. Receive: data[1], data[0]
    uint8_t rx_data[2];
    err_code = nrf_drv_twi_rx(&m_twi, BQ72441_I2C_ADDRESS, rx_data, sizeof(rx_data));
    APP_ERROR_CHECK(err_code);
    while(!m_xfer_done);
    m_xfer_done = false;

    return (((uint16_t)rx_data[1] << 8) | rx_data[0]);
    
}


// Reads and returns specified state of charge measurement
uint16_t bq27441_getSoc(void)
{
    return readWord(BQ27441_COMMAND_SOC);
}

// Reads and returns the battery voltage
uint16_t bq27441_getVoltage(void)
{
    return readWord(BQ27441_COMMAND_VOLTAGE);
}

// Reads and returns the specified current measurement
int16_t bq27441_getCurrent(void)
{
    return (int16_t) readWord(BQ27441_COMMAND_AVG_CURRENT);
}

// Reads and returns the specified capacity measurement
uint16_t bq27441_getCapacityRemain(void)
{
    return readWord(BQ27441_COMMAND_REM_CAPACITY);
}

// Reads and returns the specified capacity measurement
uint16_t bq27441_getCapacityFull(void)
{
    return readWord(BQ27441_COMMAND_FULL_CAPACITY);
}

// Reads and returns measured average power
int16_t bq27441_getPower(void)
{
    return (int16_t) readWord(BQ27441_COMMAND_AVG_POWER);
}

// Reads and returns specified state of health measurement
uint8_t bq27441_getSoh(void)
{
    uint16_t sohRaw = readWord(BQ27441_COMMAND_SOH);
    uint8_t sohStatus = sohRaw >> 8;
    uint8_t sohPercent = sohRaw & 0x00FF;
    return sohPercent;
}


// Read a 16-bit command word from the BQ27441-G1A
static uint16_t readWord(uint16_t subAddress)
{
    ///////////////////////
    //// i2cReadBytes /////
    ///////////////////////
    //Send: addr, sub_address
    uint8_t sub_address = subAddress;
    ret_code_t err_code = nrf_drv_twi_tx(&m_twi, BQ72441_I2C_ADDRESS, &sub_address, sizeof(sub_address), false);
    APP_ERROR_CHECK(err_code);
    while(!m_xfer_done);
    m_xfer_done = false;

    //Send: addr. Receive: data[1], data[0]
    uint8_t rx_data[2];
    err_code = nrf_drv_twi_rx(&m_twi, BQ72441_I2C_ADDRESS, rx_data, sizeof(rx_data));
    APP_ERROR_CHECK(err_code);
    while(!m_xfer_done);
    m_xfer_done = false;

    return ((uint16_t) rx_data[1] << 8) | rx_data[0];
}
