// SD Card interface implementation

#include <stdio.h>
#include <unistd.h>
#include <io.h>
#include "system.h"
#include "sdc.h"

// Local functions definition
void    Delay();
void    SD_ClockPulse();
void    SD_WriteByte(UINT8 uiByte);
UINT8   SD_ReadByte();
void    SD_SendCommand(UINT8 uiCmdIndex, UINT32 uiArgument);
UINT8   SD_GetR1Response();
void    SD_Ncc();

// AvSDC operation codes
#define AV_SDC_NOP          0x0
#define AV_SDC_GET_DO       0x1
#define AV_SDC_SET_CLK_LOW  0x2
#define AV_SDC_SET_CLK_HIGH 0x3
#define AV_SDC_SET_CS_LOW   0x4
#define AV_SDC_SET_CS_HIGH  0x5
#define AV_SDC_SET_DI_LOW   0x6
#define AV_SDC_SET_DI_HIGH  0x7
#define AV_SDC_SEND_DUMMY   0x8
#define AV_SDC_WRITE_BYTE   0x9
#define AV_SDC_READ_BYTE    0xA

// IO macros for SPI signals
#define SD_SET_CS_LOW()       IOWR(AVSDC_0_BASE, 0, AV_SDC_SET_CS_LOW)
#define SD_SET_CS_HIGH()      IOWR(AVSDC_0_BASE, 0, AV_SDC_SET_CS_HIGH)
#define SD_SET_CLK_LOW()      IOWR(AVSDC_0_BASE, 0, AV_SDC_SET_CLK_LOW)
#define SD_SET_CLK_HIGH()     IOWR(AVSDC_0_BASE, 0, AV_SDC_SET_CLK_HIGH)
#define SD_SET_DI_LOW()       IOWR(AVSDC_0_BASE, 0, AV_SDC_SET_DI_LOW)
#define SD_SET_DI_HIGH()      IOWR(AVSDC_0_BASE, 0, AV_SDC_SET_DI_HIGH)
#define SD_GET_DO()           ((IORD(AVSDC_0_BASE, 0) >> 8) & 1)

// Performs a half cycle delay
void Delay()
{
    asm("nop");
}

// Sends a clock pulse through SPI
void SD_ClockPulse()
{
    Delay();
    SD_SET_CLK_HIGH();
    Delay();
    SD_SET_CLK_LOW();
}

// Write a byte to the SD card through SPI
//   uiByte : Byte to send
void SD_WriteByte(UINT8 uiByte)
{
    IOWR(AVSDC_0_BASE, 0, AV_SDC_WRITE_BYTE | (uiByte << 8));
    /*
    for (int i = 0; i < 8; i++)
    {
        if (uiByte & 0x80)
        {
            SD_SET_DI_HIGH();
        }
        else
        {
            SD_SET_DI_LOW();
        }

        SD_ClockPulse();

        uiByte = uiByte << 1;
    }*/
}

// Read a byte from SD Card through SPI
//   return : Byte read
UINT8 SD_ReadByte()
{
    /*
    UINT8 uiByte = 0;

    for (int i = 0; i < 8; i++)
    {
        uiByte = uiByte << 1;
        if (SD_GET_DO() != 0)
        {
            uiByte = uiByte | 1;
        }
        
        SD_ClockPulse();
    }
   
    return uiByte;*/
    IOWR(AVSDC_0_BASE, 0, AV_SDC_READ_BYTE);
    return (IORD(AVSDC_0_BASE, 0) & 0xff);
}

// Send a command to the SD Card
//  uiCmdIndex : Command index. 0 to 63
//  uiArgument : Command argument
void SD_SendCommand(UINT8 uiCmdIndex, UINT32 uiArgument)
{
    // Send command
    UINT8 uiCmdByte = 64 + (uiCmdIndex & 63);
    SD_WriteByte(uiCmdByte);
    
    // Send argument
    int i;
    for (i = 0; i < 4; i++)
    {
        SD_WriteByte((uiArgument >> 24) & 0xff);
        uiArgument = uiArgument << 8;
    }
    
    SD_WriteByte(0x95); // Hard coded CRC
} 

// Get R1 response from command sent to SD Card
// return : Response in R1 format
UINT8 SD_GetR1Response()
{
    UINT8 uiR1 = 0xff;
    
    int iNbTry = 0;
    while ((uiR1 & 0x80) && iNbTry < 8)
    {
        uiR1 = SD_ReadByte();
        iNbTry++;
    }
    
    return uiR1;
}

// Send dummy clock pulse
void SD_Ncc()
{
    IOWR(AVSDC_0_BASE, 0, AV_SDC_SEND_DUMMY);
    /*for (int i = 0; i < 8; i++)
    {
        SD_ClockPulse();
    }*/
}

// Initialize SD Card for SPI access
//  -- return : true if successfully initialized, false otherwise
int SD_Init()
{
    int bSuccess = 1;
    
    usleep(100000);
    
    SD_SET_CS_HIGH();
    SD_SET_DI_HIGH();
    SD_SET_CLK_LOW();
    
    int i;
    for (i = 0; i < 74; i++)
    {
        SD_ClockPulse();
    }
    
    // Send idle command
    SD_SET_CS_LOW();
    SD_SendCommand(0, 0);
    
    UINT8 uiIdleStatus = SD_GetR1Response();
    
    // Check idle bit
    if (uiIdleStatus != 0x01)
    {
        printf("SD Card reset failed. Error code : %02X\n", uiIdleStatus);
        return 0;
    }
    
    // Send init command (ACMD41)
    int bInitialized = 0;
    int iNbTry = 0;
    while (!bInitialized && iNbTry < 100000)
    {
        SD_Ncc();
        SD_SendCommand(55, 0);  // Advance command prefix
        UINT8 uiTempStatus = SD_GetR1Response();
        
        if ((uiTempStatus & 0xFE) != 0x00)
        {
            printf("SD Init failed (55). Error code : %02X\n", uiTempStatus);
            return 0;
        }
        
        SD_Ncc();
        SD_SendCommand(41, 0);
        UINT8 uiInitStatus = SD_GetR1Response();
        
        if ((uiInitStatus & 0xFE) != 0x00)
        {
            printf("SD Init failed (41). Error code : %02X\n", uiInitStatus);
            return 0;
        }
        
        // Check idle bit
        if ((uiInitStatus & 1) == 0)
        {
            bInitialized = 1;
        }
        iNbTry++;
    }
    
    return bSuccess;
}

// Read a 512 bytes sector on the SD Card
//  uiSector : Sector number
//  pbyReadData : pointer to a 512 bytes buffer that will be filled with secotr data if successful
// return : true if data read successully, false otherwise
int SD_Read(UINT32 uiSector, UINT8* pbyReadData)
{
    int bSuccess = 1;
    UINT32 uiAddress = uiSector * 512;
    
    SD_Ncc();
    // Send single read command
    SD_SendCommand(17, uiAddress);
    UINT8 uiResponse = SD_GetR1Response();
    if (uiResponse != 0)
    {
        printf("SD Card read data failed. Error code : %02X\n", uiResponse);
        return 0;
    }
    
    // Wait for data token
    UINT8 uiDataToken = 0;
    int iNbTry = 0;
    while (uiDataToken != 0xFE && iNbTry < 2000)
    {
        uiDataToken = SD_ReadByte();
        iNbTry++;
    }
    
    if (uiDataToken != 0xFE)
    {
        printf("SD Card read data timeout. Error code : %02X\n", uiDataToken);
        return 0;
    }
     
    // Read sector
    int i;
    for (i = 0; i < 512; i++)
    {
        pbyReadData[i] = SD_ReadByte();
    }
    
    // Dummy read CRC
    SD_ReadByte();
    SD_ReadByte();
    
    return bSuccess;
}
