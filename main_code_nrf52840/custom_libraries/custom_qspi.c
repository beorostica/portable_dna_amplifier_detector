
#include "custom_qspi.h"

#include "boards.h"
#include "nrf_drv_qspi.h"
#include "custom_ble_manager.h"

#include "custom_log.h"


#define QSPI_STD_CMD_WRSR   0x01
#define QSPI_STD_CMD_RSTEN  0x66
#define QSPI_STD_CMD_RST    0x99

#define QSPI_NUMBER_SECTORS         3                        //2 Sectors of 4096 Bytes (To write: I can erase sectors, not blocks)
#define QSPI_SECTOR_SIZE_BYTES      4096                     //User can erase a sector of 4096 Bytes
#define QSPI_NUMBER_BLOCKS          16                       //16 Blocks of 256 Bytes = Store 4096 Bytes ~ 4KB
#define QSPI_BLOCK_SIZE_BYTES       256                      //256 Bytes is the size of a block in memory
#define QSPI_BUFFER_SIZE_UINT8      QSPI_BLOCK_SIZE_BYTES/1  //256 elements in a buffer of uint8_t  (1 Bytes)
#define QSPI_BUFFER_SIZE_UINT16     QSPI_BLOCK_SIZE_BYTES/2  //128 elements in a buffer of uint16_t (2 Bytes)
#define QSPI_BUFFER_SIZE_UINT32     QSPI_BLOCK_SIZE_BYTES/4  // 64 elements in a buffer of uint32_t (4 Bytes)
#define QSPI_BUFFER_SIZE_DETSYSDAT  QSPI_BLOCK_SIZE_BYTES/32 //  8 elements in a buffer of detection_system_data (32 Bytes)

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
}


void qspiInit(void)
{
    ret_code_t err_code;

    nrf_drv_qspi_config_t config = NRF_DRV_QSPI_DEFAULT_CONFIG;
    err_code = nrf_drv_qspi_init(&config, qspi_handler, NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("QSPI started.");

    configure_memory();

    m_finished = false;

    //Erase the first flash block of 64KB: 
    err_code = nrf_drv_qspi_erase(NRF_QSPI_ERASE_LEN_64KB, 0);
    APP_ERROR_CHECK(err_code);
    WAIT_FOR_PERIPH();
    NRF_LOG_INFO("Process of erasing first 64KB block start");

}


//Declare static variables:
static uint32_t indexBuffer_write = 0;
static uint32_t indexBlock_write = 0;
static uint32_t indexSector_write = 0;
static detection_system_data mBuffer_write[QSPI_BUFFER_SIZE_DETSYSDAT];

void qspiPushSampleInExternalFlash(detection_system_data sample)
{
    ret_code_t err_code;
 
    //Write an new element in the buffer:
    mBuffer_write[indexBuffer_write] = sample;
    NRF_LOG_INFO("Write in buffer. indexBuffer_write: %d. timeValue: %d.", indexBuffer_write, mBuffer_write[indexBuffer_write].time[0]);

    //If the buffer is complete:
    if(indexBuffer_write == (QSPI_BUFFER_SIZE_DETSYSDAT-1))
    {
        //Write the data buffer in a memory block:
        uint32_t flashAddress = (indexSector_write*QSPI_SECTOR_SIZE_BYTES) + (indexBlock_write*QSPI_BLOCK_SIZE_BYTES);
        err_code = nrf_drv_qspi_write(&mBuffer_write, QSPI_BLOCK_SIZE_BYTES, flashAddress);
        APP_ERROR_CHECK(err_code);
        WAIT_FOR_PERIPH();
        NRF_LOG_INFO("Writing. indexSector_write: %d. indexBlock_write: %d, flashAddress: %d", indexSector_write, indexBlock_write, flashAddress);

        //If the sector is complete:
        if(indexBlock_write == (QSPI_NUMBER_BLOCKS-1))
        {
            //Update memory sector index to write:
            indexSector_write = (indexSector_write+1) % ((uint32_t) QSPI_NUMBER_SECTORS);
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


static uint32_t qspiWriteReadBufferDistance(void)
{
    uint32_t indexBuffer_writeAux           = indexBuffer_write + (QSPI_BUFFER_SIZE_DETSYSDAT*(indexBlock_write + (QSPI_NUMBER_BLOCKS*indexSector_write)));
    uint32_t indexBuffer_readAux            = indexBuffer_read + (QSPI_BUFFER_SIZE_DETSYSDAT*(indexBlock_read  + (QSPI_NUMBER_BLOCKS*indexSector_read)));
    uint32_t QSPI_BUFFER_SIZE_DETSYSDAT_AUX = QSPI_BUFFER_SIZE_DETSYSDAT*(QSPI_NUMBER_BLOCKS*QSPI_NUMBER_SECTORS);

    uint32_t writeReadBufferDistance = ((QSPI_BUFFER_SIZE_DETSYSDAT_AUX-1) - ((indexBuffer_readAux + ((QSPI_BUFFER_SIZE_DETSYSDAT_AUX-1)-indexBuffer_writeAux)) % ((uint32_t) QSPI_BUFFER_SIZE_DETSYSDAT_AUX)));
    return writeReadBufferDistance;
}


void qspiReadExternalFlashAndSendBleDataIfPossible(void){

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
        NRF_LOG_INFO("Send BT data. Read from Buffer_read. indexBleData: %d. indexBuffer_read: %d. timeValue[%d]: %d", indexBleData, indexBuffer_read, indexBleData, bleData.time);
        
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
                    uint32_t flashAddress = (indexSector_read*QSPI_SECTOR_SIZE_BYTES);
                    err_code = nrf_drv_qspi_erase(QSPI_ERASE_LEN_LEN_4KB, flashAddress);
                    APP_ERROR_CHECK(err_code);
                    WAIT_FOR_PERIPH();
                    NRF_LOG_INFO("**********************************************************************");
                    NRF_LOG_INFO("*** Sector of 4KB erased. indexSector: %d. flashAddres: %d", indexSector_read, flashAddress);
                    NRF_LOG_INFO("**********************************************************************");
                
                    //Update memory sector index to read:
                    indexSector_read = (indexSector_read+1) % ((uint32_t) QSPI_NUMBER_SECTORS);    
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
        uint32_t distanceWriteReadBuffer = qspiWriteReadBufferDistance();

        //If the distance is larger enough, then read the memory flash an store data in the m_buffer_rx
        if(distanceWriteReadBuffer >= QSPI_BUFFER_SIZE_DETSYSDAT)
        {
            //Read a memory block from flash:
            uint32_t flashAddress = (indexSector_read*QSPI_SECTOR_SIZE_BYTES) + (indexBlock_read*QSPI_BLOCK_SIZE_BYTES);
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
            NRF_LOG_INFO("Send BT data. Read from Buffer_write. indexBleData: %d. indexBuffer_read: %d. timeValue[%d]: %d", indexBleData, indexBuffer_read, indexBleData, bleData.time);

            //If it is sent the last BT data:
            if(indexBleData == (4-1))
            {
                //Update index for reading:
                if(indexBuffer_read == (QSPI_BUFFER_SIZE_DETSYSDAT-1))
                {
                    if(indexBlock_read == (QSPI_NUMBER_BLOCKS-1))
                    {
                        //Update memory sector index to read:
                        indexSector_read = (indexSector_read+1) % ((uint32_t) QSPI_NUMBER_SECTORS);    
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

    }
    

}