// SD Card interface definition

#ifndef SDC_H_
#define SDC_H_

typedef unsigned int    UINT32;
typedef unsigned char   UINT8;

int SD_Init();
int SD_Read(UINT32 uiSector, UINT8* pbyReadData);

#endif /*SDC_H_*/
