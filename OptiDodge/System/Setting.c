//
// Created by wangy on 2026/10/26.
//

#include "Setting.h"
#include "W25Q64.h"
#include "string.h"
#include "Setting_Index.h"
#include "Draw.h"

#define W25Q64_START_ADDRESS 0x00000000
#define SETTING_WIDTH_1 1
#define SETTING_WIDTH_2 2
#define SETTING_COUNT 3

typedef struct {
    uint8_t Ball_Size;
    uint8_t Difficulty;
    uint16_t DPI;
}Game_Setting;

Game_Setting Default_Game_Setting = {
    DEFAULT_BALL_SIZE,
    DEFAULT_DIFFICULTY,
    DEFAULT_DPI
};

Game_Setting Real_Game_Setting = {0};

/**
 * @brief 恢复默认设置
 */
void Restore_Setting(void) {
    memcpy(&Real_Game_Setting, &Default_Game_Setting, sizeof(Game_Setting));
}

/**
 * @brief 读取已有的设置
 * @return 是否读取成功
 */
uCreateOut Load_Setting(void) {
    uint8_t i;
    for (i = 0; i < SETTING_COUNT; i++) {
        switch (i) {
            case BALL_SIZE_Ind:
                W25Q64_ReadData(&Real_Game_Setting.Ball_Size, SETTING_WIDTH_1, W25Q64_START_ADDRESS);
                break;

            case DIFFICULTY_Ind:
                W25Q64_ReadData(&Real_Game_Setting.Difficulty, SETTING_WIDTH_1, W25Q64_START_ADDRESS + SETTING_WIDTH_1);
                break;

            case DPI_Ind:
                W25Q64_ReadData((uint8_t*)&Real_Game_Setting.DPI, SETTING_WIDTH_2, W25Q64_START_ADDRESS + 2 * SETTING_WIDTH_1);
                break;

            default:
                return CrFalse;
        }
    }
    return CrTrue;
}

/**
 * @brief 保存目前的设置
 * @return 是否读取成功
 */
uCreateOut Save_Setting(void) {
    uint8_t i;
    W25Q64_SectorErase(W25Q64_START_ADDRESS);
    for (i = 0; i < SETTING_COUNT; i++) {
        switch (i) {
            case BALL_SIZE_Ind:
                W25Q64_PageProgram(W25Q64_START_ADDRESS, &Real_Game_Setting.Ball_Size, SETTING_WIDTH_1);
                break;

            case DIFFICULTY_Ind:
                W25Q64_PageProgram(W25Q64_START_ADDRESS + SETTING_WIDTH_1, &Real_Game_Setting.Difficulty, SETTING_WIDTH_1);
                break;

            case DPI_Ind:
                W25Q64_PageProgram(W25Q64_START_ADDRESS + 2 * SETTING_WIDTH_1, (uint8_t*)&Real_Game_Setting.DPI, SETTING_WIDTH_2);
                break;

            default:
                return CrFalse;
        }
    }
    return CrTrue;
}

/**
 * @brief 设置球的直径
 * @param Ball_Size 设置的球直径
 */
void Change_Ball_Size(uint8_t Ball_Size) {
    Real_Game_Setting.Ball_Size = Ball_Size;
    Set_Ball_D(Ball_Size);
}

/**
 * @brief 设置游戏难度档位
 * @param Difficulty 代表游戏难度
 */
void Change_Difficulty(uint8_t Difficulty) {
    Real_Game_Setting.Difficulty = Difficulty;
}

/**
 * @brief 设置光学传感器DPI
 * @param DPI 目标DPI值
 */
void Change_DPI(uint8_t DPI) {
    Real_Game_Setting.DPI = DPI;
}