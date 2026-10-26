//
// Created by wangy on 2026/10/26.
//

//所有分数的排行榜占第二个扇区，起始地址0x00001000

#include "Score.h"
#include "main.h"
#include "W25Q64.h"
#include "string.h"
#define W25Q64_START_ADDRESS 0x00001000
#define SCORE_WIDTH 4

uint32_t All_Score[100] = {0};
uint32_t Count = 0;

/**
 * @brief 调整最小堆，使父节点不大于子节点
 * @param scores 分数数组
 * @param count 当前堆中的元素数量
 * @param root 开始调整的节点下标
 */
static void Score_Heap_Adjust(
    uint32_t* scores,
    uint32_t count,
    uint32_t root)
{
    // 数组下标从 0 开始：
    // 左孩子 = root * 2 + 1
    // 右孩子 = root * 2 + 2
    uint32_t child = root * 2 + 1;

    while (child < count) {
        // 左右孩子都存在时，选择较小的孩子。
        if (child + 1 < count
            && scores[child + 1] < scores[child]) {
            child++;
            }

        // 父节点已经不大于最小的孩子，不需要继续调整。
        if (scores[root] <= scores[child]) {
            break;
        }

        // 将较小的孩子换到父节点位置。
        uint32_t temp = scores[root];
        scores[root] = scores[child];
        scores[child] = temp;

        // 原来的父节点继续向下调整。
        root = child;
        child = root * 2 + 1;
    }
}

/**
 * @brief 堆排序
 * @note 只排序 All_Score 中前 Count 个元素，不写入 Flash
 */
void Sort_All_Scores(void)
{
    uint32_t i;

    if (Count > sizeof(All_Score) / sizeof(All_Score[0])) {
        return;
    }

    if (Count < 2) {
        return;
    }

    // 第一步：建立最小堆。
    // 从最后一个非叶子节点开始，逐个向前调整。
    // 使用 i > 0，避免无符号变量递减到 0 后溢出。
    for (i = Count / 2; i > 0; i--) {
        Score_Heap_Adjust(All_Score, Count, i - 1);
    }

    // 第二步：每次把堆顶的最小值移到末尾。
    // 缩小堆的范围后，重新调整堆顶。
    // 最终得到从高到低的顺序。
    for (i = Count - 1; i > 0; i--) {
        uint32_t temp = All_Score[0];
        All_Score[0] = All_Score[i];
        All_Score[i] = temp;

        Score_Heap_Adjust(All_Score, i, 0);
    }
}

/**
 * @brief 加载芯片中的所有分数
 */
void Load_All_Score(void) {
    uint32_t i;
    W25Q64_ReadData((uint8_t*)&Count, SCORE_WIDTH, W25Q64_START_ADDRESS);

    // 未保存过数据，或读出的数量异常。
    if (Count > sizeof(All_Score) / sizeof(All_Score[0])) {
        Count = 0;
    }

    memset(All_Score, 0, sizeof(All_Score));

    for (i = 0; i < Count; i++) {
        W25Q64_ReadData(
            (uint8_t*)&All_Score[i],
            SCORE_WIDTH,
            W25Q64_START_ADDRESS + SCORE_WIDTH + SCORE_WIDTH * i);
    }
    Sort_All_Scores();
}

/**
 * @brief 将内存中所有分数保存至W25Q64
 */
void Save_All_Scores(void) {
    uint32_t i = 0;
    Sort_All_Scores();
    W25Q64_SectorErase(W25Q64_START_ADDRESS);
    W25Q64_PageProgram(
        W25Q64_START_ADDRESS,
        (uint8_t *)&Count,
        SCORE_WIDTH);
    for (i = 0; i < Count; i++) {
        W25Q64_PageProgram(
            W25Q64_START_ADDRESS + SCORE_WIDTH + i * SCORE_WIDTH,
            (uint8_t*)&All_Score[i],
            4);
    }
}

void Clear_All_Scores(void) {
    memset(All_Score, 0, sizeof(All_Score));
    W25Q64_SectorErase(W25Q64_START_ADDRESS);
}

/**
 * @brief 使用二分查找定位，将新分数插入排行榜
 * @param NewScore 新分数
 * @return 1：插入成功；0：未进入排行榜或 Count 异常
 * @note 调用前，All_Score 的前 Count 项必须已经按降序排列
 */
uint8_t Insert_Score(uint32_t NewScore)
{
    uint32_t Max_Count = sizeof(All_Score) / sizeof(All_Score[0]);

    if (Count > Max_Count) {
        return 0;
    }

    // 排行榜已满时，新分数必须严格大于最后一名。
    if (Count == Max_Count) {
        if (NewScore <= All_Score[Count - 1]) {
            return 0;
        }

        // 缩小有效范围，相当于先删除最后一名。
        // 原位置的数据随后会被覆盖，不需要单独清零。
        Count--;
    }

    // 在 [left, right) 中二分查找插入位置。
    uint32_t left = 0;
    uint32_t right = Count;

    while (left < right) {
        uint32_t mid = left + (right - left) / 2;

        if (All_Score[mid] >= NewScore) {
            // 当前分数更高或相同，新分数放在它后面。
            left = mid + 1;
        }
        else {
            // 当前分数更低，新分数应放在它前面。
            right = mid;
        }
    }

    // 此时 left 就是插入位置。
    // 从后向前搬移，避免覆盖尚未移动的数据。
    for (uint32_t i = Count; i > left; i--) {
        All_Score[i] = All_Score[i - 1];
    }

    All_Score[left] = NewScore;
    Count++;

    return 1;
}