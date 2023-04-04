/* 
*  SPDX-License-Identifier: MIT
*  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
*
*  pebble-le - Utility functions implementation.
*/

#include "util.h"

//

PebbleLELogLevel pebble_le_log_level = PebbleLELogLevel::NONE;

std::string bytes_to_hex_string(const std::string &input, int start)
{
  static const char characters[] = "0123456789ABCDEF";

  // Zeroes out the buffer unnecessarily, can't be avoided for std::string.
  std::string ret(input.size() * 3, 0);

  // Hack... Against the rules but avoids copying the whole buffer.
  char *buf = const_cast<char *>(ret.data());

  for (int i = start; i < input.size(); i++)
  {
    *buf++ = characters[((unsigned char)input[i]) >> 4];
    *buf++ = characters[((unsigned char)input[i]) & 0x0F];
    *buf++ = ' ';
  }
  return ret;
}
