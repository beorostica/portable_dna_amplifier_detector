
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
    //TODO: check if the two following lines are necessary.
    //while(!m_xfer_done);
    //m_xfer_done = false;

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


/*
static bool writeExtendedData(uint8_t classID, uint8_t offset, uint8_t * data, uint8_t len);
static bool blockDataControl(void);
static bool blockDataClass(uint8_t id);
static bool blockDataOffset(uint8_t offset);
static uint8_t computeBlockChecksum(void);
static uint8_t blockDataChecksum(void);
static bool writeBlockData(uint8_t offset, uint8_t data);
static bool writeBlockChecksum(uint8_t csum);
static bool enterConfig(void);
static bool exitConfig(void);
static bool seal(void);
static bool sealed(void);
static uint16_t status(void);
static bool unseal(void);
static uint16_t flags(void);
static bool executeControlWord(uint16_t function);



// Configures the design capacity of the connected battery.
bool bq27441_setCapacity(uint16_t capacity)
{
    // Write to STATE subclass (82) of BQ27441 extended memory.
    // Offset 0x0A (10)
    // Design capacity is a 2-byte piece of data - MSB first
    // Unit: mAh
    uint8_t capMSB = capacity >> 8;
    uint8_t capLSB = capacity & 0x00FF;
    uint8_t capacityData[2] = {capMSB, capLSB};

    return writeExtendedData(BQ27441_ID_STATE, 10, capacityData, 2);
}

// Write a specified number of bytes to extended data specifying a class ID, position offset.
static bool writeExtendedData(uint8_t classID, uint8_t offset, uint8_t * data, uint8_t len)
{

	enterConfig();
	
	if (!blockDataControl()) // // enable block data memory control
		return false; // Return false if enable fails
	if (!blockDataClass(classID)) // Write class ID using DataBlockClass()
		return false;
	
	blockDataOffset(offset / 32); // Write 32-bit block offset (usually 0)
	computeBlockChecksum(); // Compute checksum going in
	uint8_t oldCsum = blockDataChecksum();

	// Write data bytes:
	for (int i = 0; i < len; i++)
	{
		// Write to offset, mod 32 if offset is greater than 32
		// The blockDataOffset above sets the 32-bit block
		writeBlockData((offset % 32) + i, data[i]);
	}
	
	// Write new checksum using BlockDataChecksum (0x60)
	uint8_t newCsum = computeBlockChecksum(); // Compute the new checksum
	writeBlockChecksum(newCsum);

	exitConfig();
	
	return true;
}

// Issue a BlockDataControl() command to enable BlockData access
static bool blockDataControl(void)
{
    ///////////////////////
    //// i2cWriteBytes ////
    ///////////////////////
    //Send: addr, sub_address, data_config[1], data_config[0]
    m_xfer_done = false;
    uint8_t sub_address = BQ27441_EXTENDED_CONTROL;
    uint8_t enableByte = 0x00;
    uint8_t m_bytes_to_send[2] = {sub_address, enableByte};
    ret_code_t err_code = nrf_drv_twi_tx(&m_twi, BQ72441_I2C_ADDRESS, m_bytes_to_send, sizeof(m_bytes_to_send), false);
    APP_ERROR_CHECK(err_code);
    while(!m_xfer_done);
    m_xfer_done = false;

    return true;
}

// Issue a DataClass() command to set the data class to be accessed
static bool blockDataClass(uint8_t id)
{
    ///////////////////////
    //// i2cWriteBytes ////
    ///////////////////////
    //Send: addr, sub_address, data_config[1], data_config[0]
    m_xfer_done = false;
    uint8_t sub_address = BQ27441_EXTENDED_DATACLASS;
    uint8_t idByte = id;
    uint8_t m_bytes_to_send[2] = {sub_address, idByte};
    ret_code_t err_code = nrf_drv_twi_tx(&m_twi, BQ72441_I2C_ADDRESS, m_bytes_to_send, sizeof(m_bytes_to_send), false);
    APP_ERROR_CHECK(err_code);
    while(!m_xfer_done);
    m_xfer_done = false;

    return true;
}

// Issue a DataBlock() command to set the data block to be accessed
static bool blockDataOffset(uint8_t offset)
{
    ///////////////////////
    //// i2cWriteBytes ////
    ///////////////////////
    //Send: addr, sub_address, data_config[1], data_config[0]
    m_xfer_done = false;
    uint8_t sub_address = BQ27441_EXTENDED_DATABLOCK;
    uint8_t offsetByte = offset;
    uint8_t m_bytes_to_send[2] = {sub_address, offsetByte};
    ret_code_t err_code = nrf_drv_twi_tx(&m_twi, BQ72441_I2C_ADDRESS, m_bytes_to_send, sizeof(m_bytes_to_send), false);
    APP_ERROR_CHECK(err_code);
    while(!m_xfer_done);
    m_xfer_done = false;

    return true;
}

// Read all 32 bytes of the loaded extended data and compute a checksum based on the values.
static uint8_t computeBlockChecksum(void)
{  
    ///////////////////////
    //// i2cReadBytes /////
    ///////////////////////
    //Send: addr, sub_address_read
    m_xfer_done = false;
    uint8_t sub_address = BQ27441_EXTENDED_BLOCKDATA;
    ret_code_t err_code = nrf_drv_twi_tx(&m_twi, BQ72441_I2C_ADDRESS, &sub_address, sizeof(sub_address), false);
    APP_ERROR_CHECK(err_code);
    while(!m_xfer_done);
    m_xfer_done = false;

    //Send: addr. Receive: data
    uint8_t data[32];
    err_code = nrf_drv_twi_rx(&m_twi, BQ72441_I2C_ADDRESS, data, sizeof(data));
    APP_ERROR_CHECK(err_code);
    while(!m_xfer_done);
    m_xfer_done = false;

    uint8_t csum = 0;
    for (int i=0; i<32; i++)
    {
        csum += data[i];
    }
    csum = 255 - csum;
	
    return csum;
}

// Read the current checksum using BlockDataCheckSum()
static uint8_t blockDataChecksum(void)
{
    ///////////////////////
    //// i2cReadBytes /////
    ///////////////////////
    //Send: addr, sub_address_read
    m_xfer_done = false;
    uint8_t sub_address = BQ27441_EXTENDED_CHECKSUM;
    ret_code_t err_code = nrf_drv_twi_tx(&m_twi, BQ72441_I2C_ADDRESS, &sub_address, sizeof(sub_address), false);
    APP_ERROR_CHECK(err_code);
    while(!m_xfer_done);
    m_xfer_done = false;

    //Send: addr. Receive: data
    uint8_t csum;
    err_code = nrf_drv_twi_rx(&m_twi, BQ72441_I2C_ADDRESS, &csum, sizeof(csum));
    APP_ERROR_CHECK(err_code);
    while(!m_xfer_done);
    m_xfer_done = false;
  
    return csum;
}

// Use BlockData() to write a byte to an offset of the loaded data
static bool writeBlockData(uint8_t offset, uint8_t data)
{
    ///////////////////////
    //// i2cWriteBytes ////
    ///////////////////////
    //Send: addr, sub_address, data_config[1], data_config[0]
    m_xfer_done = false;
    uint8_t sub_address = offset + BQ27441_EXTENDED_BLOCKDATA;;
    uint8_t dataByte = data;
    uint8_t m_bytes_to_send[2] = {sub_address, dataByte};
    ret_code_t err_code = nrf_drv_twi_tx(&m_twi, BQ72441_I2C_ADDRESS, m_bytes_to_send, sizeof(m_bytes_to_send), false);
    APP_ERROR_CHECK(err_code);
    while(!m_xfer_done);
    m_xfer_done = false;

    return true;
}

// Use the BlockDataCheckSum() command to write a checksum value
static bool writeBlockChecksum(uint8_t csum)
{
    ///////////////////////
    //// i2cWriteBytes ////
    ///////////////////////
    //Send: addr, sub_address, data_config[1], data_config[0]
    m_xfer_done = false;
    uint8_t sub_address = BQ27441_EXTENDED_CHECKSUM;
    uint8_t csumByte = csum;
    uint8_t m_bytes_to_send[2] = {sub_address, csumByte};
    ret_code_t err_code = nrf_drv_twi_tx(&m_twi, BQ72441_I2C_ADDRESS, m_bytes_to_send, sizeof(m_bytes_to_send), false);
    APP_ERROR_CHECK(err_code);
    while(!m_xfer_done);
    m_xfer_done = false;

    return true;
}


// Enter configuration mode - set userControl if calling from an Arduino sketch
// and you want control over when to exitConfig
static bool enterConfig(void)
{
    if (sealed())
    {
        unseal(); // Must be unsealed before making changes
    }
	
    if (executeControlWord(BQ27441_CONTROL_SET_CFGUPDATE))
    {
        int16_t timeout = BQ72441_I2C_TIMEOUT;
	while ((timeout--) && (!(flags() & BQ27441_FLAG_CFGUPMODE)))
        {
            //delay(1);
            NRF_LOG_INFO("wait timeout: %d.", timeout);
        }
		
	if (timeout > 0)
            return true;
    }
	
    return false;
}

// Exit configuration mode with the option to perform a resimulation
static bool exitConfig(void)
{
    return executeControlWord(BQ27441_CONTROL_EXIT_CFGUPDATE);
}

// Seal the BQ27441-G1A
static bool seal(void)
{
    return readControlWord(BQ27441_CONTROL_SEALED);
}

// Check if the BQ27441-G1A is sealed or not.
static bool sealed(void)
{
    uint16_t stat = status();
    return stat & BQ27441_STATUS_SS;
}

// Read the CONTROL_STATUS subcommand of control()
static uint16_t status(void)
{
    return readControlWord(BQ27441_CONTROL_STATUS);
}

// UNseal the BQ27441-G1A
static bool unseal(void)
{
    // To unseal the BQ27441, write the key to the control
    // command. Then immediately write the same key to control again.
    if (readControlWord(BQ27441_UNSEAL_KEY))
    {
        return readControlWord(BQ27441_UNSEAL_KEY);
    }
    return false;
}

// Read the flags() command
static uint16_t flags(void)
{
    return readWord(BQ27441_COMMAND_FLAGS);
}

// Execute a subcommand() from the BQ27441-G1A's control()
static bool executeControlWord(uint16_t function)
{
    ///////////////////////
    //// i2cWriteBytes ////
    ///////////////////////
    //Send: addr, sub_address, data_config[1], data_config[0]
    m_xfer_done = false;
    uint8_t sub_address = 0x00;
    uint8_t sub_command_lsb = (function & 0x00FF);
    uint8_t sub_command_msb = (function >> 8);
    uint8_t m_bytes_to_send[3] = {sub_address, sub_command_lsb, sub_command_msb};
    ret_code_t err_code = nrf_drv_twi_tx(&m_twi, BQ72441_I2C_ADDRESS, m_bytes_to_send, sizeof(m_bytes_to_send), false);
    APP_ERROR_CHECK(err_code);
    while(!m_xfer_done);
    m_xfer_done = false;

    return true;
}
*/


/*****************************************************************************
 ************************ I2C Read and Write Routines ************************
 *****************************************************************************/
/*
// Write a specified number of bytes over I2C to a given subAddress
uint16_t i2cWriteBytes(uint8_t subAddress, uint8_t * src, uint8_t count)
{
    Wire.beginTransmission(_deviceAddress);
    Wire.write(subAddress);
    for (int i=0; i<count; i++)
    {
        Wire.write(src[i]);
    }	
    Wire.endTransmission(true);
	
    return true;	
}

// Read a specified number of bytes over I2C at a given subAddress
int16_t i2cReadBytes(uint8_t subAddress, uint8_t * dest, uint8_t count)
{
    int16_t timeout = BQ72441_I2C_TIMEOUT;	
    Wire.beginTransmission(_deviceAddress);
    Wire.write(subAddress);
    Wire.endTransmission(true);
	
    Wire.requestFrom(_deviceAddress, count);
	
    for (int i=0; i<count; i++)
    {
        dest[i] = Wire.read();
    }
	
    return timeout;
}
*/