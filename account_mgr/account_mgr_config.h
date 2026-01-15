#ifndef __ACCOUNT_MANAGER_CONFIG_H
#define __ACCOUNT_MANAGER_CONFIG_H

#include <stdio.h>





#define ACCOUNT_LOG_ENABLE 1 






#if ACCOUNT_LOG_ENABLE
#define LOG_COLOR_RED     "\033[0;31m"
#define LOG_COLOR_YELLOW  "\033[0;33m"
#define LOG_COLOR_GREEN   "\033[0;32m"
#define LOG_COLOR_RESET   "\033[0m"

#define ACCOUNT_LOGI(tag, fmt, ...) \
    printf(LOG_COLOR_GREEN "[I][%s] " fmt LOG_COLOR_RESET "\n", tag, ##__VA_ARGS__)

#define ACCOUNT_LOGW(tag, fmt, ...) \
    printf(LOG_COLOR_YELLOW "[W][%s] " fmt LOG_COLOR_RESET "\n", tag, ##__VA_ARGS__)

#define ACCOUNT_LOGE(tag, fmt, ...) \
    printf(LOG_COLOR_RED "[E][%s] " fmt LOG_COLOR_RESET "\n", tag, ##__VA_ARGS__)
#endif
// LOCK 
#define _CREATE_LOCK() (1)
#define _DELECT_LOCK(lock) 
#define _LOCK(lock, time)
#define _UNLOCK(lock)

#include "HeapManager.h"

#define __CALLOC  heap_mgr_calloc
#define __FREE    heap_mgr_free
















#endif