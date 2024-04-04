/*
 * \file   Account.c
 * \brief  Account module
 *
 * - Responsible Engineer:Jeffer Chen
 *
 * - Description: This file implments account
 *
 * - Created on: Feb 12, 2024
 * - Modify from Account C++ vertion by _FASTSHIFT
 * - Author: Jeffer Chen
 */
#include "HeapManager.h"
#include "PingPongBuffer.h"
#include "Account.h"
#include <string.h>
#define ACCOUNT_DISCARD_READ_DATA 1

static Account *AccountManager_searchAccount(AccountPoolList *node, const char *ID);
static AccountManager *g_accountManager = NULL;
/**
 * @brief  Initlize the account manager
 * @param  buffer
 * @param  size
 * @retval size
 */
void AccountManager_Init()
{
    g_accountManager = (AccountManager *)HeapManager_malloc(sizeof(AccountManager));
    memset(g_accountManager, 0, sizeof(AccountManager));
    if (g_accountManager)
    {
        g_accountManager->AccountNumber = 0;
        g_accountManager->Head = NULL;
        g_accountManager->Tail = NULL;
        DC_LOG_INFO("AccountManager Initilize successfully ");
    }
    else
    {
        DC_LOG_ERROR("AccountManager Initilize fail ");
    }
}
/**
 * @brief  Initlize the account manager
 * @param  buffer
 * @param  size
 * @retval size
 */
void AccountManager_DeInit()
{
    AccountPoolList *node = g_accountManager->Head;
    AccountPoolList *tempNode = NULL;
    while (node)
    {
        if (node->account)
        {
            AccountManager_DeleteAccount(node->account->ID);
        }
        tempNode = node->next;
        HeapManager_free(node);
        node = tempNode;
    }
    HeapManager_free(g_accountManager);
    g_accountManager = NULL;
}
/**
 * @brief  Create a account
 * @param  buffer
 * @param  size
 * @retval size
 */
bool AccountManager_CreateAccount(const char *id, uint32_t bufSize, void *userData)
{
    AccountPoolList *node = g_accountManager->Head;
    // Check if the account has not been created
    if (AccountManager_searchAccount(node, id) != NULL)
    {
        DC_LOG_ERROR("Account[%s] has already crated !!!!!!", id);
        return false;
    }
    Account *account = (Account *)HeapManager_malloc(sizeof(Account));
    memset(account, 0, sizeof(Account));
    bool status = false;
    if (account != NULL)
    {
        // Set param
        account->ID = id;
        account->BufferSize = bufSize;
        account->eventCb = NULL;
        account->publishers = NULL;
        account->subscribers = NULL;
        // Create a pingpong buffer if needed
        if (bufSize != 0)
        {
            uint8_t *buffer = (uint8_t *)HeapManager_malloc(bufSize * sizeof(uint8_t) * 2);
            memset(buffer, 0, bufSize * sizeof(uint8_t) * 2);
            if (buffer)
            {
                memset(buffer, 0, bufSize * sizeof(uint8_t) * 2);
                uint8_t *buf0 = buffer;
                uint8_t *buf1 = buffer + bufSize;
                PingPongBuffer_Init(&account->BufferManager, buf0, buf1);
                DC_LOG_INFO("Account[%s] cached %d x2 bytes", id, bufSize);
                account->BufferSize = bufSize;
            }
        }
        AccountPoolList *newList = (AccountPoolList *)HeapManager_malloc(sizeof(AccountPoolList));
        memset(newList, 0, sizeof(AccountPoolList));
        if (newList)
        {
            newList->account = account;
            newList->next = NULL;
            // If it is the first account
            if (g_accountManager->Head == NULL)
            {
                g_accountManager->Head = newList;
            }
            else
            {
                g_accountManager->Tail->next = newList;
            }
            g_accountManager->Tail = newList;
            g_accountManager->AccountNumber++;
            return true;
        }
    }
    else
    {
        return false;
    }
}
static bool AcctManager_accountPoolNodeDelete(AccountPoolList *node, const char *id)
{
    AccountPoolList *tempNode = NULL;
    AccountPoolList *preNode = NULL;
    AccountPoolList *nextNode = NULL;
    Account *account = AccountManager_searchAccount(node, id);
    // Free the publishers pool
    while (node)
    {
        {
            // Go to the publisher pool and Delete the this account
            tempNode = node->account->subscribers;
            while (tempNode)
            {
                if (tempNode->account == account)
                {
                    // Check if the node is the head node or the end node
                    if (tempNode->next)
                    {
                        nextNode = tempNode->next;
                        HeapManager_free(tempNode);
                        if (preNode)
                        {
                            preNode->next = nextNode;
                        }
                        else // If it is the head node
                        {
                            node->account->subscribers = nextNode;
                        }
                    }
                    else if (preNode) // If it is the tail node
                    {
                        HeapManager_free(tempNode);
                        preNode->next = NULL;
                    }
                    else // If it is the head node and also the last node
                    {
                        HeapManager_free(tempNode); // Free the node
                        node->account->subscribers = NULL;
                    }
                    break;
                }
                preNode = tempNode;
                tempNode = tempNode->next;
            }
        }
        // if still follow
        if (node->next)
        {
            tempNode = node->next;
            HeapManager_free(node);
            node = tempNode;
        }
        else
        {
            HeapManager_free(node);
            break;
        }
    }
}
/**
 * @brief  Delete account
 * @param  id
 * @retval true if success
 */
bool AccountManager_DeleteAccount(const char *id)
{
    AccountPoolList *node = g_accountManager->Head;
    AccountPoolList *tempNode = NULL;
    AccountPoolList *preNode = NULL;
    AccountPoolList *nextNode = NULL;
    Account *account = AccountManager_searchAccount(node, id);
    if (account)
    {
        if (account->BufferSize)
        {
            HeapManager_free(account->BufferManager.buffer[0]);
        }
        node = account->publishers;
        // Free the publishers pool 
        while (node)
        {
            {
                // Go to the publisher pool and Delete the this account
                tempNode = node->account->subscribers;
                while (tempNode)
                {
                    if (tempNode->account == account)
                    {
                        // Check if the node is the head node or the end node
                        if (tempNode->next)
                        {
                            nextNode = tempNode->next;
                            HeapManager_free(tempNode);

                            if (preNode)
                            {
                                preNode->next = nextNode;
                            }
                            else // If it is the head node
                            {
                                node->account->subscribers = nextNode;
                            }
                        }
                        else if (preNode) // If it is the tail node
                        {
                            HeapManager_free(tempNode);
                            preNode->next = NULL;
                        }
                        else // If it is the head node and also the last node
                        {
                            HeapManager_free(tempNode); // Free the node
                            node->account->subscribers = NULL;
                        }
                        break;
                    }
                    preNode = tempNode;
                    tempNode = tempNode->next;
                }
            }
            // if still follow
            if (node->next)
            {
                tempNode = node->next;
                HeapManager_free(node);
                node = tempNode;
            }
            else
            {
                HeapManager_free(node);
                break;
            }
        }

        node = account->subscribers;
        preNode = NULL;
        tempNode = NULL;
        // Free the subscribers pool
        while (node)
        {
            {
                // Delete the sub account
                tempNode = node->account->publishers;
                while (tempNode)
                {
                    if (tempNode->account == account)
                    {
                        // Check if the node is not the end node
                        if (tempNode->next != NULL)
                        {
                            nextNode = tempNode->next;
                            HeapManager_free(tempNode);
                            if (preNode)
                            {
                                preNode->next = nextNode;
                            }
                            else
                            {
                                node->account->publishers = nextNode;
                            }
                        }
                        else if (preNode) // If it is the tail node
                        {
                            HeapManager_free(tempNode);
                            preNode->next = NULL;
                        }
                        else
                        {
                            HeapManager_free(tempNode);
                            node->account->publishers = NULL;
                        }
                        break;
                    }
                    preNode = tempNode;
                    tempNode = tempNode->next;
                }
            }
            if (node->next)
            {
                tempNode = node->next;
                HeapManager_free(node);
                node = tempNode;
            }
            else
            {
                HeapManager_free(node);
                break;
            }
        }
        node = g_accountManager->Head;
        preNode = NULL;
        nextNode = NULL;
        {
            while (node)
            {
                if (node->account == account)
                {
                    if (node->next)
                    {
                        nextNode = node->next;
                        HeapManager_free(node);
                        if (preNode)
                        {
                            preNode->next = nextNode;
                        }
                        else
                        {
                            g_accountManager->Head = nextNode;
                        }
                    }
                    // The node is the head node or the tail node
                    else if (preNode != NULL)
                    {
                        preNode->next = NULL;
                        HeapManager_free(node);
                    }
                    else
                    {
                        HeapManager_free(g_accountManager->Head);
                        g_accountManager->Head = NULL;
                    }
                    break;
                }
                preNode = node;
                node = node->next;
            }
        }
        HeapManager_free(account);
        g_accountManager->AccountNumber--;
    }
    else
    {
        DC_LOG_ERROR("Account[%s] is not crated !!!!!!", id);
        return false;
    }
}
static Account *AccountManager_searchAccount(AccountPoolList *node, const char *ID)
{
    Account *account = NULL;
    while (node != NULL) // check if it is the last node
    {
        if (strcmp(node->account->ID, ID) == 0)
        {
            account = node->account;
        }
        node = node->next;
    }
    return account;
}
/**
 * @brief  Log account from the pool
 * @param  id :Account id ,if NULL,log all account from the account manager pool
 * @retval void
 */
void AccountManager_LogAccount(const char *id)
{
    AccountPoolList *node;
    uint32_t num = 0;
    if (id)
    {
        Account *account = AccountManager_searchAccount(g_accountManager->Head, id);
        if (account)
        {
            DC_LOG_INFO("Account [%s]Log all followers", id);
            node = account->subscribers;
            while (node)
            {
                if (node->account)
                {
                    DC_LOG_INFO("Account[%s] ", node->account->ID);
                    num++;
                }
                node = node->next;
            }
            DC_LOG_INFO("Total followers num [%d]", num);
            DC_LOG_INFO("Log all subscrib");
            node = account->publishers;
            num = 0;
            while (node)
            {
                if (node->account)
                {
                    DC_LOG_INFO("Account[%s] ", node->account->ID);
                    num++;
                }
                node = node->next;
            }
            DC_LOG_INFO("Total subscrib num [%d] \r\n", num);
        }
    }
    else
    {
        node = g_accountManager->Head;
        DC_LOG_INFO("Log all Accounts");
        while (node != NULL)
        {
            if (node->account)
            {
                DC_LOG_INFO("[%s] ", node->account->ID);
                num++;
            }
            node = node->next;
        }
        DC_LOG_INFO("Total account num [%d] \r\n", num);
    }
}
/**
 * @brief  Account register a callback function
 * @param  id : Account ID
 * @param  eventCb :Event callback
 * @retval true if success
 */
bool Account_registerCb(const char *id, EventCallback_t eventCb)
{
    AccountPoolList *node = g_accountManager->Head;
    Account *account = AccountManager_searchAccount(node, id);
    if (account)
    {
        account->eventCb = eventCb;
        return true;
    }
    return false;
}

/**
 * @brief  Subscribe account
 * @param  accountID :
 * @param  subID :ID that needs to be subscribe
 * @retval true if success
 */
bool Account_subscribe(const char *accountID, const char *subID)
{
    if (strcmp(accountID, subID) == 0)
    {
        DC_LOG_ERROR("Can not subscribe itseft");
        return false;
    }
    Account *Subscriber = AccountManager_searchAccount(g_accountManager->Head, accountID);
    Account *account = AccountManager_searchAccount(g_accountManager->Head, subID);
    AccountPoolList *node = NULL;

    if (Subscriber && account) // Check if the account is created or not
    {
        node = Subscriber->publishers;
        // Check if muti subscribe
        if (AccountManager_searchAccount(node, subID))
        {
            DC_LOG_ERROR("Muti subscribe ");
            return false;
        }
        // Create a new node
        AccountPoolList *newNode = (AccountPoolList *)HeapManager_malloc(sizeof(AccountPoolList));
        memset(newNode, 0, sizeof(AccountPoolList));

        if (newNode)
        {
            newNode->account = account;
            newNode->next = NULL;
            // Register in the publishers pool
            // Check if the publishers pool has aready initilize
            if (Subscriber->publishers == NULL)
            {
                Subscriber->publishers = newNode;
            }
            else
            {
                // Add the new node at the end of the node list
                node = Subscriber->publishers;
                while (node->next)
                {
                    node = node->next;
                }
                node->next = newNode;
            }
        }
        // Create a new node
        newNode = (AccountPoolList *)HeapManager_malloc(sizeof(AccountPoolList));
        memset(newNode, 0, sizeof(AccountPoolList));
        if (newNode)
        {
            newNode->account = Subscriber;
            newNode->next = NULL;
            // Register in the Subscriber pool
            // Check if the Subscriber pool has aready initilize
            if (account->subscribers == NULL)
            {
                account->subscribers = newNode;
            }
            else
            {
                // Add the new node at the end of the node list
                node = account->subscribers;
                while (node->next)
                {
                    node = node->next;
                }
                node->next = newNode;
            }
        }
        return true;
    }
    DC_LOG_ERROR("Can not subscribe ");
    return false;
}
/**
 * @brief  unsubscribe account
 * @param  accountID :
 * @param  subID :ID that needs to be subscribe
 * @retval true if success
 */
bool Account_unsubscribe(const char *accountID, const char *subID)
{
    if (strcmp(accountID, subID) == 0)
    {
        DC_LOG_ERROR("Can not unsubscribe itseft");
        return false;
    }
    Account *Subscriber = AccountManager_searchAccount(g_accountManager->Head, accountID);
    Account *account = AccountManager_searchAccount(g_accountManager->Head, subID);
    AccountPoolList *node = NULL;
    AccountPoolList *preNode = NULL;
    AccountPoolList *nextNode = NULL;
    if (Subscriber && account)
    {
        // Account Go to the publisher pool and delete this account
        node = Subscriber->publishers;
        if (AccountManager_searchAccount(node, subID) == NULL)
        {
            DC_LOG_ERROR("Not subs account[%s]yet", subID);
            return false;
        }
        while (node)
        {
            if (strcmp(node->account->ID, subID))
            {
                if (node->next)
                {
                    nextNode = node->next;
                    HeapManager_free(node);
                    if (preNode)
                    {
                        preNode->next = nextNode;
                    }
                    else // if it is the head node
                    {
                        Subscriber->publishers->next = node->next;
                    }
                }
                else if (preNode) // tail node
                {
                    preNode->next = NULL;
                    HeapManager_free(node);
                }
                else // The only node
                {
                    HeapManager_free(node); // Free the node
                    Subscriber->publishers = NULL;
                }
                break;
            }
            preNode = node;
            node = node->next;
        }
        // Publisher go the the sub pool and delete this account
        node = account->subscribers;

        while (node)
        {
            if (strcmp(node->account->ID, accountID))
            {
                if (node->next)
                {
                    nextNode = node->next;
                    HeapManager_free(node);
                    if (preNode)
                    {
                        preNode->next = nextNode;
                    }
                    else // if it is the head node
                    {
                        account->subscribers->next = node->next;
                    }
                }
                else if (preNode) // tail node
                {
                    preNode->next = NULL;
                    HeapManager_free(node);
                }
                else // The only node
                {
                    HeapManager_free(node); // Free the node
                    account->subscribers = NULL;
                }
                break;
            }
            preNode = node;
            node = node->next;
        }
        return true;
    }
    DC_LOG_ERROR("Can not subscribe ");
    return false;
}

/**
 * @brief  unsubscribe account
 * @param  accountID :
 * @retval true if success
 */
bool Account_commit(const char *id, const void *data_p, uint32_t size)
{
    AccountPoolList *node = g_accountManager->Head;
    Account *account = AccountManager_searchAccount(node, id);
    if (account)
    {
        if (!size || size != account->BufferSize)
        {
            DC_LOG_ERROR("pub[%s] has not cache", id);
            return false;
        }
        void *wBuf;
        PingPongBuffer_GetWriteBuf(&account->BufferManager, &wBuf);
        memcpy(wBuf, data_p, size);
        PingPongBuffer_SetWriteDone(&account->BufferManager);
        DC_LOG_INFO("pub[%s] commit data(0x%p)[%d] >> data(0x%p)[%d] done",
                    id, data_p, size, wBuf, size);
        return true;
    }
}
/**
 * @brief  Publish data to subscribers
 * @param  None
 * @retval error code
 */
int Account_publish(const char *id)
{
    int retval = RES_UNKNOW;
    AccountPoolList *node = g_accountManager->Head;
    Account *account = AccountManager_searchAccount(node, id);
    if (account == NULL)
        return RES_UNKNOW;
    if (account->BufferSize == 0)
    {
        DC_LOG_ERROR("pub[%s] has not cache", id);
        return RES_NO_CACHE;
    }
    void *rBuf;
    if (!PingPongBuffer_GetReadBuf(&account->BufferManager, &rBuf))
    {
        DC_LOG_WARN("pub[%s] data was not commit", id);
        return RES_NO_COMMITED;
    }
    EventParam_t param;
    param.event = EVENT_PUB_PUBLISH;
    param.tran = id;
    param.recv = NULL;
    param.data_p = rBuf;
    param.size = account->BufferSize;
    /* Publish messages to subscribers */
    node = account->subscribers;
    while (node)
    {
        if (node->account)
        {
            EventCallback_t callback = node->account->eventCb;
            DC_LOG_INFO("pub[%s] publish >> data(0x%p)[%d] >> sub[%s]...",
                        id, param.data_p, param.size, node->account->ID);
            if (callback != NULL)
            {
                param.recv = node->account->ID;
                int ret = callback(node->account, &param);

                DC_LOG_INFO("publish done: %d", ret);
                retval = ret;
            }
            else
            {
                DC_LOG_INFO("sub[%s] not register callback", node->account->ID);
            }
        }
        node = node->next;
    }
#if ACCOUNT_DISCARD_READ_DATA
    PingPongBuffer_SetReadDone(&account->BufferManager);
#endif
    return retval;
}
/**
 * @brief  Pull data from the publisher
 * @param  pubID:  Publisher ID
 * @param  data_p: Pointer to data
 * @param  size:   The size of the data
 * @retval error code
 */
int Account_pull(const char *sub, const char *pub, void *data_p, uint32_t size)
{
    AccountPoolList *node = g_accountManager->Head;
    Account *account = AccountManager_searchAccount(node, sub);
    if (account)
    {
        // Check if sub already sub the publisher
        node = account->publishers;
        Account *publiser = AccountManager_searchAccount(node, pub);
        if (publiser == NULL)
        {
            DC_LOG_ERROR("sub[%s] was not subscribe pub[%s]", sub, pub);
            return RES_NOT_FOUND;
        }
        int retval = RES_UNKNOW;
        DC_LOG_INFO("sub[%s] pull << data(0x%p)[%d] << pub[%s] ...",
                    sub, data_p, size, pub);
        EventCallback_t callback = publiser->eventCb;
        if (callback)
        {
            EventParam_t param;
            param.event = EVENT_SUB_PULL;
            param.tran = sub;
            param.recv = pub;
            param.data_p = data_p;
            param.size = size;
            int ret = callback(publiser, &param);
            DC_LOG_INFO("pull done: %d", ret);
            retval = ret;
        }
        else
        {
            DC_LOG_INFO("pub[%s] not registed pull callback, read commit cache...", pub);
            if (publiser->BufferSize == size)
            {
                void *rBuf;
                if (PingPongBuffer_GetReadBuf(&publiser->BufferManager, &rBuf))
                {
                    memcpy(data_p, rBuf, size);
#if ACCOUNT_DISCARD_READ_DATA
                    PingPongBuffer_SetReadDone(&publiser->BufferManager);
#endif
                    DC_LOG_INFO("read done");
                    retval = 0;
                }
                else
                {
                    DC_LOG_WARN("pub[%s] data was not commit!", pub);
                }
            }
            else
            {
                DC_LOG_ERROR(
                    "Data size pub[%s]:%d != sub[%s]:%d",
                    pub,
                    publiser->BufferSize,
                    sub,
                    size);
            }
        }
        return retval;
    }
    DC_LOG_WARN("Account[%s]is not created!", sub);

    return RES_UNKNOW;
}
/**
 * @brief  Send a notification to the publisher
 * @param  pubID: Publisher ID
 * @param  data_p: Pointer to data
 * @param  size:   The size of the data
 * @retval error code
 */
int Account_notify(const char *subID, const char *pubID, const void *data_p, uint32_t size)
{
    AccountPoolList *node = g_accountManager->Head;
    Account *sub = AccountManager_searchAccount(node, subID);
    if (sub)
    {
        node = sub->publishers;
        Account *pub = AccountManager_searchAccount(node, pubID);
        if (pub == NULL)
        {
            DC_LOG_ERROR("sub[%s] was not subscribe pub[%s]", subID, pubID);
            return RES_NOT_FOUND;
        }
        int retval = RES_UNKNOW;
        DC_LOG_INFO("sub[%s] notify >> data(0x%p)[%d] >> pub[%s] ...",
                    subID, data_p, size, pubID);
        EventCallback_t callback = pub->eventCb;
        if (callback != NULL)
        {
            EventParam_t param;
            param.event = EVENT_NOTIFY;
            param.tran = subID;
            param.recv = pubID;
            param.data_p = (void *)data_p;
            param.size = size;
            int ret = callback(pub, &param);
            DC_LOG_INFO("send done: %d", ret);
            retval = ret;
        }
        else
        {
            DC_LOG_WARN("pub[%s] not register callback", pub->ID);
            retval = RES_NO_CALLBACK;
        }

        return retval;
    }
    DC_LOG_WARN("Account[%s]is not created!", sub);
    return RES_UNKNOW;
}
