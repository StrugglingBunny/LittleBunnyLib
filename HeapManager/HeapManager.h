#ifndef _HEAP_MANAGER_H
#define _HEAP_MANAGER_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdbool.h>
#include <stdint.h>

#define REDIRECT_NEW_DELETE_FUNC 1
#define HEAP_DEBUG_CHECK 0

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
        struct _HeapBlockList *prev;
        struct _HeapBlockList *next;
        union
        {
            struct
            {
                uint32_t isOccupied : 1; // 0 : free 1 :used
                uint32_t size : 31;      // size of the block
            };
            uint32_t info;
        };

    } HeapBlockList;
    typedef struct _HeapManager
    {
        void *heapTop;
        void *heapEnd;
        uint32_t heapTotalSize;
        HeapBlockList *head;
        bool isEnable;
        void (*enter_critical)(void);
        void (*exit_critical)(void);
    } HeapManager;
    /**
     * @brief  Initilize the heap
     * @param  buffer: Heap buffer
     * @param  size: Size of heap
     * @param  enter_critical enter_critical function
     * @param  exit_critical exit_critical function
     */
    void heap_mgr_init(uint8_t *buffer, uint32_t size, void (*enter_critical)(void), void (*exit_critical)(void));
    /**
     * @brief  Allocate memory and initialize
     * @param  size: Size
     * @retval Pointer to the allocated memory
     */
    void *heap_mgr_malloc(uint32_t size);
    /**
     * @brief  Allocate memory and initialize to zero (calloc)
     * @param  nmemb: Number of elements
     * @param  size: Size of each element
     * @retval Pointer to the allocated memory
     */
    void *heap_mgr_calloc(uint32_t nmemb, uint32_t size);
    /**
     * @brief  Free the allocated memory
     * @retval void
     */
    void heap_mgr_free(void *ptr);
    /**
     * @brief  Reallocate memory
     * @param  addrToPtr: Allocated memory address
     * @param  size: new size of the memory
     * @retval Pointer to the allocated memory
     */
    void *heap_mgr_realloc(void *addrToPtr, uint32_t size);
    /**
     * @brief  Printf the status of the heap
     * @retval void
     */
    void heap_mgr_logHeapPool(void);
    /**
     * @brief  Get the block
     * @param  ptr block address
     * @retval block infor
     */
    HeapBlockList *heap_mgr_getHeapBlock(void *ptr);
    /**
     * @brief  Check if the heap manager is already been initilized
     * @param  void
     * @retval true if heap manager is initilized
     */
    bool heap_mgr_isInitilized(void);

#ifdef __cplusplus
}
#endif
#endif