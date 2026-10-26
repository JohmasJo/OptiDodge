//
// Created by wangy on 2026/10/26.
//

#ifndef OPTIDODGE_W25Q64_H
#define OPTIDODGE_W25Q64_H
#include "main.h"

void W25Q24_GetID(uint8_t* MID, uint16_t* DID);
void W25Q64_PageProgram(uint32_t Address, uint8_t *DataArray,uint16_t Count);
void W25Q64_SectorErase(uint32_t Address);
void W25Q64_ReadData(uint8_t *ReceiveData, uint32_t Count, uint32_t Address);

#endif //OPTIDODGE_W25Q64_H
