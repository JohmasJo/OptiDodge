//
// Created by wangy on 2026/10/27.
//

#include "Menu.h"
#include "main.h"
#include "Key.h"
#include "oled.h"
#include "Setting.h"
#include "Score.h"
#include <stdio.h>

#include "Draw.h"
#include "Pattern.h"

MenuPage current_page = MENU_MAIN;
MenuItem selected_menu_item = ITEM_START;
SettingItem selected_setting_item = ITEM_BALL;
uint16_t rank_page = 1;
uint16_t max_page = MAX_SCORE_COUNT / SCORE_PRE_PAGE;
uint8_t DPI_Choice = 1;

CreateGame Menu_Task(void) {
    uint8_t Key = Get_KeyNum();
    switch (Key) {
        case up:
            if (selected_menu_item == ITEM_START) {
                selected_menu_item = ITEM_RANKING;
            }
            else{selected_menu_item --;}
            break;
        case down:
            if (selected_menu_item == ITEM_RANKING) {
                selected_menu_item = ITEM_START;
            }
            else{selected_menu_item ++;}
            break;
        case left:
            break;
        case right:
            switch (selected_menu_item) {
                case ITEM_START:
                    OLED_Clear();
                    current_page = MENU_PLAY;
                    return GaTrue;
                case ITEM_SETTINGS:
                    OLED_Clear();
                    current_page = MENU_SETTINGS;
                    return GaFalse;
                case ITEM_RANKING:
                    OLED_Clear();
                    current_page = MENU_RANKING;
                    return GaFalse;
            }
            break;
        case Key_None:
            break;
    }
    return GaFalse;
}

void Display_Menu(void) {
    uint8_t i;
    Set_Font_size(LARGE);
    OLED_ShowString(0,0,(uint8_t *)"Menu");
    Set_Font_size(SMALL);
    OLED_ShowString(0,2,(uint8_t *)"Play");
    OLED_ShowString(0,3,(uint8_t *)"Setting");
    OLED_ShowString(0,4,(uint8_t *)"Ranking");
    for (i = 0; i < 5; i++) {
        OLED_ShowString(64, i, (uint8_t *)"    ");
    }
    OLED_ShowString(64, selected_menu_item + 2, (uint8_t *)"<-");
}

void Display_Rank(void) {
    uint8_t i;
    uint8_t Key = Get_KeyNum();
    switch (Key) {
        case Key_None:
            break;
        case right:
            break;
        case up:
            if (rank_page == 1) {
                rank_page = 1;
            }
            else {
                rank_page --;
                OLED_Clear();
            }
            break;
        case down:
            if (rank_page == max_page) {
                rank_page = max_page;
            }
            else {
                rank_page ++;
                OLED_Clear();
            }
            break;
        case left:
            OLED_Clear();
            current_page = MENU_MAIN;
            return;
    }
    Set_Font_size(LARGE);
    OLED_ShowString(0,0,(uint8_t *)"Ranking");
    Set_Font_size(SMALL);
    for (i = 0; i < SCORE_PRE_PAGE; i++) {
        uint32_t score = All_Score[SCORE_PRE_PAGE * (rank_page - 1) + i];
        char text[20];
        snprintf(text, sizeof(text), "%3d %5lu", SCORE_PRE_PAGE * (rank_page - 1) + i + 1, (unsigned long)score);
        OLED_ShowString(0, 2+i, (uint8_t*)text);
    }

    char page[10];
    snprintf(page, sizeof(page), "%2d/%2d", rank_page, max_page);
    OLED_ShowString(0, 6, (uint8_t*)page);
}

void Display_Setting(void) {
    uint8_t i;
    uint8_t Key = Get_KeyNum();
    switch (Key) {
        case Key_None:
            break;
        case up:
            if (selected_setting_item == ITEM_BALL) {
                current_page = MENU_MAIN;
                return;
            }
            else {
                selected_setting_item --;
                OLED_Clear();
            }
            break;
        case down:
            if (selected_setting_item == ITEM_CLEAR_SCORE) {
                current_page = MENU_MAIN;
                return;
            }
            else {
                selected_setting_item ++;
                OLED_Clear();
            }
            break;
        case right:
            switch (selected_setting_item) {
                case ITEM_BALL:
                    if (Real_Game_Setting.Ball_Size == MAX_D_CIRCLE) {
                        return;
                    }
                    else {
                        Change_Ball_Size(Get_Ball_D() + 1);
                    }
                    break;
                case ITEM_DIFF:
                    if (Real_Game_Setting.Difficulty == MAX_DIFFICULTY) {
                        return;
                    }
                    else {
                        Change_Difficulty(Real_Game_Setting.Difficulty + 1);
                    }
                    break;
                case ITEM_DPI:
                    if (DPI_Choice == 3) {
                        return;
                    }
                    else {
                        DPI_Choice ++;
                        Change_DPI(DPI_Choice);
                    }
                    break;
                case ITEM_RESTORE:
                    Restore_Setting();
                    break;
                case ITEM_CLEAR_SCORE:
                    Clear_All_Scores();
                    break;
            }
            break;
        case left:
            switch (selected_setting_item) {
                case ITEM_BALL:
                    if (Real_Game_Setting.Ball_Size == MIN_D_CIRCLE) {
                        return;
                    }
                    else {
                        Change_Ball_Size(Get_Ball_D() - 1);
                    }
                    break;
                case ITEM_DIFF:
                    if (Real_Game_Setting.Difficulty == MIN_DIFFICULTY) {
                        return;
                    }
                    else {
                        Change_Difficulty(Real_Game_Setting.Difficulty - 1);
                    }
                    break;
                case ITEM_DPI:
                    if (DPI_Choice == 0) {
                        return;
                    }
                    else {
                        DPI_Choice --;
                        Change_DPI(DPI_Choice);
                    }
                    break;
                case ITEM_RESTORE:
                    return;
                case ITEM_CLEAR_SCORE:
                    return;
            }
            break;
    }
    Set_Font_size(LARGE);
    OLED_ShowString(0,0,(uint8_t *)"Setting");
    Set_Font_size(SMALL);
    OLED_ShowString(0,2,(uint8_t *)"Ball Size");
    OLED_ShowString(0,3,(uint8_t *)"Game Diff");
    OLED_ShowString(0,4,(uint8_t *)"DPI");
    OLED_ShowString(0,5,(uint8_t *)"Restore");
    OLED_ShowString(0,6,(uint8_t *)"Clear Score");
    char text[20];
    snprintf(text, sizeof(text), "%d", Real_Game_Setting.Ball_Size);
    OLED_ShowString(64,2,(uint8_t*)text);
    snprintf(text, sizeof(text), "%d", Real_Game_Setting.Difficulty);
    OLED_ShowString(64,3,(uint8_t*)text);
    snprintf(text, sizeof(text), "%4d", Real_Game_Setting.DPI);
    OLED_ShowString(64,4,(uint8_t*)text);
    for (i = 0; i < 5; i++) {
        if (i == selected_setting_item) {
            OLED_ShowString(108, i+2, (uint8_t*)"<-");
        }
        else {
            OLED_ShowString(108, i+2, (uint8_t*)" ");
        }
    }
}
