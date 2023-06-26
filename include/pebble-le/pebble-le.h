/* 
*  SPDX-License-Identifier: MIT
*  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
*
*  pebble-le - C API definition.
*/

#ifndef LIBPEBBLE_LE_H
#define LIBPEBBLE_LE_H

#include <pebble-le-config.h>

#include "pebble-types.h"

//

// Documentation partially copied over from https://developer.rebble.io/developer.pebble.com/docs/c/Foundation/AppMessage/index.html
//                                      and https://developer.rebble.io/developer.pebble.com/docs/c/Foundation/Dictionary/index.html

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Connects to a nearby Pebble Watch via Bluetooth LE.
 *        Performs bluetooth pairing if necessary.
 */
void
PEBBLE_LE_EXPORT pebble_le_connect();

/**
 * @brief Connects to a specific Pebble Watch via Bluetooth LE given its bluetooth address.
 *        Performs bluetooth pairing if necessary.
 *
 * @param bt_addr Bluetooth address of the targeted Pebble.
 */
void
PEBBLE_LE_EXPORT pebble_le_connect_with_bt_addr(const char *bt_addr);

/**
 * @brief Closes the connection with the specified Pebble Watch and frees the related memory.
 *
 * @param bt_addr Bluetooth address of the Pebble to disconnect.
 */
void
PEBBLE_LE_EXPORT pebble_le_free(const char *bt_addr);

/**
 * @brief Registers a function that will be called after a watch has connected or disconnected.
 *        Only one callback may be registered at a time. Each subsequent call to this function will replace the previous callback. The callback is optional; setting it to NULL will deregister the current callback and no function will be called anymore.
 * 
 * @param callback The callback that will be called going forward; NULL to not have a callback.
 */
void
PEBBLE_LE_EXPORT pebble_le_register_watch_connectivity(void (*connected_callback)(const char *bt_addr, bool connected));

/**
 * @brief Registers a function that will be called after any Inbox message is received successfully.
 *        Only one callback may be registered at a time. Each subsequent call to this function will replace the previous callback. The callback is optional; setting it to NULL will deregister the current callback and no function will be called anymore.
 * 
 * @param callback The callback that will be called going forward; NULL to not have a callback.
 */
void
PEBBLE_LE_EXPORT pebble_le_register_inbox_received(AppMessageInboxReceived received_callback);

/**
 * @brief Initialize the given DictionaryIterator for writing to the AppMessage-Outbox.
 * 
 * @param iterator Location to write the DictionaryIterator pointer. This will be NULL on failure.
 */
void
PEBBLE_LE_EXPORT pebble_le_app_message_outbox_begin(DictionaryIterator **iterator);

/**
 * @brief Sends out an app message. This will free the DictionaryIterator containing the message.
 * 
 * @param bt_addr Bluetooth address of the targeted Pebble.
 * @param app_uuid UUID of the watch-app to send the AppMessage to.
 * @param iterator The DictionaryIterator containing the message.
 * @return true on successful send,
 * @return false otherwise.
 */
bool
PEBBLE_LE_EXPORT pebble_le_app_message_outbox_send(const char *bt_addr, const char *app_uuid, DictionaryIterator *iterator);

/**
 * @brief Get / set verbosity of libpebble-le.
 */
extern PebbleLELogLevel
PEBBLE_LE_EXPORT pebble_le_log_level;

/*
 *  Pebble Dictionary
 */

/**
 * @brief Adds a key with a byte array value pair to the dictionary.
 * 
 * @param iter The dictionary iterator
 * @param key The key
 * @param data Pointer to the byte array
 * @param size Length of the byte array
 * @return true on success,
 * @return false otherwise.
 */
bool
PEBBLE_LE_EXPORT dict_write_data(DictionaryIterator *iter, const uint32_t key, const uint8_t * const data, const uint16_t size);

#ifdef __cplusplus
}
#endif
#endif // LIBPEBBLE_LE_H
