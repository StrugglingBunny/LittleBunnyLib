

#include <stdio.h>
#include <string.h>
#include "../HeapManager.h"

static uint8_t heapPool[1024 * 10];
typedef struct _TestInfor
{
    const char *test;
    uint32_t Avalue;
    uint32_t Bvalue;

} TestInfor;
int main()
{
    HeapManager_Init(&heapPool[0], sizeof(heapPool));
    uint8_t *i = (uint8_t *)HeapManager_malloc(sizeof(uint8_t));
    if (i)
    {
        memset(i, 0, sizeof(uint8_t));
        *i = 1;
        printf("%d\r\n", *i);
        HeapManager_logHeapPool();
        HeapManager_free(i);
        i = NULL;
    }
    TestInfor *infor = (TestInfor *)HeapManager_malloc(sizeof(TestInfor));
    if (infor)
    {
        memset(infor, 0, sizeof(TestInfor));
        infor->test = "Hello ,this is a test demo ,show you how to use this heapManager to malloc and free";
        infor->Avalue = 1000;
        infor->Bvalue = 1000 * 1000;
        printf("test is :%s Avalue is :%d Bvalue is :%d \r\n", infor->test, infor->Avalue, infor->Bvalue);
        HeapManager_logHeapPool();
        HeapManager_free(infor);
        infor = NULL;
    }

    HeapManager_logHeapPool();
    return 0;
}
