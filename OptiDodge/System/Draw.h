//
// Created by wangy on 2026/10/22.
//

/* ----------------User Define----------------*/
#ifndef OPTIDODGE_DRAW_H
#define OPTIDODGE_DRAW_H

/* ----------------User Function----------------*/
void Ball_Init(void);
void Set_Ball_D(uint8_t Diameter);
void draw_Ball(unsigned char* current_canvas, int16_t anchor_x, int8_t anchor_y);
void draw_Player(unsigned char* current_canvas, int16_t anchor_x, int8_t anchor_y);

#endif //OPTIDODGE_DRAW_H
