#ifndef _HEAP_MANAGER_H
#define _HEAP_MANAGER_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>
#include <stdint.h>
#define HEAP_MANAGER_USE_LOG 1

#if (HEAP_MANAGER_USE_LOG == 1)
#include <stdio.h>
#define _HEAP_MANAGER_LOG(format, ...) printf("[HEAP MANAGER]" format "\r\n", ##__VA_ARGS__)
#define HEAP_MANAGER_INFO(format, ...) _HEAP_MANAGER_LOG("[Info] " format, ##__VA_ARGS__)
#define HEAP_MANAGER_WARN(format, ...) _HEAP_MANAGER_LOG("[Warn] " format, ##__VA_ARGS__)
#define HEAP_MANAGER_ERROR(format, ...) _HEAP_MANAGER_LOG("[Error] " format, ##__VA_ARGS__)
#else
#define HEAP_MANAGER_INFO(...)
#define HEAP_MANAGER_INFO(...)
#define HEAP_MANAGER_WARN(...)
#define HEAP_MANAGER_ERROR(...)
#endif
typedef struct _HeapBlockList
{
    union 
    {
        struct 
        {
            unsigned int isOccupied : 1;	 
            unsigned int size : 31; 
        };
        int info; 
    };
    struct _HeapBlockList *next; 
} HeapBlockList;
typedef struct _HeapManager
{
    void *heapTop;
    void *heapEnd;
    unsigned int heapTotalSize;
    HeapBlockList *head;
} HeapManager;
void HeapManager_Init(unsigned char *buffer, unsigned int size);
void *HeapManager_malloc(int size);
void HeapManager_free(void *ptr);
bool HeapManager_blockSizeAppend(uint8_t **addrToPtr,int32_t bytes);
void HeapManager_logHeapPool(void);
HeapBlockList *HeapManager_getHeapBlock(void *ptr);
#ifdef __cplusplus
}
#endif
#endif