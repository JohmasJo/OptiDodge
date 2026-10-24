//
// Created by wangy on 2026/10/24.
//

/* Includes */
#include "stm32f1xx_hal.h"
#include "Draw.h"
#include "Game.h"
#include "Pattern.h"

/* Varieties */


/* Structure */
typedef struct{
    uint16_t player_x;
    uint16_t player_y;

    int16_t player_delta_x;
    int16_t player_delta_y;

}Player;

typedef struct{
    int16_t x;
    int16_t y;

    uint8_t x_velocity; //1tick在x上的移动距离，1tick是40ms，游戏采用25fps视为1个时间刻
    uint8_t y_velocity;

    uint8_t isActive;
}Ball;

typedef struct{
    Player player;
    Ball ball[MAX_BALLS];
    uint32_t score;
}Gaming_Info;

/* Functions*/
/**
 * @brief 输入小球和玩家信息，以计算是否会发生碰撞
 * @param ball 单个小球的结构体
 * @param player 玩家数据结构体
 * @return 玩家是否与小球碰撞
 */
uint8_t Collision_Detect(Ball* ball, Player* player) {
    if (!ball->isActive) {
        return CollisionFalse;
    }
    int8_t Ball_radius = Get_Ball_D() / 2;

    //计算初始的小球相对于玩家的坐标，以玩家为原点
    int16_t start_x = ball->x - player->player_x;
    int16_t start_y = ball->y - player->player_y;

    //计算小球在这一时间刻内相对于玩家移动的长度
    int16_t move_x = ball->x_velocity - player->player_delta_x;
    int16_t move_y = ball->y_velocity - player->player_delta_y;

    //计算两个物体间的最小允许距离：将两者的半径进行相加
    int16_t radius_sum = PLAYER_RADIUS + Ball_radius / 2;
    //使用乘方来防止进行开方运算
    uint32_t min_distance_squared = (int32_t)radius_sum * radius_sum;
    uint32_t start_distance_squared = (int32_t)start_x * start_x + (int32_t)start_y * start_y;
    uint32_t move_distance_squared = (int32_t)move_x * move_x + (int32_t)move_y * move_y;
    
    if (start_distance_squared <= min_distance_squared) {
        return CollisionTrue;//当开始的时候距离已经小于最小距离，就说明已经碰上了，根本不用算
    }
    
    //此处为向量-OA·AB
    int32_t projection = - (((int32_t)start_x * move_x) + ((int32_t)start_y * move_y));

    //若结果为负数，则小球远离玩家，刚刚算的结果就是最近距离
    if (projection <= 0) {
        return CollisionFalse;
    }
    
    //若结果为正数，则小球先靠近玩家，要进一步计算最短距离
    //向量点积大于相对位移的模的平方 -> 末端未到达直线最靠近原点那一点，取末尾位置的距离判断
    if (projection >= move_distance_squared) {
        //计算小球的末尾位置
        int16_t end_x = start_x + move_x;
        int16_t end_y = start_y + move_y;

        uint32_t end_distance_squared = (int32_t)end_x * end_x + end_y * end_y;
        if (end_distance_squared < min_distance_squared) {
            return CollisionTrue;
        }
        else {
            return CollisionFalse;
        }
    }

    //向量点积小于相对位移模的平方 -> 最短距离就是直线距离原点最近那一点
    else {
        /*
         *cross: (start_x, start_y)x(move_x, move_y)，使用矢量叉乘（行列式的几何意义）来计算2*面积
         *大小等效于移动距离乘上原点到直线距离，将其与最小距离和移动距离的乘积相比，比大小
         * 最终得到的结果就是点到直线距离和最小距离进行比较*/
        int64_t cross = (int64_t)start_x * move_y - (int64_t)start_y * move_x;
        if (cross * cross <= min_distance_squared * move_distance_squared) {
            return CollisionTrue;
        }
        else {
            return CollisionFalse;
        }
    }
    
    return CollisionFalse;
}