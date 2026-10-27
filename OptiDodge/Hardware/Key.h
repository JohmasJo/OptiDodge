//
// Created by wangy on 2026/10/27.
//

#ifndef OPTIDODGE_KEY_H
#define OPTIDODGE_KEY_H

typedef enum{
    Key_None = 0,
    down = 1,
    up = 2,
    right = 3,
    left = 4
}Orientation;

Orientation Get_KeyNum(void);

#endif //OPTIDODGE_KEY_H
