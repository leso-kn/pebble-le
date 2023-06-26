/* 
*  SPDX-License-Identifier: MIT
*  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
*
*  pebble-le - C API types.
*/

#ifndef LIBPEBBLE_LE_TYPES_H
#define LIBPEBBLE_LE_TYPES_H

#include <stdint.h>

//

#ifdef __cplusplus
extern "C" {
#endif

// From Pebble SDK pebble.h
typedef enum {
  TUPLE_BYTE_ARRAY = 0,
  TUPLE_CSTRING = 1,
  TUPLE_UINT = 2,
  TUPLE_INT = 3
} TupleType;

// From Pebble SDK pebble.h
typedef struct __attribute__((__packed__)) {
  uint32_t key;
  TupleType type:8;
  uint16_t length;

  union {
    uint8_t data[0];
    char cstring[0];
    uint8_t uint8;
    uint16_t uint16;
    uint32_t uint32;
    int8_t int8;
    int16_t int16;
    int32_t int32;
  } value[];
} Tuple;

// From Pebble SDK pebble.h
struct Dictionary;
// From Pebble SDK pebble.h
typedef struct Dictionary Dictionary;

// From Pebble SDK pebble.h
typedef struct {
    Dictionary *dictionary;
    const void *end;
    Tuple *cursor;
} DictionaryIterator;

/**
 * @brief Called after an incoming message is received.
 * @param iterator The dictionary iterator to the received message.
 * @param app_uuid UUID of the watch-app that sent the message.
 * @param bt_addr Bluetooth address of the Pebble from which the message was received.
 * 
 * @return true to return an ACK reply to the watch,
 * @return false to return a NACK reply to the watch.
 */
typedef bool (*AppMessageInboxReceived)(DictionaryIterator *iterator, const char *app_uuid, const char *bt_addr);

enum PebbleLELogLevel
{
  NONE,
  VERBOSE
};

#ifdef __cplusplus
}
#endif
#endif // LIBPEBBLE_LE_TYPES_H
