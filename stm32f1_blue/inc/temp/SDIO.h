#ifndef __SDIOLIB_H
#define __SDIOLIB_H

#include "stdint.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define 	SDIO_FLAG_CCRCFAIL   ((uint32_t)0x00000001)
#define 	SDIO_IT_STBITERR     ((uint32_t)0x00000200)

void SD_LowLevel_Init(void);
void SD_Init(void);
void SD_WriteSingleBlock(uint8_t *buf, uint32_t blk);
void SD_ReadSingleBlock(uint8_t *buf, uint32_t blk);
//void SD_WaitReadWriteEnd(void);
void SD_WaitTransmissionEnd(void);
void SD_StartMultipleBlockWrite(uint32_t blk);
void SD_WriteData(uint8_t *buf, uint32_t cnt);
void SD_StopMultipleBlockWrite(void);

//void SD_WaitReadWriteEnd(void);

#ifdef __cplusplus
}
#endif
#endif //__SDIOLIB_H

