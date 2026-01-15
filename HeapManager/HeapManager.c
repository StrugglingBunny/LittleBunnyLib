/*
 * \file   HeapManager.c
 * \brief  Manage heap
 *
 *
 * - Description: This file implments heap
 *
 * - Created on: Jan 13, 2026
 *
 * - Author: StrugglingBunny
 * - GitHub:StrugglingBunny
 */
#include "HeapManager.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
static void __enter_critical();
static void __exit_critical();
static void *__internal_malloc(uint32_t size);
static void __internal_free(void *ptr);

static bool __heap_mgr_checkHeapPool(void);
static bool __heap_mgr_checkHeapBlock(void *ptr, uint32_t *size);
static HeapBlockList *__heap_mgr_getFreeBlock(uint32_t size, bool *memCut);
static uint32_t __heap_mgr_getMaxFreeBlockSize(void);

static HeapManager __heapMgr;

/**
 * @brief  Internal malloc memery from Heap manager
 * @param  size
 * @retval Adress of the block
 */
static void *__internal_malloc(uint32_t size)
{
    uint32_t free_size = __heap_mgr_getMaxFreeBlockSize();
    if (size && free_size >= size)
    {
        HeapBlockList *ptr = __heapMgr.head;
        HeapBlockList *free_block = NULL;
        uint32_t n = size / sizeof(void *);
        uint32_t currentSize = n * sizeof(void *);
        if (size % sizeof(void *) != 0)
        {
            currentSize += sizeof(void *);
        }
        bool isNeedCut = false;
        HeapBlockList *node = __heap_mgr_getFreeBlock(currentSize, &isNeedCut);
        if (node == NULL)
        {
            HEAP_MANAGER_INFO("malloc size = %d faile !!!!!\n", size);
            heap_mgr_logHeapPool();
            return (void *)(NULL);
        }
        node->isOccupied = 1;
        uint8_t *p = (uint8_t *)node;
        if (isNeedCut)
        {
            p += sizeof(HeapBlockList) + currentSize;
            free_block = (HeapBlockList *)(p);
            free_block->size = node->size - currentSize - sizeof(HeapBlockList);
            free_block->isOccupied = 0;
            free_block->next = node->next;
            free_block->prev = node;
            if (free_block->next != NULL)
            {
                free_block->next->prev = free_block;
            }
            node->next = free_block;
            node->size = currentSize;
        }

        p = (uint8_t *)node;
        p += sizeof(HeapBlockList);
        if (__heap_mgr_checkHeapPool() == false)
        {
            node->isOccupied = 0;
            return NULL;
        }
        return (void *)(p);
    }

    return NULL;
}
/**
 * @brief  Internal free memery from Heap manager
 * @param  ptr
 * @retval void
 */
static void __internal_free(void *ptr)
{
    if (ptr == NULL)
        return;
    HeapBlockList *curr = (HeapBlockList *)((uint8_t *)ptr - sizeof(HeapBlockList));
    if (curr < (HeapBlockList *)__heapMgr.heapTop || curr >= (HeapBlockList *)__heapMgr.heapEnd)
    {
        return;
    }

    curr->isOccupied = 0; // set to empty

    //  (Merge Next)
    //
    HeapBlockList *next_node = curr->next;
    if (next_node != NULL && next_node->isOccupied == 0)
    {
        curr->size += sizeof(HeapBlockList) + next_node->size;
        curr->next = next_node->next;
        if (curr->next != NULL)
        {
            curr->next->prev = curr;
        }
    }
    //  (Merge Prev)
    HeapBlockList *prev_node = curr->prev;
    if (prev_node != NULL && prev_node->isOccupied == 0)
    {

        prev_node->size += sizeof(HeapBlockList) + curr->size;
        prev_node->next = curr->next;
        if (prev_node->next != NULL)
        {
            prev_node->next->prev = prev_node;
        }
    }
}

/**
 * @brief  Get max free block size
 * @param  NONE
 * @retval size
 */
static uint32_t __heap_mgr_getMaxFreeBlockSize(void)
{
    HeapBlockList *node = __heapMgr.head;
    uint32_t size = 0;
    while (node)
    {
        if (node->isOccupied == 0)
        {
            if (node->size > size)
            {
                size = node->size;
            }
        }
        node = node->next;
    }
    return size;
}
/**
 * @brief  Get min free block
 * @param  NONE
 * @retval size
 */
static HeapBlockList *__heap_mgr_getFreeBlock(uint32_t size, bool *memCut)
{
    HeapBlockList *current_node = __heapMgr.head;
    HeapBlockList *ret_node = NULL;
    uint32_t min_size = 0xffffffff;
    *memCut = false;
    while (current_node)
    {
        if (current_node->isOccupied == 0 && current_node->size >= size) // 块是否空闲
        {
            bool current_cut = false;
            if (current_node->size >= (size + sizeof(HeapBlockList) + (sizeof(void *))))
                current_cut = true;

            if (current_node->size < min_size)
            {
                min_size = current_node->size;
                *memCut = current_cut;
                ret_node = current_node;

                if (min_size == size)
                    break;
            }
        }
        current_node = current_node->next;
    }
    return ret_node;
}

/**
 * @brief Check Heap block Pool
 * @param  NONE
 * @retval size
 */
static bool __heap_mgr_checkHeapPool(void)
{
#if HEAP_DEBUG_CHECK == 1
    HeapBlockList *node = __heapMgr.head;
    bool status = true;
    uint32_t size = 0;
    uint32_t cnt = 0;

    while (node)
    {
        cnt++;
        size += node->size;
        node = node->next;
    }

    if (size < (__heapMgr.heapTotalSize - cnt * sizeof(HeapBlockList)))
    {
        HEAP_MANAGER_INFO("__heap_mgr_checkHeapPool err now only have %d block, size = %d!!!\n", cnt, size);
        heap_mgr_logHeapPool();
        status = false;
    }
    return status;
#else
    return true;
#endif
}

/**
 * @brief Check Heap block Pool
 * @param  NONE
 * @retval size
 */
static bool __heap_mgr_checkHeapBlock(void *ptr, uint32_t *size)
{
    if ((ptr < __heapMgr.heapTop) || (ptr > __heapMgr.heapEnd))
    {
        return false; // Adress no valid
    }
    HeapBlockList *node = __heapMgr.head;
    uint8_t *current_p = NULL;
    while (node)
    {
        uint32_t current_size = node->size;
        current_p = (uint8_t *)node + sizeof(HeapBlockList);
        if ((ptr == current_p) && (ptr < (current_p + current_size)))
        {
            if (node->isOccupied == 0)
            {
                HEAP_MANAGER_INFO("__heap_mgr_checkHeapBlock this address = %p is not active !!\n", ptr);
                return false;
            }
            else
            {
                if (size != NULL)
                    *size = node->size;
                return true;
            }
        }
        node = node->next;
    }
    return false;
}
static void *__internal_heap_mgr_realloc(void *ptr, uint32_t new_size)
{

    if (ptr == NULL)
    {
        return __internal_malloc(new_size);
    }
    if (new_size == 0)
    {
        __internal_free(ptr);
        return NULL;
    }
    uint32_t preSize = 0;
    if ((ptr < __heapMgr.heapTop) || (ptr >= __heapMgr.heapEnd))
    {
        return NULL;
    }
    HeapBlockList *node = (HeapBlockList *)((uint8_t *)ptr - sizeof(HeapBlockList));
    uint32_t old_size = node->size;
    if (old_size >= new_size)
    {
        return ptr;
    }
    void *new_ptr = __internal_malloc(new_size);
    if (new_ptr != NULL)
    {
        memcpy(new_ptr, ptr, old_size);
        __internal_free(ptr);
        return new_ptr;
    }
    return NULL;
}

static void __enter_critical()
{
    if (__heapMgr.enter_critical && __heapMgr.exit_critical)
    {
        __heapMgr.enter_critical();
    }
}
static void __exit_critical()
{
    if (__heapMgr.enter_critical && __heapMgr.exit_critical)
    {
        __heapMgr.exit_critical();
    }
}

/************************** Public function ****************************************** */

/**
 * @brief  Initilize the Heap Pool Manager
 * @param  buffer
 * @param  size
 * @retval size
 */
void heap_mgr_init(uint8_t *buffer, uint32_t size, void (*enter_critical)(void), void (*exit_critical)(void))
{
    // 1. Calculate aligned address
    uintptr_t addr = (uintptr_t)buffer;
    uintptr_t aligned_addr = (addr + sizeof(void *) - 1) & ~(sizeof(void *) - 1);
    // 2. Calculate the lost bytes
    uint32_t offset = aligned_addr - addr;
    __heapMgr.heapTop = (void *)aligned_addr;
    __heapMgr.heapTotalSize = size - offset;
    __heapMgr.heapEnd = (uint8_t *)__heapMgr.heapTop + __heapMgr.heapTotalSize;
    __heapMgr.head = (HeapBlockList *)__heapMgr.heapTop;
    __heapMgr.head->size = __heapMgr.heapTotalSize - sizeof(HeapBlockList);
    __heapMgr.head->isOccupied = 0;
    __heapMgr.head->next = NULL;
    __heapMgr.head->prev = NULL;
    __heapMgr.isEnable = true;
    __heapMgr.enter_critical = enter_critical;
    __heapMgr.exit_critical = exit_critical;
    HEAP_MANAGER_INFO("HeapManager Initilize successfully adress:%p ,size : %d KB\n", __heapMgr.heapTop, size / 1024);
}
/**
 * @brief  Check if the heap manager is already been initilized
 * @param  void
 * @retval true if heap manager is initilized
 */
bool heap_mgr_isInitilized()
{
    return __heapMgr.isEnable;
}
/**
 * @brief  Allocate memory and initialize
 * @param  size: Size
 * @retval Pointer to the allocated memory
 */
void *heap_mgr_malloc(uint32_t size)
{
    void *p = NULL;
    __enter_critical();
    p = __internal_malloc(size);
    __exit_critical();
    return p;
}
/**
 * @brief  Free the allocated memory
 * @retval void
 */
void heap_mgr_free(void *ptr)
{
    __enter_critical();
    __internal_free(ptr);
    __exit_critical();
}
/**
 * @brief  Reallocate memory
 * @param  addrToPtr: Allocated memory address
 * @param  size: new size of the memory
 * @retval Pointer to the allocated memory
 */
void *heap_mgr_realloc(void *ptr, uint32_t new_size)
{
    __enter_critical();
    void *_ptr = __internal_heap_mgr_realloc(ptr, new_size);
    __exit_critical();
    return _ptr;
}
/**
 * @brief  Allocate memory and initialize to zero (calloc)
 * @param  nmemb: Number of elements
 * @param  size: Size of each element
 * @retval Pointer to the allocated memory
 */
void *heap_mgr_calloc(uint32_t nmemb, uint32_t size)
{
    if (nmemb && size > (0xFFFFFFFF / nmemb))
    {
        return NULL;
    }
    uint32_t total_size = nmemb * size;
    void *ptr = heap_mgr_malloc(total_size);
    if (ptr != NULL)
    {
        memset(ptr, 0, total_size);
    }
    return ptr;
}
/**
 * @brief  Get Heap block Pool Infor
 * @param  NONE
 * @retval size
 */
void heap_mgr_logHeapPool(void)
{
    HeapBlockList *node = __heapMgr.head;
    while (node)
    {
        HEAP_MANAGER_INFO("address = %p, is used = %d, next = %p, size = %d\n", node, node->isOccupied, node->next, node->size);
        node = node->next;
    }
}
/**
 * @brief  Get the block
 * @param  ptr block address
 * @retval block infor
 */
HeapBlockList *heap_mgr_getHeapBlock(void *ptr)
{
    if ((ptr < __heapMgr.heapTop) || (ptr > __heapMgr.heapEnd))
    {
        return NULL; // Adress no valid
    }
    HeapBlockList *node = __heapMgr.head;
    uint8_t *current_p = NULL;
    while (node)
    {
        uint32_t current_size = node->size;
        current_p = (uint8_t *)node + sizeof(HeapBlockList);
        if ((ptr == current_p) && (ptr < (current_p + current_size)))
        {
            return node;
        }
        node = node->next;
    }
    return NULL;
}
