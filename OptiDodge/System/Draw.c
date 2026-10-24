//
// Created by wangy on 2026/10/22.
//

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "Draw.h"
#include <string.h>
#include "Pattern.h"

/* Structure -----------------------------------------------------------------*/


/* Varieties -----------------------------------------------------------------*/
/*设置单个小球图样的数组，以及定义小球的直径*/
uint8_t D_CIRCLE = 7;
static unsigned char Ball[MAX_D_CIRCLE * ((MAX_D_CIRCLE + OLED_PAGE_SIZE - 1) / OLED_PAGE_SIZE)] = {0};
static unsigned char Player[] = {0x04, 0x04, 0x1F, 0x04, 0x04};

/* Functions -----------------------------------------------------------------*/
/**
 * @brief 绘制圆的时候在圆内落笔
 * @param x 落笔的x坐标（0~127）
 * @param y 落笔的y坐标（0~63）
 */
void Ball_draw_point(uint16_t x, uint8_t y) {
    if (x>=0 && x< BALL_WIDTH
        && y>=0 && y < BALL_HEIGHT) {
        uint16_t point_index = (y / OLED_PAGE_SIZE) * BALL_WIDTH +x;
        Ball[point_index] |= 0x01 << (y % OLED_PAGE_SIZE);
    }
}

/**
 * @brief 使用上方的接口，使用Bresenham算法进行圆形的绘制
 */
void Ball_Init(void) {
    memset(Ball, 0, sizeof(Ball));//清空球图案
    //找到球的圆心
    int16_t Center_Low = (D_CIRCLE - 1) / 2;
    int16_t Center_High = (D_CIRCLE) / 2;
    uint8_t Is_Even = (D_CIRCLE % 2 == 0);

    int16_t x = 0;
    int16_t y = Center_Low;
    //初始化误差值d，使用Is_Even分别计算两种情况下的误差值
    int32_t d = 3 - 2 * y + 2 * Is_Even;

    //Bresenham算法的思想：为了节省资源，使用圆的对称性进行绘图，实际只需要计算1/8的点的坐标即可
    while (x <= y) {
        Ball_draw_point(Center_High + x, Center_Low + y);
        Ball_draw_point(Center_High - x, Center_Low + y);
        Ball_draw_point(Center_High + x, Center_Low - y);
        Ball_draw_point(Center_High - x, Center_Low - y);

        //交换x和y得到剩下的四个点坐标
        Ball_draw_point(Center_High + y, Center_Low + x);
        Ball_draw_point(Center_High - y, Center_Low + x);
        Ball_draw_point(Center_High + y, Center_Low - x);
        Ball_draw_point(Center_High - y, Center_Low - x);

        /*开始更新，
         *Bresenham的思想就是：查看误差函数d的值，
         *      如果<0说明在圆内，则只要向右移动；
         *      如果>0说明在园外，则需要向右下移动来逼近圆，
         *      由于只画1/8个圆，所以在结束的时候对应圆的斜率也只是-1，不会追不上
         *      以下代码中的表达式为提前推导，对比标准Bresenham算法，其中Delta  = 2 * p + 1
         */
        if (d < 0) {
            d += 4 * x + 6 + 2 * (Is_Even);
            x ++;
        }
        else {
            d += 4 * ( x - y ) + 10;
            x ++;
            y --;
        }


    }
}

/**
 * @brief 设置圆的大小并重新初始化
 * @param Diameter 新设置的圆的直径
 */
void Set_Ball_D(uint8_t Diameter){
	D_CIRCLE = Diameter;
	Ball_Init();
}

/**
 * @brief 将设置圆的直径暴露给外部程序
 * @return 输出程序内的小球直径
 */
uint8_t Get_Ball_D(void) {
    return D_CIRCLE;
}

/**
 * @brief 将小球画到传入的画布上
 * @param current_canvas 要绘制的画布，此处传入画布地址
 * @param anchor_x 小球的左上角坐标x
 * @param anchor_y 小球的左上角坐标y
 */
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

/**
 * @brief 将玩家绘制到传入的画布上
 * @param current_canvas 要绘制的画布，此处传入画布地址
 * @param Player_x 玩家中心所在的x坐标
 * @param Player_y 玩家中心所在的y坐标
 */
void draw_Player(unsigned char* current_canvas, int16_t Player_x, int8_t Player_y) {
    if (Player_x > - PLAYER_WIDTH &&
        Player_x < SCREEN_WIDTH &&
        Player_y >- PLAYER_HEIGHT &&
        Player_y < SCREEN_HEIGHT)
    {
        int16_t anchor_x = Player_x - PLAYER_HEIGHT / 2;
        int16_t anchor_y = Player_y - PLAYER_WIDTH / 2;
        uint8_t y_offset;
        uint8_t Cross_Page;
        if (anchor_y < 0) {
            y_offset = OLED_PAGE_SIZE - (-anchor_y) % OLED_PAGE_SIZE;
            Cross_Page = (anchor_y + PLAYER_HEIGHT) / OLED_PAGE_SIZE - (anchor_y) / OLED_PAGE_SIZE + 2;
            //注意，C对于整数除法做的是截断，即只保留整数部分，对于 (int)-2 / 8 = 0
        }
        else {
            y_offset = (anchor_y) % OLED_PAGE_SIZE;
            Cross_Page = (anchor_y + PLAYER_HEIGHT) / OLED_PAGE_SIZE - (anchor_y) / OLED_PAGE_SIZE + 1;
        }
        int16_t index_start = SCREEN_WIDTH *(anchor_y / OLED_PAGE_SIZE) + anchor_x;//算出起始更改的屏幕元素
        uint16_t Max_Player_Index = PLAYER_WIDTH * (PLAYER_HEIGHT / OLED_PAGE_SIZE + 1) - 1;
        for (uint8_t y = 0; y < Cross_Page; y++){
            for (uint8_t x = 0; x < PLAYER_WIDTH; x++) {
                int16_t canvas_index = index_start + SCREEN_WIDTH * y + x;
                if (anchor_x + x >= 0 &&
                    anchor_x + x < SCREEN_WIDTH &&
                    anchor_y + y * OLED_PAGE_SIZE >= 0 &&
                    anchor_y + y * OLED_PAGE_SIZE < SCREEN_HEIGHT)
                {
                    int16_t Former_Index = x - PLAYER_WIDTH + y * PLAYER_WIDTH;
                    int16_t Next_Index = x + y * PLAYER_WIDTH;
                    if (Next_Index > Max_Player_Index) {
                        current_canvas[canvas_index] |= Player[Former_Index] >> (OLED_PAGE_SIZE - y_offset);
                    }
                    else if (Former_Index < 0) {
                        current_canvas[canvas_index] |= (Player[Next_Index] << y_offset);
                    }
                    else {
                        current_canvas[canvas_index] |= (Player[Next_Index] << y_offset) | Player[Former_Index] >> (OLED_PAGE_SIZE - y_offset);
                    }
                }
            }
        }
    }
    else {
        return;
    }
}
