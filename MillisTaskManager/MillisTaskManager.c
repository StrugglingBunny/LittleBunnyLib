/*
 * \file   MillisTaskManager.c
 * \brief  MillisTaskManager
 *
 * - Responsible Engineer:Jeffer Chen
 *
 * - Description: This file implments MillisTaskManager
 *
 *   Modify from C++ vertion (by _FASTSHEET) to
 * - Created on: Feb 12, 2024
 * - Author: Jeffer Chen
 * - GitHub:StrugglingBunny
 * - Component needed
 *             HeapManager
 */
#include "MillisTaskManager.h"
#include "HeapManager.h"

#define TASK_NEW(task)                                       \
    do                                                       \
    {                                                        \
        task = (Task_t *)HeapManager_malloc(sizeof(Task_t)); \
        memset(task, 0, sizeof(Task_t));                     \
    } while (0)
#define TASK_DEL(task)          \
    do                          \
    {                           \
        HeapManager_free(task); \
    } while (0)

static Task_t *MillisTaskManager_findTask(TaskFunction_t func);

static MillisTaskManager *TaskManager = NULL;

/**
 * @brief  Initlize the millis task manager
 * @param  priorityEnable
 * @retval 无
 */
void MillisTaskManager_Init(bool priorityEnable)
{
    TaskManager = (MillisTaskManager *)HeapManager_malloc(sizeof(MillisTaskManager));
    if (TaskManager)
    {
        TaskManager->Head = NULL;
        TaskManager->Tail = NULL;
        TaskManager->PriorityEnable = priorityEnable;
    }
}
/**
 * @brief  Delete the millis Task manager
 * @param  无
 * @retval 无
 */
void MillisTaskManager_DeInit()
{
    Task_t *now = TaskManager->Head;
    while (true)
    {
        if (now == NULL)
            break;
        Task_t *now_del = now;
        now = now->Next;
        TASK_DEL(now_del);
    }
    if (TaskManager)
    {
        HeapManager_free(TaskManager);
    }
}
/**
 * @brief  Delete the millis Task manager
 * @param  无
 * @retval 无
 */
static Task_t *MillisTaskManager_findTask(TaskFunction_t func)
{
    Task_t *now = TaskManager->Head;
    Task_t *task = NULL;
    while (true)
    {
        if (now == NULL) 
            break;
        if (now->Function == func)
        {
            task = now;
            break;
        }
        now = now->Next;
    }
    return task;
}
/**
 * @brief  Register a task in the task manager
 * @param  func:Function
 * @param  timeMs:Task period
 * @param  state
 * @retval Task node
 */
Task_t *MillisTaskManager_register(TaskFunction_t func, uint32_t timeMs, bool state, void *param)
{
    Task_t *task = MillisTaskManager_findTask(func);
    if (task != NULL)
    {
        // Update info
        task->Time = timeMs;
        task->State = state;
        return task;
    }
    TASK_NEW(task);
    if (task == NULL)
    {
        return NULL;
    }
    task->param = param;
    task->Function = func; 
    task->Time = timeMs;   
    task->State = state;   
    task->TimePrev = 0;   
    task->TimeCost = 0;  
    task->TimeError = 0;  
    task->Next = NULL;    
    if (TaskManager->Head == NULL)
    {
        TaskManager->Head = task;
    }
    else
    {
        TaskManager->Tail->Next = task;
    }
    TaskManager->Tail = task;
    return task;
}
/**
 * @brief  Get the pre node
 * @param  task:当前任务节点地址
 * @retval 前一个任务节点地址
 */
Task_t *MillisTaskManager_getPrevNode(Task_t *task)
{
    Task_t *now = TaskManager->Head; 
    Task_t *prev = NULL;            
    Task_t *retval = NULL;         
    while (true)
    {
        if (now == NULL)
        {
            break;
        }
        if (now == task)
        {
            retval = prev;
            break;
        }
        prev = now;
        now = now->Next;
    }
    return retval;
}

/**
 * @brief  Unregister Task
 * @param  func:
 * @retval true if success
 */
bool MillisTaskManager_Unregister(TaskFunction_t func)
{
    Task_t *task = MillisTaskManager_findTask(func);
    if (task == NULL)
        return false;
    Task_t *prev = MillisTaskManager_getPrevNode(task); 
    Task_t *next = task->Next;                         
    if (prev == NULL && next != NULL)
    {
        TaskManager->Head = next;
    }
    else if (prev != NULL && next == NULL)
    {
        prev->Next = NULL;
    }
    else if (prev != NULL && next != NULL)
    {
        prev->Next = next;
    }
    TASK_DEL(task);

    return true;
}

/**
 * @brief  Control the task state
 * @param  func:
 * @param  state:
 * @retval true if success
 */
bool MillisTaskManager_setTaskState(TaskFunction_t func, bool state)
{
    Task_t *task = MillisTaskManager_findTask(func);
    if (task == NULL)
        return false;
    task->State = state;
    return true;
}

/**
 * @brief  Config the task period
 * @param  func:
 * @param  timeMs:
 * @retval true if success
 */
bool MillisTaskManager_setIntervalTime(TaskFunction_t func, uint32_t timeMs)
{
    Task_t *task = MillisTaskManager_findTask(func);
    if (task == NULL)
        return false;

    task->Time = timeMs;
    return true;
}

#if (MTM_USE_CPU_USAGE == 1)
#include "Arduino.h"                //需要使用micros()
static uint32_t UserFuncLoopUs = 0; // 累计时间
/**
 * @brief  获取CPU占用率
 * @param  无
 * @retval CPU占用率，0~100%
 */
float MillisTaskManager_GetCPU_Usage()
{
    static uint32_t MtmStartUs;
    float usage = (float)UserFuncLoopUs / (micros() - MtmStartUs) * 100.0f;

    if (usage > 100.0f)
        usage = 100.0f;

    MtmStartUs = micros();
    UserFuncLoopUs = 0;
    return usage;
}
#endif

/**
 * @brief  Time elaps
 * @param  nowTick
 * @param  prevTick
 * @retval Time
 */
uint32_t MillisTaskManager_getTickElaps(uint32_t nowTick, uint32_t prevTick)
{
    uint32_t actTime = nowTick;

    if (actTime >= prevTick)
    {
        prevTick = actTime - prevTick;
    }
    else
    {
        prevTick = /*UINT32_MAX*/ 0xFFFFFFFF - prevTick + 1;
        prevTick += actTime;
    }

    return prevTick;
}

/**
 * @brief  Get the task time cost(us)
 * @param  func:
 * @retval (us)
 */
uint32_t MillisTaskManager_getTimeCost(TaskFunction_t func)
{
    Task_t *task = MillisTaskManager_findTask(func);
    if (task == NULL)
        return 0;

    return task->TimeCost;
}

/**
 * @brief  Schedule
 * @param  tick:give the tick
 * @retval 无
 */
void MillisTaskManager_Running(uint32_t tick)
{
    Task_t *now = TaskManager->Head;
    while (true)
    {
        /*当前节点是否为空*/
        if (now == NULL)
        {
            /*遍历结束*/
            break;
        }

        if (now->Function != NULL && now->State)
        {
            uint32_t elapsTime = MillisTaskManager_getTickElaps(tick, now->TimePrev);
            if (elapsTime >= now->Time)
            {
                /*获取时间误差，误差越大实时性越差*/
                now->TimeError = elapsTime - now->Time;

                /*记录时间点*/
                now->TimePrev = tick;

#if (MTM_USE_CPU_USAGE == 1)
                /*记录开始时间*/
                uint32_t start = micros();

                /*执行任务*/
                now->Function();

                /*获取执行时间*/
                uint32_t timeCost = micros() - start;

                /*记录执行时间*/
                now->TimeCost = timeCost;

                /*总时间累加*/
                UserFuncLoopUs += timeCost;
#else
                now->Function(now->param);
#endif

                /*判断是否开启优先级*/
                if (TaskManager->PriorityEnable)
                {
                    /*遍历结束*/
                    break;
                }
            }
        }

        /*移动到下一个节点*/
        now = now->Next;
    }
}
