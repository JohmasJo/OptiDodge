#ifndef Game_H
#define Game_H

/* User Define */
#define CollisionTrue 1
#define CollisionFalse 0
#define GameEndTrue 1
#define GameEndFalse 0
#define ACTIVE 1
#define INACTIVE 0

#include <stdint.h>
#include "Pattern.h"
#include "common.h"

typedef struct{
    uint16_t player_x;
    uint16_t player_y;

    int16_t player_delta_x;
    int16_t player_delta_y;

}Player;

typedef struct{
    int16_t x;
    int16_t y;

    int8_t x_velocity; //1tick在x上的移动距离，1tick是40ms，游戏采用25fps视为1个时间刻
    int8_t y_velocity;

    uint8_t isActive;
}Ball;

typedef struct{
    Player player;
    Ball ball[MAX_BALLS];
    uint32_t score;
}Gaming_Info;

uint8_t Update_Game(Gaming_Info *info);
uCreateOut Create_New_Ball(Ball* ball);

#endif //GAME_H
