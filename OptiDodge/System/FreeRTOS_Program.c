//
// Created by wangy on 2026/10/24.
//

/* User Includes --------------------------*/
#include "stm32f1xx_hal.h"
#include "FreeRTOS_Program.h"
#include "FreeRTOS.h"
#include "Game.h"
#include "oled.h"
#include "task.h"
#include "timers.h"
#include "event_groups.h"
#include "../Interface/Menu.h"
#include "PAW3327.h"
#include <stdio.h>
#include <string.h>

#include "Draw.h"
#include "queue.h"
#include "Score.h"
#include "Setting.h"

#define GAME_OVER_BIT       (1U << 0)
#define BALL_STOPPED_BIT    (1U << 1)
#define DRAW_STOPPED_BIT    (1U << 2)

#define START_TASK_STACK_SIZE 128
#define START_TASK_PRIORITY 1
TaskHandle_t Start_Task_handle;
void InitTask( void *pvParameters);

#define MENU_TASK_STACK_SIZE 128
#define MENU_TASK_PRIORITY 1
TaskHandle_t Menu_Task_handle;
void MenuTask( void *pvParameters);

#define GAME_TASK_STACK_SIZE 128
#define GAME_TASK_PRIORITY 1
TaskHandle_t Game_Task_handle;
void GameTask( void *pvParameters);

#define GAME_DRAW_TASK_STACK_SIZE 256
#define GAME_DRAW_TASK_PRIORITY 1
TaskHandle_t Game_Draw_Task_handle;
void GameDrawTask( void *pvParameters);

#define BALL_CREATE_TASK_STACK_SIZE 128
#define BALL_CREATE_TASK_PRIORITY 1
TaskHandle_t Ball_Create_Task_handle;
void BallCreateTask( void *pvParameters);

xTimerHandle Game_Tick_Handle;
void Game_Tick_Callback(TimerHandle_t xTimer);

xTimerHandle Ball_Create_handle;
void Ball_Create_Callback(TimerHandle_t xTimer);

#define BALL_CREATE_QUEUE_LENGTH 2
QueueHandle_t Ball_Create_Queue_Handle;

#define GAME_INFO_QUEUE_LENGTH 2
QueueHandle_t Game_Info_Queue_Handle;

EventGroupHandle_t Game_Over_Event_Group;

/**
 * @brief 初始化RTOS，唤醒初始任务
 */
void RTOS_Init(void) {
    //创建初始化任务
    xTaskCreate((TaskFunction_t)InitTask,
        (char *)"InitTask",
        (uint16_t)START_TASK_STACK_SIZE,
        (void *)NULL,
        (UBaseType_t)START_TASK_PRIORITY,
        (TaskHandle_t *)&Start_Task_handle);
    vTaskStartScheduler();//启动调度器
}

/**
 * 初始化任务，负责初始化所有的
 * @param pvParameters 任务传参列表
 */
void InitTask(void *pvParameters) {
    taskENTER_CRITICAL();//进入临界区，防止初始化被中断打扰
    Load_Setting();
    Set_Ball_D(Get_Setting_Ball_Size());
    Load_Setting();
    Ball_Create_Queue_Handle = xQueueCreate(
        BALL_CREATE_QUEUE_LENGTH,
        sizeof(Ball));
    Game_Info_Queue_Handle = xQueueCreate(
        GAME_INFO_QUEUE_LENGTH,
        sizeof(Gaming_Info));
    Game_Over_Event_Group = xEventGroupCreate();
    xTaskCreate((TaskFunction_t)MenuTask,
        (char *)"Menu_Task",
        (uint16_t)MENU_TASK_STACK_SIZE,
        (void *)NULL,
        (UBaseType_t)MENU_TASK_PRIORITY,
        (TaskHandle_t *)&Menu_Task_handle);
    taskEXIT_CRITICAL();//退出临界区
    vTaskDelete(NULL);//删除自身，释放资源
}

void MenuTask( void *pvParameters) {
    OLED_Clear();
    Set_Page(MENU_MAIN);
    MenuPage Last_page = MENU_MAIN;
    while (1) {
		CreateGame isStart;
        MenuPage Now_page = Get_Page();
        if (Now_page != Last_page) {
            OLED_Clear();
        }
        if (Last_page == MENU_SETTINGS && Now_page != MENU_SETTINGS) {
            Save_Setting();
        }
        if (Last_page != MENU_RANKING && Now_page == MENU_RANKING) {
            Load_All_Score();
        }
        Last_page = Now_page;
        switch (Get_Page()) {
            case MENU_MAIN:
                isStart = Display_Menu();
                if (isStart == GaTrue) {
                    taskENTER_CRITICAL();
                    xTaskCreate(
                        (TaskFunction_t)GameTask,
                        (char *)"Game_Task",
                        (uint16_t)GAME_TASK_STACK_SIZE,
                        (void *)NULL,
                        (UBaseType_t)GAME_TASK_PRIORITY,
                        (TaskHandle_t *)&Game_Task_handle);
                    xTaskCreate(
                        (TaskFunction_t)GameDrawTask,
                        (char *)"Game_Draw_Task",
                        (uint16_t)GAME_DRAW_TASK_STACK_SIZE,
                        (void *)NULL,
                        (UBaseType_t)GAME_DRAW_TASK_PRIORITY,
                        (TaskHandle_t *)&Game_Draw_Task_handle);
                    xTaskCreate(
                        (TaskFunction_t)BallCreateTask,
                        (char *)"Ball_Create_Task",
                        (uint16_t)BALL_CREATE_TASK_STACK_SIZE,
                        (void *)NULL,
                        (UBaseType_t)BALL_CREATE_TASK_PRIORITY,
                        (TaskHandle_t *)&Ball_Create_Task_handle
                        );
                    Game_Tick_Handle = xTimerCreate(
                        "Game_Tick",
                        (TickType_t)40,
                        pdTRUE,
                        (void *)1,
                        Game_Tick_Callback
                        );
                    taskEXIT_CRITICAL();
                    vTaskDelete(NULL);
                }
                else {
                    continue;
                }
                break;
            case MENU_RANKING:
                Display_Rank();
                break;
            case MENU_SETTINGS:
                Display_Setting();
                break;
        }
    }
}

void GameTask( void *pvParameters) {
    Gaming_Info All_Info = {0};
    UBaseType_t isCreated = errQUEUE_EMPTY;
    Ball New_Ball = {0};
    uint8_t isOver = GameEndFalse;
    uint8_t i;
    //传感器会积累之前的数值，先读取一次
    PAW3327_ReadMotion(&All_Info.player.player_delta_y, &All_Info.player.player_delta_x);
    All_Info.player.player_delta_y = 0;
    All_Info.player.player_delta_x = 0;
    srand((unsigned int)HAL_GetTick());
    All_Info.player.player_x = SCREEN_WIDTH / 2;
    All_Info.player.player_y = SCREEN_HEIGHT / 2;
    Create_New_Ball(&All_Info.ball[0]);
    xTimerStart(Game_Tick_Handle, portMAX_DELAY);
    while (1) {
        ulTaskNotifyTake(pdTRUE,(TickType_t)50);
        isCreated = xQueueReceive(Ball_Create_Queue_Handle, &New_Ball, 0);
        if (isCreated == pdPASS) {
            for (i = 0; i < MAX_BALLS; i++) {
                if (All_Info.ball[i].isActive == INACTIVE) {
                    /* 同类型结构体可以直接赋值 */
                    All_Info.ball[i] = New_Ball;
                    All_Info.ball[i].isActive = ACTIVE;
                    /* 一个新球只占用一个槽 */
                    break;
                }
            }
        }
        PAW3327_ReadMotion(&All_Info.player.player_delta_y, &All_Info.player.player_delta_x);
        All_Info.player.player_delta_x = -All_Info.player.player_delta_x;
        uint16_t DPI = Real_Game_Setting.DPI;
        int32_t Move_X = (int32_t)All_Info.player.player_delta_x* DPI / (1500L * 20);
        int32_t Move_Y = (int32_t)All_Info.player.player_delta_y* DPI / (1500L * 20);
        // if (Move_X > 4) {
        //     Move_X = 4;
        // }
        // else if (Move_X < -4) {
        //     Move_X = -4;
        // }
        // if (Move_Y > 4) {
        //     Move_Y = 4;
        // }
        // else if (Move_Y < -4) {
        //     Move_Y = -4;
        // }
        All_Info.player.player_delta_x = (int16_t)Move_X;
        All_Info.player.player_delta_y = (int16_t)Move_Y;
        isOver = Update_Game(&All_Info);
        xQueueSend(
            Game_Info_Queue_Handle,
            &All_Info,
            portMAX_DELAY);
        if (isOver == GameEndTrue) {
            Insert_Score(All_Info.score);
            Save_All_Scores();
            xEventGroupSetBits(Game_Over_Event_Group, GAME_OVER_BIT);
            xEventGroupWaitBits(
                Game_Over_Event_Group,
                BALL_STOPPED_BIT | DRAW_STOPPED_BIT,
                pdTRUE,
                pdTRUE,
                portMAX_DELAY);
            xEventGroupClearBits(Game_Over_Event_Group, GAME_OVER_BIT);
            xTimerDelete(Game_Tick_Handle, portMAX_DELAY);
            xTaskCreate((TaskFunction_t)MenuTask,
                (char *)"Menu_Task",
                (uint16_t)MENU_TASK_STACK_SIZE,
                (void *)NULL,
                (UBaseType_t)MENU_TASK_PRIORITY,
                (TaskHandle_t *)&Menu_Task_handle);
            vTaskDelete(NULL);
            break;
        }
    }
}

void GameDrawTask( void *pvParameters){
    static uint8_t Canvas[SCREEN_WIDTH * SCREEN_HEIGHT / OLED_PAGE_SIZE] = {0};
    Ball_Init();
    while (1) {
        Gaming_Info All_Info = {0};
        uint8_t i;
        memset(Canvas, 0, sizeof(Canvas));
        UBaseType_t isUpdated = errQUEUE_EMPTY;
        isUpdated = xQueueReceive(
            Game_Info_Queue_Handle,
            &All_Info,
            TIME_PRE_TICKS + 20);
        if ((xEventGroupGetBits(Game_Over_Event_Group) & GAME_OVER_BIT) != 0) {
            Set_Font_size(LARGE);
            OLED_ShowString(20, 4, (u8 *)"GAME OVER!!!");
            vTaskDelay(3000);
            xEventGroupSetBits(Game_Over_Event_Group, DRAW_STOPPED_BIT);
            vTaskDelete(NULL);
            break;
        }
        if (isUpdated == errQUEUE_EMPTY) {
            continue;
        }
        draw_Player(Canvas, All_Info.player.player_x, All_Info.player.player_y);
        for (i = 0; i < MAX_BALLS; i++) {
            if (All_Info.ball[i].isActive == ACTIVE) {
                draw_Ball(Canvas, All_Info.ball[i].x - Get_Ball_D() / 2, All_Info.ball[i].y - Get_Ball_D() / 2);
            }
        }
        OLED_DrawBMP(0,0,127,7,Canvas);
        OLED_ShowScore(All_Info.score);
    }
}

void BallCreateTask( void *pvParameters) {
    TickType_t Spawn_Time;
    Spawn_Time = pdMS_TO_TICKS(rand() % (MAX_SPAWN_TIME - MIN_SPAWN_TIME) + MIN_SPAWN_TIME);
    Ball_Create_handle = xTimerCreate(
        "Ball_Create_Task",
        Spawn_Time,
        pdFALSE,
        (void *)2,
        Ball_Create_Callback);
    if (Ball_Create_handle == NULL) {
        xEventGroupSetBits(Game_Over_Event_Group, BALL_STOPPED_BIT);
        vTaskDelete(NULL);
        return;
    }
    xTimerStart(Ball_Create_handle, portMAX_DELAY);
    while (1) {
        uint32_t notify;
        Ball New_Ball = {0};
        notify = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50));
        if ((xEventGroupGetBits(Game_Over_Event_Group) & GAME_OVER_BIT) != 0) {
            xTimerDelete(Ball_Create_handle, portMAX_DELAY);
            Ball_Create_handle = NULL;
            xEventGroupSetBits(Game_Over_Event_Group, BALL_STOPPED_BIT);
            vTaskDelete(NULL);
            break;
        }
        /* 超时只检查退出条件，不生成小球 */
        if (notify == 0) {
            continue;
        }
        Create_New_Ball(&New_Ball);
        /* 队列满时放弃本次小球，避免任务一直堵在这里 */
        xQueueSend(Ball_Create_Queue_Handle, &New_Ball, 0);
        Spawn_Time = pdMS_TO_TICKS(rand() % (MAX_SPAWN_TIME - MIN_SPAWN_TIME) + MIN_SPAWN_TIME);
        /* 修改同一个定时器的周期，同时启动下一轮计时 */
        xTimerChangePeriod(Ball_Create_handle, Spawn_Time, portMAX_DELAY);
    }
}

void Game_Tick_Callback(TimerHandle_t xTimer) {
    xTaskNotifyGive(Game_Task_handle);
}

void Ball_Create_Callback(TimerHandle_t xTimer) {
    xTaskNotifyGive(Ball_Create_Task_handle);
}
