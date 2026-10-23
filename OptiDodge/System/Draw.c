//
// Created by wangy on 2026/10/22.
//

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "Draw.h"
#include "Pattern.h"

/* Varieties -----------------------------------------------------------------*/
unsigned char Ball[MAX_D_CIRCLE * ((MAX_D_CIRCLE + OLED_PAGE_SIZE - 1) / OLED_PAGE_SIZE)] = {0};


void draw_Ball(unsigned char* current_canvas, int16_t anchor_x, int8_t anchor_y) {
    if (anchor_x > - BALL_WIDTH &&
        anchor_x < SCREEN_WIDTH &&
        anchor_y >- BALL_HEIGHT &&
        anchor_y < SCREEN_HEIGHT)
    {
        uint8_t y_offset;
        uint8_t Cross_Page;
        if (anchor_y < 0) {
            y_offset = OLED_PAGE_SIZE - (-anchor_y) % OLED_PAGE_SIZE;
            Cross_Page = (anchor_y + BALL_HEIGHT) / OLED_PAGE_SIZE - (anchor_y) / OLED_PAGE_SIZE + 2;
            //注意，C对于整数除法做的是截断，即只保留整数部分，对于 (int)-2 / 8 = 0
        }
        else {
            y_offset = (anchor_y) % OLED_PAGE_SIZE;
            Cross_Page = (anchor_y + BALL_HEIGHT) / OLED_PAGE_SIZE - (anchor_y) / OLED_PAGE_SIZE + 1;
        }
        int16_t index_start = SCREEN_WIDTH *(anchor_y / OLED_PAGE_SIZE) + anchor_x;//算出起始更改的屏幕元素
        uint16_t Max_Ball_Index = BALL_WIDTH * (BALL_HEIGHT / OLED_PAGE_SIZE + 1) - 1;
        for (uint8_t y = 0; y < Cross_Page; y++){
            for (uint8_t x = 0; x < BALL_WIDTH; x++) {
                int16_t canvas_index = index_start + SCREEN_WIDTH * y + x;
                if (anchor_x + x >= 0 &&
                    anchor_x + x < SCREEN_WIDTH &&
                    anchor_y + y * OLED_PAGE_SIZE >= 0 &&
                    anchor_y + y * OLED_PAGE_SIZE < SCREEN_HEIGHT)
                {
                    int16_t Former_Index = x - BALL_WIDTH + y * BALL_WIDTH;
                    int16_t Next_Index = x + y * BALL_WIDTH;
                    if (Next_Index > Max_Ball_Index) {
                        current_canvas[canvas_index] |= Ball[Former_Index] >> (OLED_PAGE_SIZE - y_offset);
                    }
                    else if (Former_Index < 0) {
                        current_canvas[canvas_index] |= (Ball[Next_Index] << y_offset);
                    }
                    else {
                        current_canvas[canvas_index] |= (Ball[Next_Index] << y_offset) | Ball[Former_Index] >> (OLED_PAGE_SIZE - y_offset);
                    }
                }
            }
        }
    }
    else {
        return;
    }

}
