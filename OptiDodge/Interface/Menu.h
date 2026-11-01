//
// Created by wangy on 2026/10/27.
//

#ifndef OPTIDODGE_MENU_H
#define OPTIDODGE_MENU_H

#define SCORE_PRE_PAGE 4
typedef enum
{
    MENU_MAIN,       // 主菜单
    MENU_PLAY,       // 游戏页面
    MENU_SETTINGS,   // 设置页面
    MENU_RANKING     // 排行榜页面
} MenuPage;

typedef enum
{
    ITEM_START,
    ITEM_SETTINGS,
    ITEM_RANKING,
} MenuItem;

typedef enum {
    ITEM_BALL,
    ITEM_DIFF,
    ITEM_DPI,
    ITEM_RESTORE,
    ITEM_CLEAR_SCORE
} SettingItem;

typedef enum {
    GaFalse = 0,
    GaTrue = 1
}CreateGame;

CreateGame Display_Menu(void);
void Display_Rank(void);
void Display_Setting(void);
MenuPage Get_Page(void);
void Set_Page(MenuPage page);

#endif //OPTIDODGE_MENU_H
