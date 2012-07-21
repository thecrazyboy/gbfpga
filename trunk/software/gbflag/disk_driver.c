#include "FatFs/diskio.h"
#include "SDC/sdc.h"

#include <stdio.h>

int g_bInitDone;

DSTATUS disk_initialize (BYTE byDrive)
{
    g_bInitDone = SD_Init();
    
    if (g_bInitDone)
    {
        return 0;
    }
    else
    {
        return STA_NOINIT;
    }
}
DSTATUS disk_status (BYTE byDrive)
{
     if (g_bInitDone)
    {
        return 0;
    }
    else
    {
        return STA_NOINIT;
    }   
}

DRESULT disk_read (BYTE byDrive, BYTE* pBuffer, DWORD dwSector, BYTE byCount)
{
    int bSuccess = 1;
    
    int i;
    for (i = 0; i < byCount && bSuccess; i++)
    {
        bSuccess = bSuccess && SD_Read(dwSector + i, &pBuffer[i * 512]);
    }
    
    if (bSuccess)
    {
        return RES_OK;
    }
    else
    {
        return RES_ERROR;
    }
}

DRESULT disk_ioctl (BYTE byDrive, BYTE byCode, void* pBuffer)
{
    return RES_OK;
}
