//
// Created by wangy on 2026/10/26.
//

#ifndef OPTIDODGE_SETTING_H
#define OPTIDODGE_SETTING_H
#include <stdint.h>
#include "common.h"

#define MAX_DIFFICULTY 5
#define MIN_DIFFICULTY 1

extern uint16_t DPI_List[];

typedef struct {
    uint8_t Ball_Size;
    uint8_t Difficulty;
    uint16_t DPI;
}Game_Setting;

extern Game_Setting Real_Game_Setting;

void Restore_Setting(void);
uCreateOut Load_Setting(void);
uCreateOut Save_Setting(void);
void Change_Ball_Size(uint8_t Ball_Size);
void Change_Difficulty(uint8_t Difficulty);
void Change_DPI(uint8_t DPI);
uint8_t Get_Setting_Ball_Size(void);

#endif //OPTIDODGE_SETTING_H
