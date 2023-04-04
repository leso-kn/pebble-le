/* 
*  SPDX-License-Identifier: MIT
*  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
*
*  pebble-le - Utility functions interface.
*/

#include <string>

#include "../include/pebble-le/pebble-types.h"

//

extern PebbleLELogLevel pebble_le_log_level;

#ifndef PEBBLE_LE_UTIL_H
#define PEBBLE_LE_UTIL_H

#define put(ptr, T, data) \
    *(T*)ptr = data; \
    ptr+=sizeof(T)

#define log(...) \
    if (pebble_le_log_level) printf(__VA_ARGS__)

std::string bytes_to_hex_string(const std::string &input, int start = 0);

#endif
