#ifndef _MILLIS_MANAGER_H
#define _MILLIS_MANAGER_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdbool.h>
#include <stdint.h>
#define HEAP_MANAGER_USE_LOG 1

#if (HEAP_MANAGER_USE_LOG == 1)
#include <stdio.h>
#define _MILLISTASK_LOG(format, ...) printf("[TASK MANAGER]" format "\r\n", ##__VA_ARGS__)
#define MILLISTASK__INFO(format, ...) _MILLISTASK_LOG("[Info] " format, ##__VA_ARGS__)
#define MILLISTASK__WARN(format, ...) _MILLISTASK_LOG("[Warn] " format, ##__VA_ARGS__)
#define MILLISTASK__ERROR(format, ...) _MILLISTASK_LOG("[Error] " format, ##__VA_ARGS__)
#else
#define HEAP_MANAGER_INFO(...)
#define HEAP_MANAGER_INFO(...)
#define HEAP_MANAGER_WARN(...)
#define HEAP_MANAGER_ERROR(...)
#endif
    typedef void (*TaskFunction_t)(void *); // 任务回调函数
    typedef struct _Task
    {
        bool State;              // Task state
        void *param;             // Task param
        TaskFunction_t Function; // Task handle
        uint32_t Time;           // Task period
        uint32_t TimePrev;       // Task last run time
        uint32_t TimeCost;       // Task time cost (us)
        uint32_t TimeError;      // Time error
        struct _Task *Next;      // Next node
    } Task_t;
    typedef struct _MillisTaskManager
    {
        Task_t *Head;        // Node head
        Task_t *Tail;        // node tail
        bool PriorityEnable; // Priority
    } MillisTaskManager;


void MillisTaskManager_Init(bool priorityEnable);

Task_t* MillisTaskManager_register(TaskFunction_t func, uint32_t timeMs, bool state,void *param);
void MillisTaskManager_Running(uint32_t tick);

#ifdef __cplusplus
}
#endif
#endif