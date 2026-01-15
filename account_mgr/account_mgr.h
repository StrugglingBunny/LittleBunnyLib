#ifndef ACCOUNT_MGR_H
#define ACCOUNT_MGR_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C"
{
#endif

   typedef enum
   {
      ACCOUNT_MGR_EVT_TYPE_NOTIFY,
      ACCOUNT_MGR_EVT_TYPE_PULL,

   } ACCOUNT_MGR_EVT_TYPE_T;

   /**
    * @brief Callback type for account subscription
    *
    * @param publisher Name of the publisher account triggering the callback
    * @param data Pointer to published data (can be NULL)
    * @param data_len Length of the data
    * @param user_ctx User context pointer provided during subscription
    */
   typedef void (*sub_cb_t)(const char *publisher, void *data, size_t data_len, void *user_ctx);

   /**
    * @brief Callback type for account notify
    *
    * @param usr_arg User argument associated with the account
    * @param ntf_from Name of the subscriber being notified
    * @param data Pointer to data to send (can be NULL)
    * @param data_length Length of the data
    */
   typedef void (*account_evt_cb_t)(void *usr_arg,ACCOUNT_MGR_EVT_TYPE_T type, void *data, uint32_t data_length);

   /**
    * @brief Initialize the Account Manager
    *
    * Must be called before any other account manager function.
    */
   void account_mgr_init(void);

   /**
    * @brief Create an account
    *
    * @param name Unique string name of the account
    * @param usr_arg User argument passed to notify callback
    * @param notify_cb Optional notify callback for the account
    * @return true if created or already exists, false if failed
    */
   bool account_mgr_create_account(const char *name, void *usr_arg, account_evt_cb_t notify_cb);

   /**
    * @brief Subscribe a subscriber account to a publisher account
    *
    * @param publisher Name of the publisher account
    * @param subscriber Name of the subscriber account
    * @param cb Callback function to invoke when publisher publishes
    * @param user_ctx User context pointer for callback
    * @return true if subscription succeeded, false on error or duplicate
    */
   bool account_mgr_subscribe(const char *publisher, const char *subscriber, sub_cb_t cb, void *user_ctx);

   /**
    * @brief Unsubscribe a subscriber from a publisher
    *
    * @param subscriber Name of the subscriber account
    * @param publisher Name of the publisher account
    * @param cb Callback function used during subscription
    * @param user_ctx User context pointer used during subscription
    * @return true if unsubscription succeeded, false if not found
    */
   bool account_mgr_unsubscribe(const char *subscriber, const char *publisher, sub_cb_t cb, void *user_ctx);

   /**
    * @brief Publish data from a publisher account to all its subscribers
    *
    * @param publisher Name of the publisher account
    * @param data Pointer to data to publish (can be NULL)
    * @param len Length of data
    * @return true if publish succeeded, false if publisher not found
    */
   bool account_mgr_publish(const char *publisher, void *data, size_t len);

   /**
    * @brief Notify a specific subscriber from a publisher account
    *
    * @param subscriber Name of the subscriber account to notify
    * @param publisher Name of the publisher account
    * @param data Pointer to data (can be NULL)
    * @param len Length of data
    * @return true if notification sent, false if accounts not found or subscriber not registered
    */
   bool account_mgr_notify(const char *subscriber, const char *publisher, void *data, uint32_t len);

   /**
    * @brief Destroy all accounts and free all resources
    *
    * After calling this, the manager must be re-initialized before use.
    */
   void account_mgr_destroy(void);

#ifdef __cplusplus
}
#endif

#endif // ACCOUNT_MGR_H
