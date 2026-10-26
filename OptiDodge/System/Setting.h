//
// Created by wangy on 2026/10/26.
//

#ifndef OPTIDODGE_SETTING_H
#define OPTIDODGE_SETTING_H
#include "main.h"

typedef enum{CrTrue = 1, CrFalse = 0} uCreateOut;

void Restore_Setting(void);
uCreateOut Load_Setting(void);
uCreateOut Save_Setting(void);
void Change_Ball_Size(uint8_t Ball_Size);
void Change_Difficulty(uint8_t Difficulty);
void Change_DPI(uint8_t DPI);

#endif //OPTIDODGE_SETTING_H
