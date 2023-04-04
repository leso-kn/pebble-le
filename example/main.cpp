/* 
*  SPDX-License-Identifier: MIT
*  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
*
*  pebble-le - Example program.
*/

#include <pebble-le/pebble-le.h>

#include <iostream>
#include <mutex>

//

#define PBL_APP_UUID "WatchApp-UUID-here-0000-000000000000" // TODO argv[1]
#define MESSAGE "Hello :)" // TODO argv[2]

#define MESSAGE_DICT_KEY 1

//

std::mutex *m;

void on_connectivity(const char *bt_addr, bool connected)
{
    if (connected)
    {
        // Send Some Data!
        std::cout << ">> Send Demo Data." << std::endl;

        DictionaryIterator *iter;

        pebble_le_app_message_outbox_begin(&iter);
        dict_write_data(iter, MESSAGE_DICT_KEY, (const unsigned char*)MESSAGE, sizeof(MESSAGE));
        pebble_le_app_message_outbox_send(bt_addr, PBL_APP_UUID, iter);
    }
    else
    {
        // Exit the program
        m->unlock();
    }
}

bool on_app_message(DictionaryIterator *iter, const char *bt_addr)
{
    std::cout << "<< Received an AppMessage saying \"" << iter->cursor->value << "\"" << std::endl;

    // Returning false would send a NACK-packet to Pebble - true means ACK
    return true;
}

int main(int argc, char const *argv[])
{
    // Enable communication logging
    pebble_le_log_level = PebbleLELogLevel::VERBOSE;

    // Subscribe to connectivity events
    pebble_le_register_watch_connectivity(&on_connectivity);
    pebble_le_register_inbox_received(&on_app_message);

    // Connect to a nearby Pebble!
    pebble_le_connect();

    // Use a mutex to keep the program alive until watch disconnects
    m = new std::mutex();
    m->lock();
    m->lock();

    return 0;
}
