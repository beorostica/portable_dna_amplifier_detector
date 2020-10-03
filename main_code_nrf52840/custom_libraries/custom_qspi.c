
#include "custom_qspi.h"

#include "boards.h"
#include "nrf_drv_qspi.h"
#include "custom_ble_manager.h"
#include "custom_timer.h"

#include "custom_log.h"


#define QSPI_STD_CMD_WRSR   0x01
#define QSPI_STD_CMD_RSTEN  0x66
#define QSPI_STD_CMD_RST    0x99

#define QSPI_SECTOR_SIZE_BYTES      4096                     //User can erase a sector of 4096 Bytes
#define QSPI_NUMBER_BLOCKS          16                       //16 Blocks of 256 Bytes = Store 4096 Bytes ~ 4KB
#define QSPI_BLOCK_SIZE_BYTES       256                      //256 Bytes is the size of a block in memory
#define QSPI_BUFFER_SIZE_UINT8      QSPI_BLOCK_SIZE_BYTES/1  //256 elements in a buffer of uint8_t  (1 Bytes)
#define QSPI_BUFFER_SIZE_UINT16     QSPI_BLOCK_SIZE_BYTES/2  //128 elements in a buffer of uint16_t (2 Bytes)
#define QSPI_BUFFER_SIZE_UINT32     QSPI_BLOCK_SIZE_BYTES/4  // 64 elements in a buffer of uint32_t (4 Bytes)


#define QSPI_NUMBER_SECTORS_DETSYS  3                        //3 Sectors of 4096 Bytes (To write: I can erase sectors, not blocks)
#define QSPI_BUFFER_SIZE_DETSYSDAT  QSPI_BLOCK_SIZE_BYTES/32 //  8 elements in a buffer of detection_system_data (32 Bytes)
#define QSPI_OFFSET_DETSYS          0                        //Offset

#define QSPI_NUMBER_SECTORS_CONSYS  2                        //2 Sectors of 4096 Bytes ~ 4k Bytes
#define QSPI_BUFFER_SIZE_CONSYSDAT  QSPI_BLOCK_SIZE_BYTES/16  // 16 elements in a buffer of control_system_data (16 Bytes)
#define QSPI_OFFSET_CONSYS          QSPI_OFFSET_DETSYS + (QSPI_NUMBER_SECTORS_DETSYS*QSPI_SECTOR_SIZE_BYTES) //Offset

#define QSPI_NUMBER_SECTORS_BATSYS  2                        //2 Sectors of 4096 Bytes ~ 4k Bytes
#define QSPI_BUFFER_SIZE_BATSYSDAT  QSPI_BLOCK_SIZE_BYTES/16 // 16 elements in a buffer of battery_system_data (16 Bytes)
#define QSPI_OFFSET_BATSYS          QSPI_OFFSET_CONSYS + (QSPI_NUMBER_SECTORS_CONSYS*QSPI_SECTOR_SIZE_BYTES) //Offset


#define WAIT_FOR_PERIPH() do { \
        while (!m_finished) {} \
        m_finished = false;    \
    } while (0)



static volatile bool m_finished = false;

static void qspi_handler(nrf_drv_qspi_evt_t event, void * p_context)
{
    UNUSED_PARAMETER(event);
    UNUSED_PARAMETER(p_context);
    m_finished = true;
}

static void configure_memory(void)
{
    /* 
    // For Nordic nRF52840 DK:
    ret_code_t err_code;
    
    uint8_t temporary = 0x40;
    nrf_qspi_cinstr_conf_t cinstr_cfg = {
        .opcode    = QSPI_STD_CMD_RSTEN,
        .length    = NRF_QSPI_CINSTR_LEN_1B,
        .io2_level = true,
        .io3_level = true,
        .wipwait   = true,
        .wren      = true
    };

    // Send reset enable
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    // Send reset command
    cinstr_cfg.opcode = QSPI_STD_CMD_RST;
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    // Switch to qspi mode
    cinstr_cfg.opcode = QSPI_STD_CMD_WRSR;
    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_2B;
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, &temporary, NULL);
    APP_ERROR_CHECK(err_code);
    */

    // For Feather nRF52840:
    uint32_t err_code;
    nrf_qspi_cinstr_conf_t cinstr_cfg = {
        .opcode    = QSPI_STD_CMD_RSTEN,
        .length    = NRF_QSPI_CINSTR_LEN_1B,
        .io2_level = true,
        .io3_level = true,
        .wipwait   = true,
        .wren      = true
    };

    // Send reset enable
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    // Send reset command
    cinstr_cfg.opcode = QSPI_STD_CMD_RST;
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
    APP_ERROR_CHECK(err_code);    

    // Switch to qspi mode
    uint8_t full_status[2] = {0x00, 0x02};
    cinstr_cfg.opcode = QSPI_STD_CMD_WRSR;
    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_3B;
    err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, full_status, NULL);
    APP_ERROR_CHECK(err_code);

}


void qspiInit(void)
{
    ret_code_t err_code;

    nrf_drv_qspi_config_t config = NRF_DRV_QSPI_DEFAULT_CONFIG;
    
    /////////////////////////////////////////////////////////////////
    // For Feather nRF52850:
    // Set QSPI pins to pins related to connected board.
    config.pins.sck_pin = NRF_GPIO_PIN_MAP(0,19);
    config.pins.csn_pin = NRF_GPIO_PIN_MAP(0,20);
    config.pins.io0_pin = NRF_GPIO_PIN_MAP(0,17);
    config.pins.io1_pin = NRF_GPIO_PIN_MAP(0,22);
    config.pins.io2_pin = NRF_GPIO_PIN_MAP(0,23);
    config.pins.io3_pin = NRF_GPIO_PIN_MAP(0,21);
    // More configurations:
    config.prot_if.readoc = NRF_QSPI_READOC_READ4O;
    config.prot_if.writeoc = NRF_QSPI_WRITEOC_PP4O;
    config.phy_if.sck_delay = 10;
    /////////////////////////////////////////////////////////////////

    err_code = nrf_drv_qspi_init(&config, qspi_handler, NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("QSPI started.");

    configure_memory();

    m_finished = false;

    //Erase NUMBER_PIECES pices of 65536 bytes ~ 64k bytes: 
    uint8_t indexPiece;
    uint8_t NUMBER_PIECES = 1;
    uint32_t PIECE_SIZE_BYTES = 65536;
    for(indexPiece = 0; indexPiece < NUMBER_PIECES; indexPiece++){
        uint32_t flashAddress = (indexPiece*PIECE_SIZE_BYTES) + QSPI_OFFSET_DETSYS;
        err_code = nrf_drv_qspi_erase(NRF_QSPI_ERASE_LEN_64KB, flashAddress);
        APP_ERROR_CHECK(err_code);
        WAIT_FOR_PERIPH();
        NRF_LOG_INFO("External Flash Erased. indexPiece: %d. flashAddress = %d.", indexPiece, flashAddress);
    }

}


///////////////////////////////////////////////////////////////////////////////////////////
/// For Detection System //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

//Declare static variables:
static uint32_t indexBuffer_write = 0;
static uint32_t indexBlock_write = 0;
static uint32_t indexSector_write = 0;
static detection_system_data mBuffer_write[QSPI_BUFFER_SIZE_DETSYSDAT];

void qspiDetectionSystem_PushSampleInExternalFlash(detection_system_data sample)
{
    ret_code_t err_code;
 
    //Write an new element in the buffer:
    mBuffer_write[indexBuffer_write] = sample;
    NRF_LOG_INFO("Write in buffer. indexBuffer_write: %d. timeValue: %d.", indexBuffer_write, mBuffer_write[indexBuffer_write].time[0]);

    //If the buffer is complete:
    if(indexBuffer_write == (QSPI_BUFFER_SIZE_DETSYSDAT-1))
    {
        //Write the data buffer in a memory block:
        uint32_t flashAddress = (indexSector_write*QSPI_SECTOR_SIZE_BYTES) + (indexBlock_write*QSPI_BLOCK_SIZE_BYTES) + QSPI_OFFSET_DETSYS;
        err_code = nrf_drv_qspi_write(&mBuffer_write, QSPI_BLOCK_SIZE_BYTES, flashAddress);
        APP_ERROR_CHECK(err_code);
        WAIT_FOR_PERIPH();
        NRF_LOG_INFO("Writing. indexSector_write: %d. indexBlock_write: %d, flashAddress: %d", indexSector_write, indexBlock_write, flashAddress);

        //If the sector is complete:
        if(indexBlock_write == (QSPI_NUMBER_BLOCKS-1))
        {
            //Update memory sector index to write:
            indexSector_write = (indexSector_write+1) % ((uint32_t) QSPI_NUMBER_SECTORS_DETSYS);
        }

        //Update memory block index to write:
        indexBlock_write = (indexBlock_write+1) % ((uint32_t) QSPI_NUMBER_BLOCKS);
    }
    
    //Update counter of the buffer:
    indexBuffer_write = (indexBuffer_write+1) % ((uint32_t) QSPI_BUFFER_SIZE_DETSYSDAT);

}

static uint32_t indexBuffer_read = 0;
static uint32_t indexBlock_read = 0;
static uint32_t indexSector_read = 0;


static uint32_t qspiDetectionSystem_WriteReadBufferDistance(void)
{
    uint32_t indexBuffer_writeAux           = indexBuffer_write + (QSPI_BUFFER_SIZE_DETSYSDAT*(indexBlock_write + (QSPI_NUMBER_BLOCKS*indexSector_write)));
    uint32_t indexBuffer_readAux            = indexBuffer_read + (QSPI_BUFFER_SIZE_DETSYSDAT*(indexBlock_read  + (QSPI_NUMBER_BLOCKS*indexSector_read)));
    uint32_t QSPI_BUFFER_SIZE_DETSYSDAT_AUX = QSPI_BUFFER_SIZE_DETSYSDAT*(QSPI_NUMBER_BLOCKS*QSPI_NUMBER_SECTORS_DETSYS);

    uint32_t writeReadBufferDistance = ((QSPI_BUFFER_SIZE_DETSYSDAT_AUX-1) - ((indexBuffer_readAux + ((QSPI_BUFFER_SIZE_DETSYSDAT_AUX-1)-indexBuffer_writeAux)) % ((uint32_t) QSPI_BUFFER_SIZE_DETSYSDAT_AUX)));
    return writeReadBufferDistance;
}


void qspiDetectionSystem_ReadExternalFlashAndSendBleDataIfPossible(void){

    ret_code_t err_code;

    //Declare static variables:
    static detection_system_data m_buffer_rx[QSPI_BUFFER_SIZE_DETSYSDAT];
    static bool isThereRxBufferSavedFromFlash = false;

    //If there is a m_buffer_rx saved from the flash, then send BT data as soon as possible until the the buffer is totally read:
    if(isThereRxBufferSavedFromFlash)
    {
        static uint8_t indexBleData = 0;

        //Send data via BT from the m_buffer_rx:
        detection_system_single_data bleData = detectionSystem_getStructSingleData(m_buffer_rx[indexBuffer_read], indexBleData);
        bleCusSensSendData(bleData);
        NRF_LOG_INFO("BLE SENS SERVICE: send. Read from Buffer_read. indexBleData: %d. indexBuffer_read: %d. timeValue[%d]: %d", indexBleData, indexBuffer_read, indexBleData, bleData.time);
        
        //If it is sent the last BT data:
        if(indexBleData == (4-1))
        {
            //If it is send the last of the read buffer:
            if(indexBuffer_read == (QSPI_BUFFER_SIZE_DETSYSDAT-1))
            {   
                //There is no longer a m_buffer_rx to read:
                isThereRxBufferSavedFromFlash = false;
            
                //If it is read the last block from flash memory:
                if(indexBlock_read == (QSPI_NUMBER_BLOCKS-1))
                {
                    //Then erase the last sector of 4KB already read:
                    uint32_t flashAddress = (indexSector_read*QSPI_SECTOR_SIZE_BYTES) + QSPI_OFFSET_DETSYS;
                    err_code = nrf_drv_qspi_erase(QSPI_ERASE_LEN_LEN_4KB, flashAddress);
                    APP_ERROR_CHECK(err_code);
                    WAIT_FOR_PERIPH();
                    NRF_LOG_INFO("**********************************************************************");
                    NRF_LOG_INFO("*** Sector of 4KB erased. indexSector: %d. flashAddres: %d", indexSector_read, flashAddress);
                    NRF_LOG_INFO("**********************************************************************");
                
                    //Update memory sector index to read:
                    indexSector_read = (indexSector_read+1) % ((uint32_t) QSPI_NUMBER_SECTORS_DETSYS);    
                }
                //Update memory block index to read:
                indexBlock_read = (indexBlock_read+1) % ((uint32_t) QSPI_NUMBER_BLOCKS); 
            }
            //Update the buffer index to read:
            indexBuffer_read = (indexBuffer_read+1) % ((uint32_t) QSPI_BUFFER_SIZE_DETSYSDAT);
        }
        //Update the index of the BT data to send:
        indexBleData = (indexBleData+1) % ((uint8_t) 4);

    }
    //If there is not a m_buffer_rx saved from the flash, then check how larger is the distance
    else
    {
        uint32_t distanceWriteReadBuffer = qspiDetectionSystem_WriteReadBufferDistance();

        //If the distance is larger enough, then read the memory flash an store data in the m_buffer_rx
        if(distanceWriteReadBuffer >= QSPI_BUFFER_SIZE_DETSYSDAT)
        {
            //Read a memory block from flash:
            uint32_t flashAddress = (indexSector_read*QSPI_SECTOR_SIZE_BYTES) + (indexBlock_read*QSPI_BLOCK_SIZE_BYTES) + QSPI_OFFSET_DETSYS;
            err_code = nrf_drv_qspi_read(m_buffer_rx, QSPI_BLOCK_SIZE_BYTES, flashAddress);
            WAIT_FOR_PERIPH();
            NRF_LOG_INFO("Reading. indexSector_read: %d. indexBlock_read: %d, flashAddress: %d", indexSector_read, indexBlock_read, flashAddress);

            //Now there is a m_buffer_rx to read:
            isThereRxBufferSavedFromFlash = true;
        }
        else if(distanceWriteReadBuffer > 0)
        {
            static uint8_t indexBleData = 0;

            //Send data via BT from the mBuffer_write:
            detection_system_single_data bleData = detectionSystem_getStructSingleData(mBuffer_write[indexBuffer_read], indexBleData);
            bleCusSensSendData(bleData);
            NRF_LOG_INFO("BLE SENS SERVICE: send. Read from Buffer_write. indexBleData: %d. indexBuffer_read: %d. timeValue[%d]: %d", indexBleData, indexBuffer_read, indexBleData, bleData.time);

            //If it is sent the last BT data:
            if(indexBleData == (4-1))
            {
                //Update index for reading:
                if(indexBuffer_read == (QSPI_BUFFER_SIZE_DETSYSDAT-1))
                {
                    //If it is read the last block from flash memory (actually, the flash is not directly read):
                    if(indexBlock_read == (QSPI_NUMBER_BLOCKS-1))
                    {
                        //Then erase the last sector of 4KB already read:
                        uint32_t flashAddress = (indexSector_read*QSPI_SECTOR_SIZE_BYTES) + QSPI_OFFSET_DETSYS;
                        err_code = nrf_drv_qspi_erase(QSPI_ERASE_LEN_LEN_4KB, flashAddress);
                        APP_ERROR_CHECK(err_code);
                        WAIT_FOR_PERIPH();
                        NRF_LOG_INFO("**********************************************************************");
                        NRF_LOG_INFO("*** Sector of 4KB erased. indexSector: %d. flashAddres: %d", indexSector_read, flashAddress);
                        NRF_LOG_INFO("**********************************************************************");
                

                        //Update memory sector index to read:
                        indexSector_read = (indexSector_read+1) % ((uint32_t) QSPI_NUMBER_SECTORS_DETSYS);    
                    }
                    //Update memory block index to read:
                    indexBlock_read = (indexBlock_read+1) % ((uint32_t) QSPI_NUMBER_BLOCKS); 
                }
                //Update the buffer index to read:
                indexBuffer_read = (indexBuffer_read+1) % ((uint32_t) QSPI_BUFFER_SIZE_DETSYSDAT);
            }
            //Update the index of the BT data to send:
            indexBleData = (indexBleData+1) % ((uint8_t) 4);
        }
        //If it is the last data, if it is not measuring and there is still data on flash for "Sens" (data on flash means there is still data to send)
        else if(!deviceStatus_getStructData_isMeasuring() && deviceStatus_getStructData_isSensDataOnFlash())
        {
            //Update device status data for "isSensDataOnFlash":
            deviceStatus_saveStructData_isSensDataOnFlash(false);
            NRF_LOG_INFO("FLASH: isSensDataOnFlash = %d.", deviceStatus_getStructData_isSensDataOnFlash());

            //If there is not data on flash for "Cont" nor "Batt" either: 
            if(!deviceStatus_getStructData_isContDataOnFlash() && !deviceStatus_getStructData_isBattDataOnFlash())
            {
                //Stop the millis Timer and update the internal device status data:
                hundredMillisStop();
                deviceStatus_saveStructData_isDataOnFlash(false);
                deviceStatus_saveStructData_fileName(0,0,0,0,0,0);
                NRF_LOG_INFO("FLASH: isDataOnFlash = %d.", deviceStatus_getStructData_isDataOnFlash());

                //Update device status and notify STAT characteristic:
                bleCusStatSendData(deviceStatus_getStructData());
                NRF_LOG_INFO("FLASH: Send notification of the STAT characteristic. commandFromPhone = %d. isMeasuring = %d. isDataOnFlash = %d", deviceStatus_getStructData_commandFromPhone(), deviceStatus_getStructData_isMeasuring(), deviceStatus_getStructData_isDataOnFlash());
            }
                                                    
        }

    }
    

}


///////////////////////////////////////////////////////////////////////////////////////////
/// For Controller System /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
//Declare static variables:
static uint32_t indBuffer_write = 0;
static uint32_t indBlock_write = 0;
static uint32_t indSector_write = 0;
static control_system_data mBuff_write[QSPI_BUFFER_SIZE_CONSYSDAT];

void qspiControlSystem_PushSampleInExternalFlash(control_system_data sample)
{
    ret_code_t err_code;
 
    //Write an new element in the buffer:
    mBuff_write[indBuffer_write] = sample;
    NRF_LOG_INFO("QSPI_CONT: Write in buffer. indBuffer_write: %d. timeValue: %d.", indBuffer_write, mBuff_write[indBuffer_write].time);

    //If the buffer is complete:
    if(indBuffer_write == (QSPI_BUFFER_SIZE_CONSYSDAT-1))
    {
        //Write the data buffer in a memory block:
        uint32_t flashAddress = (indSector_write*QSPI_SECTOR_SIZE_BYTES) + (indBlock_write*QSPI_BLOCK_SIZE_BYTES) + QSPI_OFFSET_CONSYS;
        err_code = nrf_drv_qspi_write(&mBuff_write, QSPI_BLOCK_SIZE_BYTES, flashAddress);
        APP_ERROR_CHECK(err_code);
        WAIT_FOR_PERIPH();
        NRF_LOG_INFO("QSPI_CONT: Writing. indSector_write: %d. indBlock_write: %d, flashAddress: %d", indSector_write, indBlock_write, flashAddress);

        //If the sector is complete:
        if(indBlock_write == (QSPI_NUMBER_BLOCKS-1))
        {
            //Update memory sector ind to write:
            indSector_write = (indSector_write+1) % ((uint32_t) QSPI_NUMBER_SECTORS_CONSYS);
        }

        //Update memory block ind to write:
        indBlock_write = (indBlock_write+1) % ((uint32_t) QSPI_NUMBER_BLOCKS);
    }
    
    //Update counter of the buffer:
    indBuffer_write = (indBuffer_write+1) % ((uint32_t) QSPI_BUFFER_SIZE_CONSYSDAT);

}


static uint32_t indBuffer_read = 0;
static uint32_t indBlock_read = 0;
static uint32_t indSector_read = 0;


static uint32_t qspiControlSystem_WriteReadBufferDistance(void)
{
    uint32_t indBuffer_writeAux = indBuffer_write + (QSPI_BUFFER_SIZE_CONSYSDAT*(indBlock_write + (QSPI_NUMBER_BLOCKS*indSector_write)));
    uint32_t indBuffer_readAux  = indBuffer_read + (QSPI_BUFFER_SIZE_CONSYSDAT*(indBlock_read  + (QSPI_NUMBER_BLOCKS*indSector_read)));
    uint32_t QSPI_BUFFER_SIZE_CONSYSDAT_AUX = QSPI_BUFFER_SIZE_CONSYSDAT*(QSPI_NUMBER_BLOCKS*QSPI_NUMBER_SECTORS_CONSYS);

    uint32_t writeReadBufferDistance = ((QSPI_BUFFER_SIZE_CONSYSDAT_AUX-1) - ((indBuffer_readAux + ((QSPI_BUFFER_SIZE_CONSYSDAT_AUX-1)-indBuffer_writeAux)) % ((uint32_t) QSPI_BUFFER_SIZE_CONSYSDAT_AUX)));
    return writeReadBufferDistance;
}


void qspiControlSystem_ReadExternalFlashAndSendBleDataIfPossible(void){

    ret_code_t err_code;

    //Declare static variables:
    static control_system_data m_buffer_rx[QSPI_BUFFER_SIZE_CONSYSDAT];
    static bool isThereRxBufferSavedFromFlash = false;

    //If there is a m_buffer_rx saved from the flash, then send BT data as soon as possible until the the buffer is totally read:
    if(isThereRxBufferSavedFromFlash)
    {
        //Send data via BT from the m_buffer_rx:
        control_system_data bleData = m_buffer_rx[indBuffer_read];
        bleCusContSendData(bleData);
        NRF_LOG_INFO("BLE CONT SERVICE: send. Read from Buffer_read. indBuffer_read: %d. time: %d", indBuffer_read, bleData.time);
        
        
        //If it is send the last of the read buffer:
        if(indBuffer_read == (QSPI_BUFFER_SIZE_CONSYSDAT-1))
        {   
            //There is no longer a m_buffer_rx to read:
            isThereRxBufferSavedFromFlash = false;
           
            //If it is read the last block from flash memory:
            if(indBlock_read == (QSPI_NUMBER_BLOCKS-1))
            {
                //Then erase the last sector of 4KB already read:
                uint32_t flashAddress = (indSector_read*QSPI_SECTOR_SIZE_BYTES) + QSPI_OFFSET_CONSYS;
                err_code = nrf_drv_qspi_erase(QSPI_ERASE_LEN_LEN_4KB, flashAddress);
                APP_ERROR_CHECK(err_code);
                WAIT_FOR_PERIPH();
                NRF_LOG_INFO("**********************************************************************");
                NRF_LOG_INFO("*** QSPI_CONT: Sector of 4KB erased. indSector: %d. flashAddres: %d", indSector_read, flashAddress);
                NRF_LOG_INFO("**********************************************************************");
                
                //Update memory sector ind to read:
                indSector_read = (indSector_read+1) % ((uint32_t) QSPI_NUMBER_SECTORS_CONSYS);    
            }
            //Update memory block ind to read:
            indBlock_read = (indBlock_read+1) % ((uint32_t) QSPI_NUMBER_BLOCKS); 
        }
        //Update the buffer ind to read:
        indBuffer_read = (indBuffer_read+1) % ((uint32_t) QSPI_BUFFER_SIZE_CONSYSDAT);

    }
    //If there is not a m_buffer_rx saved from the flash, then check how larger is the distance
    else
    {
        uint32_t distanceWriteReadBuffer = qspiControlSystem_WriteReadBufferDistance();

        //If the distance is large enough, then read the memory flash an store data in the m_buffer_rx
        if(distanceWriteReadBuffer >= QSPI_BUFFER_SIZE_CONSYSDAT)
        {
            //Read a memory block from flash:
            uint32_t flashAddress = (indSector_read*QSPI_SECTOR_SIZE_BYTES) + (indBlock_read*QSPI_BLOCK_SIZE_BYTES) + QSPI_OFFSET_CONSYS;
            err_code = nrf_drv_qspi_read(m_buffer_rx, QSPI_BLOCK_SIZE_BYTES, flashAddress);
            WAIT_FOR_PERIPH();
            NRF_LOG_INFO("QSPI_CONT: Reading. indSector_read: %d. indBlock_read: %d, flashAddress: %d", indSector_read, indBlock_read, flashAddress);

            //Now there is a m_buffer_rx to read:
            isThereRxBufferSavedFromFlash = true;
        }
        else if(distanceWriteReadBuffer > 0)
        {
            //Send data via BT from the mBuff_write:
            control_system_data bleData = mBuff_write[indBuffer_read];
            bleCusContSendData(bleData);
            NRF_LOG_INFO("BLE CONT SERVICE: send. Read from Buffer_write. indBuffer_read: %d. time: %d", indBuffer_read, bleData.time);

            //Update ind for reading:
            if(indBuffer_read == (QSPI_BUFFER_SIZE_CONSYSDAT-1))
            {
                //If it is read the last block from flash memory (actually, the flash is not directly read):
                if(indBlock_read == (QSPI_NUMBER_BLOCKS-1))
                {
                    //Then erase the last sector of 4KB already read:
                    uint32_t flashAddress = (indSector_read*QSPI_SECTOR_SIZE_BYTES) + QSPI_OFFSET_CONSYS;
                    err_code = nrf_drv_qspi_erase(QSPI_ERASE_LEN_LEN_4KB, flashAddress);
                    APP_ERROR_CHECK(err_code);
                    WAIT_FOR_PERIPH();
                    NRF_LOG_INFO("**********************************************************************");
                    NRF_LOG_INFO("*** QSPI_CONT: Sector of 4KB erased. indSector: %d. flashAddres: %d", indSector_read, flashAddress);
                    NRF_LOG_INFO("**********************************************************************");
                

                    //Update memory sector ind to read:
                    indSector_read = (indSector_read+1) % ((uint32_t) QSPI_NUMBER_SECTORS_CONSYS);    
                }
                //Update memory block ind to read:
                indBlock_read = (indBlock_read+1) % ((uint32_t) QSPI_NUMBER_BLOCKS); 
            }
            //Update the buffer ind to read:
            indBuffer_read = (indBuffer_read+1) % ((uint32_t) QSPI_BUFFER_SIZE_CONSYSDAT);

        }
        //If it is the last data, if it is not measuring and there is still data on flash for "Cont" (data on flash means there is still data to send)
        else if(!deviceStatus_getStructData_isMeasuring() && deviceStatus_getStructData_isContDataOnFlash())
        {
            //Update device status data for "isContDataOnFlash":
            deviceStatus_saveStructData_isContDataOnFlash(false);
            NRF_LOG_INFO("FLASH: isContDataOnFlash = %d.", deviceStatus_getStructData_isContDataOnFlash());

            //If there is not data on flash for "Sens" nor "Batt" either: 
            if(!deviceStatus_getStructData_isSensDataOnFlash() && !deviceStatus_getStructData_isBattDataOnFlash())
            {
                //Stop the millis Timer and update the internal device status data:
                hundredMillisStop();
                deviceStatus_saveStructData_isDataOnFlash(false);
                deviceStatus_saveStructData_fileName(0,0,0,0,0,0);
                NRF_LOG_INFO("FLASH: isDataOnFlash = %d.", deviceStatus_getStructData_isDataOnFlash());
            
                //Update device status and notify STAT characteristic:
                bleCusStatSendData(deviceStatus_getStructData());
                NRF_LOG_INFO("FLASH: Send notification of the STAT characteristic. commandFromPhone = %d. isMeasuring = %d. isDataOnFlash = %d", deviceStatus_getStructData_commandFromPhone(), deviceStatus_getStructData_isMeasuring(), deviceStatus_getStructData_isDataOnFlash());                             
            }

        }

    }
    
}


////////////////////////////////////////////////////////////////////////////////////////
/// For Battery System /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//Declare static variables:
static uint32_t idxBuffer_write = 0;
static uint32_t idxBlock_write = 0;
static uint32_t idxSector_write = 0;
static battery_system_data mBufr_write[QSPI_BUFFER_SIZE_BATSYSDAT];

void qspiBatterySystem_PushSampleInExternalFlash(battery_system_data sample)
{
    ret_code_t err_code;
 
    //Write an new element in the buffer:
    mBufr_write[idxBuffer_write] = sample;
    NRF_LOG_INFO("QSPI_BATT: Write in buffer. idxBuffer_write: %d. timeValue: %d.", idxBuffer_write, mBufr_write[idxBuffer_write].time);

    //If the buffer is complete:
    if(idxBuffer_write == (QSPI_BUFFER_SIZE_BATSYSDAT-1))
    {
        //Write the data buffer in a memory block:
        uint32_t flashAddress = (idxSector_write*QSPI_SECTOR_SIZE_BYTES) + (idxBlock_write*QSPI_BLOCK_SIZE_BYTES) + QSPI_OFFSET_BATSYS;
        err_code = nrf_drv_qspi_write(&mBufr_write, QSPI_BLOCK_SIZE_BYTES, flashAddress);
        APP_ERROR_CHECK(err_code);
        WAIT_FOR_PERIPH();
        NRF_LOG_INFO("QSPI_BATT: Writing. idxSector_write: %d. idxBlock_write: %d, flashAddress: %d", idxSector_write, idxBlock_write, flashAddress);

        //If the sector is complete:
        if(idxBlock_write == (QSPI_NUMBER_BLOCKS-1))
        {
            //Update memory sector idx to write:
            idxSector_write = (idxSector_write+1) % ((uint32_t) QSPI_NUMBER_SECTORS_BATSYS);
        }

        //Update memory block idx to write:
        idxBlock_write = (idxBlock_write+1) % ((uint32_t) QSPI_NUMBER_BLOCKS);
    }
    
    //Update counter of the buffer:
    idxBuffer_write = (idxBuffer_write+1) % ((uint32_t) QSPI_BUFFER_SIZE_BATSYSDAT);

}


static uint32_t idxBuffer_read = 0;
static uint32_t idxBlock_read = 0;
static uint32_t idxSector_read = 0;

static uint32_t qspiBatterySystem_WriteReadBufferDistance(void)
{
    uint32_t idxBuffer_writeAux = idxBuffer_write + (QSPI_BUFFER_SIZE_BATSYSDAT*(idxBlock_write + (QSPI_NUMBER_BLOCKS*idxSector_write)));
    uint32_t idxBuffer_readAux  = idxBuffer_read + (QSPI_BUFFER_SIZE_BATSYSDAT*(idxBlock_read  + (QSPI_NUMBER_BLOCKS*idxSector_read)));
    uint32_t QSPI_BUFFER_SIZE_BATSYSDAT_AUX = QSPI_BUFFER_SIZE_BATSYSDAT*(QSPI_NUMBER_BLOCKS*QSPI_NUMBER_SECTORS_BATSYS);

    uint32_t writeReadBufferDistance = ((QSPI_BUFFER_SIZE_BATSYSDAT_AUX-1) - ((idxBuffer_readAux + ((QSPI_BUFFER_SIZE_BATSYSDAT_AUX-1)-idxBuffer_writeAux)) % ((uint32_t) QSPI_BUFFER_SIZE_BATSYSDAT_AUX)));
    return writeReadBufferDistance;
}


void qspiBatterySystem_ReadExternalFlashAndSendBleDataIfPossible(void){

    ret_code_t err_code;

    //Declare static variables:
    static battery_system_data m_buffer_rx[QSPI_BUFFER_SIZE_BATSYSDAT];
    static bool isThereRxBufferSavedFromFlash = false;

    //If there is a m_buffer_rx saved from the flash, then send BT data as soon as possible until the the buffer is totally read:
    if(isThereRxBufferSavedFromFlash)
    {
        //Send data via BT from the m_buffer_rx:
        battery_system_data bleData = m_buffer_rx[idxBuffer_read];
        bleCusBattSendData(bleData);
        NRF_LOG_INFO("BLE BATT SERVICE: send. Read from Buffer_read. idxBuffer_read: %d. time: %d", idxBuffer_read, bleData.time);
        
        
        //If it is send the last of the read buffer:
        if(idxBuffer_read == (QSPI_BUFFER_SIZE_BATSYSDAT-1))
        {   
            //There is no longer a m_buffer_rx to read:
            isThereRxBufferSavedFromFlash = false;
           
            //If it is read the last block from flash memory:
            if(idxBlock_read == (QSPI_NUMBER_BLOCKS-1))
            {
                //Then erase the last sector of 4KB already read:
                uint32_t flashAddress = (idxSector_read*QSPI_SECTOR_SIZE_BYTES) + QSPI_OFFSET_BATSYS;
                err_code = nrf_drv_qspi_erase(QSPI_ERASE_LEN_LEN_4KB, flashAddress);
                APP_ERROR_CHECK(err_code);
                WAIT_FOR_PERIPH();
                NRF_LOG_INFO("**********************************************************************");
                NRF_LOG_INFO("*** QSPI_BATT: Sector of 4KB erased. idxSector: %d. flashAddres: %d", idxSector_read, flashAddress);
                NRF_LOG_INFO("**********************************************************************");
                
                //Update memory sector idx to read:
                idxSector_read = (idxSector_read+1) % ((uint32_t) QSPI_NUMBER_SECTORS_BATSYS);    
            }
            //Update memory block idx to read:
            idxBlock_read = (idxBlock_read+1) % ((uint32_t) QSPI_NUMBER_BLOCKS); 
        }
        //Update the buffer idx to read:
        idxBuffer_read = (idxBuffer_read+1) % ((uint32_t) QSPI_BUFFER_SIZE_BATSYSDAT);

    }
    //If there is not a m_buffer_rx saved from the flash, then check how larger is the distance
    else
    {
        uint32_t distanceWriteReadBuffer = qspiBatterySystem_WriteReadBufferDistance();

        //If the distance is large enough, then read the memory flash an store data in the m_buffer_rx
        if(distanceWriteReadBuffer >= QSPI_BUFFER_SIZE_BATSYSDAT)
        {
            //Read a memory block from flash:
            uint32_t flashAddress = (idxSector_read*QSPI_SECTOR_SIZE_BYTES) + (idxBlock_read*QSPI_BLOCK_SIZE_BYTES) + QSPI_OFFSET_BATSYS;
            err_code = nrf_drv_qspi_read(m_buffer_rx, QSPI_BLOCK_SIZE_BYTES, flashAddress);
            WAIT_FOR_PERIPH();
            NRF_LOG_INFO("QSPI_BATT: Reading. idxSector_read: %d. idxBlock_read: %d, flashAddress: %d", idxSector_read, idxBlock_read, flashAddress);

            //Now there is a m_buffer_rx to read:
            isThereRxBufferSavedFromFlash = true;
        }
        else if(distanceWriteReadBuffer > 0)
        {
            //Send data via BT from the mBufr_write:
            battery_system_data bleData = mBufr_write[idxBuffer_read];
            bleCusBattSendData(bleData);
            NRF_LOG_INFO("BLE BATT SERVICE: send. Read from Buffer_write. idxBuffer_read: %d. time: %d", idxBuffer_read, bleData.time);

            //Update idx for reading:
            if(idxBuffer_read == (QSPI_BUFFER_SIZE_BATSYSDAT-1))
            {
                //If it is read the last block from flash memory (actually, the flash is not directly read):
                if(idxBlock_read == (QSPI_NUMBER_BLOCKS-1))
                {
                    //Then erase the last sector of 4KB already read:
                    uint32_t flashAddress = (idxSector_read*QSPI_SECTOR_SIZE_BYTES) + QSPI_OFFSET_BATSYS;
                    err_code = nrf_drv_qspi_erase(QSPI_ERASE_LEN_LEN_4KB, flashAddress);
                    APP_ERROR_CHECK(err_code);
                    WAIT_FOR_PERIPH();
                    NRF_LOG_INFO("**********************************************************************");
                    NRF_LOG_INFO("*** QSPI_BATT: Sector of 4KB erased. idxSector: %d. flashAddres: %d", idxSector_read, flashAddress);
                    NRF_LOG_INFO("**********************************************************************");
                

                    //Update memory sector idx to read:
                    idxSector_read = (idxSector_read+1) % ((uint32_t) QSPI_NUMBER_SECTORS_BATSYS);    
                }
                //Update memory block idx to read:
                idxBlock_read = (idxBlock_read+1) % ((uint32_t) QSPI_NUMBER_BLOCKS); 
            }
            //Update the buffer idx to read:
            idxBuffer_read = (idxBuffer_read+1) % ((uint32_t) QSPI_BUFFER_SIZE_BATSYSDAT);

        }
        //If it is the last data, if it is not measuring and there is still data on flash for "Batt" (data on flash means there is still data to send)
        else if(!deviceStatus_getStructData_isMeasuring() && deviceStatus_getStructData_isBattDataOnFlash())
        {
            //Update device status data for "isBattDataOnFlash":
            deviceStatus_saveStructData_isBattDataOnFlash(false);
            NRF_LOG_INFO("FLASH: isBattDataOnFlash = %d.", deviceStatus_getStructData_isBattDataOnFlash());

            //If there is not data on flash for "Sens" nor for "Cont" either: 
            if(!deviceStatus_getStructData_isSensDataOnFlash() && !deviceStatus_getStructData_isContDataOnFlash())
            {
                //Stop the millis Timer and update the internal device status data:
                hundredMillisStop();
                deviceStatus_saveStructData_isDataOnFlash(false);
                deviceStatus_saveStructData_fileName(0,0,0,0,0,0);
                NRF_LOG_INFO("FLASH: isDataOnFlash = %d.", deviceStatus_getStructData_isDataOnFlash());
            
                //Update device status and notify STAT characteristic:
                bleCusStatSendData(deviceStatus_getStructData());
                NRF_LOG_INFO("FLASH: Send notification of the STAT characteristic. commandFromPhone = %d. isMeasuring = %d. isDataOnFlash = %d", deviceStatus_getStructData_commandFromPhone(), deviceStatus_getStructData_isMeasuring(), deviceStatus_getStructData_isDataOnFlash());                             
            }

        }
        
    }
    
}