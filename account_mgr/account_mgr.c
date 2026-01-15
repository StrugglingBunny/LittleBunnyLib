#include "account_mgr.h"
#include "account_mgr_config.h"
#include <string.h>
#include <stdlib.h>
#include "HeapManager.h"





static const char *TAG = "account_mgr";


typedef struct subscriber_node {
    char *name;                     // subscriber account name
    sub_cb_t cb;              // callback
    void *user_ctx;                 // user context
    struct subscriber_node *next;
} subscriber_node_t;

typedef struct account_node {
    char *account_name;             // account name
    void *account_usr_arg;          // user argument for notify callback
    account_evt_cb_t evt_cb;  // optional notify callback
    subscriber_node_t *subscribers; // linked list of subscribers
    struct account_node *next;
} account_node_t;

static account_node_t *head = NULL;
static void* mutex = NULL;

// ------------------ Init ------------------
void account_mgr_init(void)
{
    if (!mutex) {
        mutex = _CREATE_LOCK();
        head = NULL;
        ACCOUNT_LOGI(TAG, "Account Manager initialized");
    }
}

// ------------------ Find account ------------------
static account_node_t* find_account_node(const char *name)
{
    account_node_t *curr = head;
    while (curr) {
        if (strcmp(curr->account_name, name) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
}

// ------------------ Create account ------------------
bool account_mgr_create_account(const char *name, void *usr_arg, account_evt_cb_t evt_cb)
{
    if (!mutex || !name) return false;
    _LOCK(mutex, portMAX_DELAY);

    account_node_t *node = find_account_node(name);
    if (!node) {
        node = (account_node_t *)__CALLOC(1, sizeof(account_node_t));
        if (!node) {
            _UNLOCK(mutex);
            return false;
        }
        node->account_name = strdup(name);
        if (!node->account_name) {
            __FREE(node);
            _UNLOCK(mutex);
            return false;
        }
        node->account_usr_arg = usr_arg;
        node->evt_cb = evt_cb;
        node->next = head;
        head = node;
        ACCOUNT_LOGI(TAG, "Account created: %s", name);
    }

    _UNLOCK(mutex);
    return true;
}

// ------------------ Subscribe ------------------
bool account_mgr_subscribe(const char *publisher, const char *subscriber, sub_cb_t cb, void *user_ctx)
{
    if (!mutex || !publisher || !subscriber || !cb) return false;
    _LOCK(mutex, portMAX_DELAY);

    account_node_t *pub_node = find_account_node(publisher);
    account_node_t *sub_node_account = find_account_node(subscriber);
    if (!pub_node || !sub_node_account) {
        _UNLOCK(mutex);
        ACCOUNT_LOGE(TAG, "Publisher or subscriber account doesn't exist");
        return false;
    }

    // Check for duplicate subscription
    subscriber_node_t *sub = pub_node->subscribers;
    while (sub) {
        if (strcmp(sub->name, subscriber) == 0) {
            ACCOUNT_LOGE(TAG, "[%s] already subscribed to [%s]", subscriber, publisher);
            _UNLOCK(mutex);
            return false;
        }
        sub = sub->next;
    }

    // Add new subscriber
    subscriber_node_t *new_sub = (subscriber_node_t *)__CALLOC(1, sizeof(subscriber_node_t));
    if (!new_sub) {
        _UNLOCK(mutex);
        return false;
    }
    new_sub->name = strdup(subscriber);
    if (!new_sub->name) {
        __FREE(new_sub);
        _UNLOCK(mutex);
        return false;
    }
    new_sub->cb = cb;
    new_sub->user_ctx = user_ctx;
    new_sub->next = pub_node->subscribers;
    pub_node->subscribers = new_sub;

    _UNLOCK(mutex);
    ACCOUNT_LOGI(TAG, "[%s] subscribed to [%s]", subscriber, publisher);
    return true;
}

// ------------------ Unsubscribe ------------------
bool account_mgr_unsubscribe(const char *subscriber, const char *publisher, sub_cb_t cb, void *user_ctx)
{
    if (!mutex || !publisher || !subscriber) return false;
    _LOCK(mutex, portMAX_DELAY);

    account_node_t *pub_node = find_account_node(publisher);
    if (!pub_node) {
        _UNLOCK(mutex);
        return false;
    }

    subscriber_node_t *prev = NULL;
    subscriber_node_t *curr = pub_node->subscribers;
    while (curr) {
        if (curr->cb == cb && curr->user_ctx == user_ctx && strcmp(curr->name, subscriber) == 0) {
            if (prev) prev->next = curr->next;
            else pub_node->subscribers = curr->next;

            __FREE(curr->name);
            __FREE(curr);
            _UNLOCK(mutex);
            ACCOUNT_LOGI(TAG, "[%s] unsubscribed from [%s]", subscriber, publisher);
            return true;
        }
        prev = curr;
        curr = curr->next;
    }

    _UNLOCK(mutex);
    return false;
}

// ------------------ Publish ------------------
bool account_mgr_publish(const char *publisher, void *data, size_t len)
{
    if (!mutex || !publisher) return false;
    _LOCK(mutex, portMAX_DELAY);

    account_node_t *pub_node = find_account_node(publisher);
    if (!pub_node) {
        _UNLOCK(mutex);
        return false;
    }

    // Make a copy of subscriber list to avoid executing callbacks under mutex
    subscriber_node_t *sub_copy_head = NULL;
    subscriber_node_t *sub = pub_node->subscribers;
    while (sub) {
        subscriber_node_t *n = (subscriber_node_t *)__CALLOC(1, sizeof(subscriber_node_t));
        if (!n) break;
        n->name = sub->name; // shallow copy, safe
        n->cb = sub->cb;
        n->user_ctx = sub->user_ctx;
        n->next = sub_copy_head;
        sub_copy_head = n;
        sub = sub->next;
    }

    _UNLOCK(mutex);

    // Execute callbacks without holding mutex
    sub = sub_copy_head;
    while (sub) {
        if (sub->cb) {
            sub->cb(publisher, data, len, sub->user_ctx);
        }
        subscriber_node_t *tmp = sub;
        sub = sub->next;
        __FREE(tmp); // __FREE temporary node, not the name
    }

    return true;
}

// ------------------ pull from the publisher ------------------
bool account_mgr_pull(const char *subscriber, const char *publisher, void *data, uint32_t len)
{
    if (!mutex || !subscriber || !publisher) return false;
    _LOCK(mutex, portMAX_DELAY);

    account_node_t *pub_node = find_account_node(publisher);
    if (!pub_node) {
        _UNLOCK(mutex);
        return false;
    }

    // find the subscriber
    subscriber_node_t *sub = pub_node->subscribers;
    while (sub) {
        if (strcmp(sub->name, subscriber) == 0) {
            if (pub_node->evt_cb) {
                // call notify with account_usr_arg
                void *usr_arg = pub_node->account_usr_arg;
                _UNLOCK(mutex);
                pub_node->evt_cb(usr_arg,ACCOUNT_MGR_EVT_TYPE_PULL, data, len);
                return true;
            }
        }
        sub = sub->next;
    }

    _UNLOCK(mutex);
    return false;
}
// ------------------ notify the publisher ------------------
bool account_mgr_notify(const char *subscriber, const char *publisher, void *data, uint32_t len)
{
    if (!mutex || !subscriber || !publisher) return false;
    _LOCK(mutex, portMAX_DELAY);

    account_node_t *pub_node = find_account_node(publisher);
    if (!pub_node) {
        _UNLOCK(mutex);
        return false;
    }

    // find the subscriber
    subscriber_node_t *sub = pub_node->subscribers;
    while (sub) {
        if (strcmp(sub->name, subscriber) == 0) {
            if (pub_node->evt_cb) {
                // call notify with account_usr_arg
                void *usr_arg = pub_node->account_usr_arg;
                _UNLOCK(mutex);
                pub_node->evt_cb(usr_arg,ACCOUNT_MGR_EVT_TYPE_NOTIFY, data, len);
                return true;
            }
        }
        sub = sub->next;
    }

    _UNLOCK(mutex);
    return false;
}
// ------------------ Destroy all accounts ------------------
void account_mgr_destroy(void)
{
    if (!mutex) return;
    _LOCK(mutex, portMAX_DELAY);

    account_node_t *curr = head;
    while (curr) {
        account_node_t *next = curr->next;

        // __FREE subscribers
        subscriber_node_t *sub = curr->subscribers;
        while (sub) {
            subscriber_node_t *snext = sub->next;
            __FREE(sub->name);
            __FREE(sub);
            sub = snext;
        }

        __FREE(curr->account_name);
        __FREE(curr);
        curr = next;
    }
    head = NULL;
    _UNLOCK(mutex);
    _DELECT_LOCK(mutex);
    mutex = NULL;
}
