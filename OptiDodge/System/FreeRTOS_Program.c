//
// Created by wangy on 2026/10/24.
//

/* User Includes --------------------------*/
#include "stm32f1xx_hal.h"
#include "FreeRTOS_Program.h"
#include "FreeRTOS.h"
#include "task.h"

#define START_TASK_STACK_SIZE 128
#define START_TASK_PRIORITY 1
TaskHandle_t Start_Task_handle;
void InitTask( void *pvParameters);

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
void InitTask(void *pvParameters); {
    taskENTER_CRITICAL();//进入临界区，防止初始化被中断打扰

    taskEXIT_CRITICAL();//退出临界区
    vTaskDelete(NULL);//删除自身，释放资源
}
