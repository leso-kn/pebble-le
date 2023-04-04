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
 * @brief 
 * 
 * @param callback 
 */
void
PEBBLE_LE_EXPORT pebble_le_register_watch_connectivity(void (*connected_callback)(const char *bt_addr, bool connected));

/**
 * @brief 
 * 
 * @param callback 
 */
void
PEBBLE_LE_EXPORT pebble_le_register_inbox_received(AppMessageInboxReceived received_callback);

/**
 * @brief 
 * 
 * @param iterator 
 * @param app_uuid 
 */
void
PEBBLE_LE_EXPORT pebble_le_app_message_outbox_begin(DictionaryIterator **iterator);

/**
 * @brief 
 * 
 * @param bt_addr Bluetooth address of the targeted Pebble.
 * @param iterator
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

bool
PEBBLE_LE_EXPORT dict_write_data(DictionaryIterator *iter, const uint32_t key, const uint8_t * const data, const uint16_t size);

#ifdef __cplusplus
}
#endif
#endif // LIBPEBBLE_LE_H
