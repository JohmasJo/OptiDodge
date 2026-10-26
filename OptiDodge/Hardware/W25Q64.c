//
// Created by wangy on 2026/10/26.
//

#include "W25Q64.h"
#include "main.h"
#include "W25Q64_Ins.h"

extern SPI_HandleTypeDef hspi2;

/**
 * @brief 拉低对应从机片选电频，使能芯片
 */
void W25Q64_Enable(void) {
    HAL_GPIO_WritePin(PAW3327_NCS_GPIO_Port, PAW3327_NCS_Pin,GPIO_PIN_SET);
    HAL_GPIO_WritePin(W25Q64_NCS_GPIO_Port, W25Q64_NCS_Pin,GPIO_PIN_RESET);
}

/**
 * @brief 拉高对应芯片片选电平，失能芯片
 */
void W25Q64_Disable(void) {
    HAL_GPIO_WritePin(W25Q64_NCS_GPIO_Port, W25Q64_NCS_Pin,GPIO_PIN_SET);
}

/**
 * @brief 模拟SPI发送过程：双向循环移位发送
 * @param SendData 要发送给从机的数据
 * @return 接收来自从机的数据
 */
uint8_t W25Q64_SwapByte(uint8_t SendData) {
    uint8_t ReceiveData;
    HAL_SPI_TransmitReceive(&hspi2, &SendData, &ReceiveData, 1, 100);
    return  ReceiveData;
}

/**
 * @brief 获得设备ID
 * @param MID 设备MID
 * @param DID 设备DID
 */
void W25Q24_GetID(uint8_t* MID, uint16_t* DID){//使用指针传参，传输8位设备id和16位的厂商id
    W25Q64_Enable();
    W25Q64_SwapByte(W25Q64_JEDEC_ID);//发送指令，设备收到该指令后返回两个id（时序见手册）
    *MID = W25Q64_SwapByte(W25Q64_DUMMY_BYTE);//接收第一个字节，只需接收，所以返回一个无用字节0xff
    *DID = W25Q64_SwapByte(W25Q64_DUMMY_BYTE);//先接受高八位
    *DID <<= 8;
    *DID |= W25Q64_SwapByte(W25Q64_DUMMY_BYTE);//接收低八位，使用|=赋值替换低位的0
    W25Q64_Disable();
}

/**
 * @brief 发送指令，使得芯片写使能
 */
void W25Q64_WriteEnable(void){
    W25Q64_Enable();
    W25Q64_SwapByte(W25Q64_WRITE_ENABLE);
    W25Q64_Disable();
}

/**
 * @brief 等待W25Q64忙
 */
void W25Q64_WaitBusy(void){//读状态寄存器1，看最低位，是否设备在忙
    uint32_t Timeout;
    W25Q64_Enable();
    W25Q64_SwapByte(W25Q64_READ_STATUS_REGISTER_1);
    while((W25Q64_SwapByte(W25Q64_DUMMY_BYTE) & 0x01) == 0x01){
        Timeout --;
        if(Timeout == 0){break;}//等待忙
    }
    W25Q64_Disable();
}

void W25Q64_PageProgram(uint32_t Address, uint8_t *DataArray,uint16_t Count){
    uint16_t i;
    W25Q64_WriteEnable();//写使能
    W25Q64_Enable();
    W25Q64_SwapByte(W25Q64_PAGE_PROGRAM);
    W25Q64_SwapByte(Address >> 16);
    W25Q64_SwapByte(Address >> 8);
    W25Q64_SwapByte(Address);//发送24位地址
    for(i = 0; i < Count; i++ ){
        W25Q64_SwapByte(DataArray[i]);//在扇区内逐项写入数据
    }
    W25Q64_Disable();	
    W25Q64_WaitBusy();//事后等待，若是在事前等待，则读类型指令也要先等芯片不忙时再进行读
}//写操作不可跨页，若出现跨页操作，如0FF开始写，则会自动回到该页的开端覆写，即继续从000开始写而不是100

void W25Q64_SectorErase(uint32_t Address){//清除该地址所在扇区的所有数据
    W25Q64_WriteEnable();//写使能
    W25Q64_Enable();
    W25Q64_SwapByte(W25Q64_SECTOR_ERASE_4KB);
    W25Q64_SwapByte(Address >> 16);
    W25Q64_SwapByte(Address >> 8);
    W25Q64_SwapByte(Address);//发送24位地址
    W25Q64_Disable();
    W25Q64_WaitBusy();
}

void W25Q64_ReadData(uint8_t *ReceiveData, uint32_t Count, uint32_t Address){
    uint32_t i;
    W25Q64_Enable();
    W25Q64_SwapByte(W25Q64_READ_DATA);
    W25Q64_SwapByte(Address >> 16);
    W25Q64_SwapByte(Address >> 8);
    W25Q64_SwapByte(Address);//发送24位地址
    for(i = 0; i < Count; i++){
        ReceiveData[i] = W25Q64_SwapByte(W25Q64_DUMMY_BYTE);
    }
    W25Q64_Disable();	
}//读操作可跨页

