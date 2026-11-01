//
// Created by wangy on 2026/10/26.
//

#ifndef OPTIDODGE_SCORE_H
#define OPTIDODGE_SCORE_H
#include <stdint.h>

#define W25Q64_START_ADDRESS 0x00001000
#define SCORE_WIDTH 4
#define MAX_SCORE_COUNT 100

extern uint32_t All_Score[MAX_SCORE_COUNT];
extern uint8_t Score_Count;

void Load_All_Score(void);
void Save_All_Scores(void);
void Clear_All_Scores(void);
uint8_t Insert_Score(uint32_t NewScore);

#endif //OPTIDODGE_SCORE_H
