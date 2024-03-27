/*
 * \file   HeapManager.c
 * \brief  Manage heap
 *
 * - Responsible Engineer:Jeffer Chen
 *
 * - Description: This file implments heap
 *
 * - Created on: Feb 12, 2024
 * 
 * - Author:  CJourneys  & Jeffer Chen
 * - GitHub:StrugglingBunny
 */
#include "HeapManager.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
static bool HeapManager_checkHeapPool(void);
static bool HeapManager_checkHeapBlock(void *ptr, int *size);
static HeapBlockList *HeapManager_getFreeBlock(uint32_t size, bool *memCut);
static HeapBlockList *HeapManager_getFreeBlock(uint32_t size, bool *memCut);
static uint32_t HeapManager_getMaxFreeBlockSize(void);

static HeapManager g_HeapManager;
/**
 * @brief  Initilize the Heap Pool Manager
 * @param  buffer
 * @param  size
 * @retval size
 */
void HeapManager_Init(uint8_t *buffer, uint32_t size)
{
    g_HeapManager.heapEnd = &buffer[size];
    g_HeapManager.heapTop = buffer;
    g_HeapManager.heapTotalSize = size;
    g_HeapManager.head = (HeapBlockList *)g_HeapManager.heapTop;
    g_HeapManager.head->size = g_HeapManager.heapTotalSize - sizeof(HeapBlockList);
    g_HeapManager.head->isOccupied = 0;
    g_HeapManager.head->next = NULL;
    HEAP_MANAGER_INFO("HeapManager Initilize successfully adress:%p ,size : %d KB\n", g_HeapManager.heapTop, size / 1024);
}
/**
 * @brief  Malloc memery from Heap manager
 * @param  size
 * @retval Adress of the block
 */
void *HeapManager_malloc(int size)
{
    int free_size = HeapManager_getMaxFreeBlockSize();
    if (size && free_size >= size)
    {
        HeapBlockList *ptr = g_HeapManager.head;
        HeapBlockList *free_block = NULL;
        int n = size / sizeof(void *);
        int currentSize = n * sizeof(void *);
        if (size % sizeof(void *) != 0)
        {
            currentSize += sizeof(void *);
        }
        bool isNeedCut = false;
        HeapBlockList *node = HeapManager_getFreeBlock(currentSize, &isNeedCut);
        if (node == NULL)
        {
            HEAP_MANAGER_INFO("malloc size = %d faile !!!!!\n", size);
            HeapManager_logHeapPool();
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
            node->next = free_block;
            node->size = currentSize;
        }

        p = (uint8_t *)node;
        p += sizeof(HeapBlockList); // 偏移8个字节为真正使用的malloc地址
        if (HeapManager_checkHeapPool() == false)
        {
            return NULL;
        }
       
        return (void *)(p);
    }

    return NULL;
}
/**
 * @brief  Free memery from Heap manager
 * @param  ptr
 * @retval void
 */
void HeapManager_free(void *ptr)
{
    if (ptr != NULL)
    {
        if ((ptr > g_HeapManager.heapTop) && (ptr < g_HeapManager.heapEnd))
        {

            /* 计算地址对应的原始节点 */
            HeapBlockList *source_node = (HeapBlockList *)((uint8_t *)ptr - sizeof(HeapBlockList));
            //memset(ptr, 0, source_node->size);
            /* 找到node的前一个节点和下一个节点 */
            HeapBlockList *previous_node = g_HeapManager.head;
            while (previous_node && previous_node->next != source_node)
            {
                previous_node = previous_node->next;
            }
            HeapBlockList *next_node = source_node->next;

            source_node->isOccupied = 0;

            HeapBlockList *connect_node;
            if (previous_node && (previous_node->isOccupied == 0)) // 前一个节点是否空闲
            {
                connect_node = source_node->next;
                int size = source_node->size + sizeof(HeapBlockList);

                if (next_node && (next_node->isOccupied == 0)) // 下一个节点是否空闲
                {
                    connect_node = next_node->next;
                    size += next_node->size + sizeof(HeapBlockList);
                }
                previous_node->next = connect_node;
                previous_node->size += size;
            }
            else
            {
                connect_node = source_node->next;
                int size = source_node->size;

                if (next_node && (next_node->isOccupied == 0))
                {
                    connect_node = next_node->next;
                    size += next_node->size + sizeof(HeapBlockList);
                }
                source_node->next = connect_node;
                source_node->size = size;
            }
        }
        else
        {
            HEAP_MANAGER_INFO("HeapManager_free not allowd this address = %p(%p --- %p)\n", ptr, g_HeapManager.heapTop, g_HeapManager.heapEnd);
        }
    }
}
bool HeapManager_blockSizeAppend(uint8_t **addrToPtr, int32_t bytes)
{
    bool status = false;
    int preSize = 0;
    if (HeapManager_checkHeapBlock(*addrToPtr, &preSize) == true)
    {
        uint8_t *tempPtr;
        if ((preSize + bytes) > 0)
        {
            tempPtr = HeapManager_malloc(preSize + bytes);
            if (tempPtr != NULL)
            {
                if (bytes < 0)
                {
                    preSize += bytes;
                }
                for (uint32_t i = 0; i < preSize; i++)
                {
                    tempPtr[i] = (*addrToPtr)[i];
                }
                HeapManager_free(*addrToPtr);
                if (HeapManager_checkHeapBlock(tempPtr, NULL) == true)
                {
                    *addrToPtr = tempPtr;
                    status = true;
                }
            }
        }
        else
        {
            HeapManager_free(*addrToPtr);
            status = true;
        }
    }
    return (status);
}
/**
 * @brief  Get max free block size
 * @param  NONE
 * @retval size
 */
static uint32_t HeapManager_getMaxFreeBlockSize(void)
{
    HeapBlockList *node = g_HeapManager.head;
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
static HeapBlockList *HeapManager_getFreeBlock(uint32_t size, bool *memCut)
{
    HeapBlockList *current_node = g_HeapManager.head;
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
 * @brief  Get Heap block Pool Infor
 * @param  NONE
 * @retval size
 */
void HeapManager_logHeapPool(void)
{
    HeapBlockList *node = g_HeapManager.head;
    while (node)
    {
        HEAP_MANAGER_INFO("address = %p, is used = %d, next = %p, size = %d\n", node, node->isOccupied, node->next, node->size);
        node = node->next;
    }
}
/**
 * @brief Check Heap block Pool
 * @param  NONE
 * @retval size
 */
static bool HeapManager_checkHeapPool(void)
{
    HeapBlockList *node = g_HeapManager.head;
    bool status = true;
    int size = 0;
    int cnt = 0;

    while (node)
    {
        cnt++;
        size += node->size;
        node = node->next;
    }

    if (size < (g_HeapManager.heapTotalSize - cnt * sizeof(HeapBlockList)))
    {
        HEAP_MANAGER_INFO("HeapManager_checkHeapPool err now only have %d block, size = %d!!!\n", cnt, size);
        HeapManager_logHeapPool();
        status = false;
    }
    return status;
}

/**
 * @brief Check Heap block Pool
 * @param  NONE
 * @retval size
 */
static bool HeapManager_checkHeapBlock(void *ptr, int *size)
{
    if ((ptr < g_HeapManager.heapTop) || (ptr > g_HeapManager.heapEnd))
    {
        return false; // Adress no valid
    }
    HeapBlockList *node = g_HeapManager.head;
    uint8_t *current_p = NULL;
    while (node)
    {
        int current_size = node->size;
        current_p = (uint8_t *)node + sizeof(HeapBlockList);
        if ((ptr == current_p) && (ptr < (current_p + current_size)))
        {
            if (node->isOccupied == 0)
            {
                HEAP_MANAGER_INFO("HeapManager_checkHeapBlock this address = %p is not active !!\n", ptr);
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
HeapBlockList *HeapManager_getHeapBlock(void *ptr)
{
    if ((ptr < g_HeapManager.heapTop) || (ptr > g_HeapManager.heapEnd))
    {
        return NULL; // Adress no valid
    }
    HeapBlockList *node = g_HeapManager.head;
    uint8_t *current_p = NULL;
    while (node)
    {
        int current_size = node->size;
        current_p = (uint8_t *)node + sizeof(HeapBlockList);
        if ((ptr == current_p) && (ptr < (current_p + current_size)))
        {
            return node;
        }
        node = node->next;
    }
    return NULL;
}
