#ifndef __ACCOUNT_H
#define __ACCOUNT_H

#ifdef __cplusplus
#include <stdint.h>
extern "C"
{
#endif
#include "PingPongBuffer.h"
#define DATA_CENTER_USE_LOG 1
#if (DATA_CENTER_USE_LOG == 1)
#include <stdio.h>
#define _DC_LOG(format, ...) printf("[DC]" format "\r\n", ##__VA_ARGS__)
#define DC_LOG_INFO(format, ...) _DC_LOG("[Info] " format, ##__VA_ARGS__)
#define DC_LOG_WARN(format, ...) _DC_LOG("[Warn] " format, ##__VA_ARGS__)
#define DC_LOG_ERROR(format, ...) _DC_LOG("[Error] " format, ##__VA_ARGS__)
#else
#define DC_LOG_INFO(...)
#define DC_LOG_WARN(...)
#define DC_LOG_ERROR(...)
#endif
    typedef struct _Account Account;
    /* Event type enumeration */
    typedef enum
    {
        EVENT_NONE,
        EVENT_PUB_PUBLISH, // Publisher posted information
        EVENT_SUB_PULL,    // Subscriber data pull request
        EVENT_NOTIFY,      // Subscribers send notifications to publishers
        EVENT_TIMER,       // Timed event
        _EVENT_LAST
    } EventCode_t;
    /* Error type enumeration */
    typedef enum
    {
        RES_OK = 0,
        RES_UNKNOW = -1,
        RES_SIZE_MISMATCH = -2,
        RES_UNSUPPORTED_REQUEST = -3,
        RES_NO_CALLBACK = -4,
        RES_NO_CACHE = -5,
        RES_NO_COMMITED = -6,
        RES_NOT_FOUND = -7,
        RES_PARAM_ERROR = -8
    } ResCode_t;
    /* Event parameter structure */
    typedef struct
    {
        EventCode_t event; // Event type
        void *data_p;      // Pointer to data
        const char *tran;  // Pointer to sender
        const char *recv;  // Pointer to receiver
        uint32_t size;     // The length of the data
    } EventParam_t;
    /* Event callback function pointer */
    typedef int (*EventCallback_t)(Account *account, EventParam_t *param);
    typedef struct _AccountPoolList
    {
        Account *account;
        struct _AccountPoolList *next;
    } AccountPoolList;
    typedef struct _Account
    {
        const char *ID; /* Unique account ID */
        void *UserData; /*  account ID */
        uint32_t BufferSize;
        PingPongBuffer_t BufferManager;
        EventCallback_t eventCb;
        AccountPoolList *publishers;  /* Followed publishers */
        AccountPoolList *subscribers; /* Followed subscribers */
    } Account;
    typedef struct _AccountManager
    {
        AccountPoolList *Head;
        AccountPoolList *Tail;
        uint32_t AccountNumber;
    } AccountManager;

    /**
     * @brief  Initlize the account manager
     * @param  buffer
     * @param  size
     * @retval size
     */
    void AccountManager_Init();
    /**
     * @brief  De init the account manager
     * @retval void
     */
    void AccountManager_DeInit();
    /**
     * @brief  Create a account
     * @param  buffer
     * @param  size
     * @retval size
     */
    bool AccountManager_CreateAccount(const char *id, uint32_t bufSize, void *userData);
    /**
     * @brief  Delete account
     * @param  id
     * @retval true if success
     */
    bool AccountManager_DeleteAccount(const char *id);
    /**
     * @brief  Log account from the pool
     * @param  id :Account id ,if NULL,log all account from the account manager pool
     * @retval void
     */
    void AccountManager_LogAccount(const char *id);
    /**
     * @brief  Account register a callback function
     * @param  id : Account ID
     * @param  eventCb :Event callback
     * @retval true if success
     */
    bool Account_registerCb(const char *id, EventCallback_t eventCb);
    /**
     * @brief  Subscribe account
     * @param  accountID :
     * @param  subID :ID that needs to be subscribe
     * @retval true if success
     */
    bool Account_subscribe(const char *accountID, const char *subID);
    /**
     * @brief  unsubscribe account
     * @param  accountID :
     * @param  subID :ID that needs to be subscribe
     * @retval true if success
     */
    bool Account_unsubscribe(const char *accountID, const char *subID);
    /**
     * @brief  unsubscribe account
     * @param  accountID :
     * @retval true if success
     */
    bool Account_commit(const char *id, const void *data_p, uint32_t size);
    /**
     * @brief  Publish data to subscribers
     * @param  None
     * @retval error code
     */
    int Account_publish(const char *id);
    /**
     * @brief  Pull data from the publisher
     * @param  pubID:  Publisher ID
     * @param  data_p: Pointer to data
     * @param  size:   The size of the data
     * @retval error code
     */
    int Account_pull(const char *sub, const char *pub, void *data_p, uint32_t size);
    /**
     * @brief  Send a notification to the publisher
     * @param  pubID: Publisher ID
     * @param  data_p: Pointer to data
     * @param  size:   The size of the data
     * @retval error code
     */
    int Account_notify(const char *subID, const char *pubID, const void *data_p, uint32_t size);
#ifdef __cplusplus
}
#endif
#endif