/* 
*  SPDX-License-Identifier: MIT
*  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
*
*  pebble-le - C API implementation.
*/

#include "../include/pebble-le/pebble-le.h"

#include <thread>
#include <mutex>
#include <map>

#include "gatt-client.h"
#include "gatt-server.h"
#include "util.h"

#include "pebble-proto.h"

//

namespace libpebble_le
{
    PebblePPoGATTServer *ppogatt_server;
    std::map <std::string, PebblePPoGATTClient*> ppogatt_clients;

    bool ppogatt_initialized = false;
    void ppogatt_init();    
}

void libpebble_le::ppogatt_init()
{
    ppogatt_initialized = true;

    std::mutex m;
    m.lock();

    // GATT Server
    new std::thread([&](){
        auto s = PebblePPoGATTServer();
        ppogatt_server = &s;

        m.unlock();
        s.start();
    });

    m.lock();
}

using namespace libpebble_le;

// API

void pebble_le_connect() { pebble_le_connect_with_bt_addr(""); }

void pebble_le_connect_with_bt_addr(const char *bt_addr)
{
    if (!ppogatt_initialized) ppogatt_init();

    std::mutex m;
    m.lock();

    new std::thread([&]()
    {
        auto c = PebblePPoGATTClient(bt_addr);
        ppogatt_clients[bt_addr] = &c;

        m.unlock();
        c.start();

        ppogatt_clients.erase(bt_addr);
    });

    m.lock();
}

// disable optimization to prevent dealloc segfault on -O2 / -O3
void __attribute__((optimize("0"))) pebble_le_free(const char *bt_addr)
{
    ppogatt_clients[bt_addr]->stop();
}

void pebble_le_register_watch_connectivity(void (*connected_callback)(const char *bt_addr, bool connected))
{
    ppogatt_server->watch_connectivity_callback = connected_callback;
}

void pebble_le_register_inbox_received(AppMessageInboxReceived received_callback)
{
    ppogatt_server->app_message_received_callback = received_callback;
}

void pebble_le_app_message_outbox_begin(DictionaryIterator **iterator)
{
    *iterator = new DictionaryIterator();
    auto &iter = **iterator;

    iter.dictionary = (Dictionary*)new std::string(1, 0); // 1 = char: num of keys in appmessage
    iter.cursor = (Tuple*)(((std::string*)iter.dictionary)->data()+1);
    iter.end = ((char*)iter.cursor)+1;
}

bool pebble_le_app_message_outbox_send(const char *bt_addr, const char *app_uuid, DictionaryIterator *iterator)
{
    if (!ppogatt_server || !ppogatt_server->is_connected(bt_addr)) return false;

    AppMessagePacket msg(UUID(app_uuid), iterator);
    ppogatt_server->send(msg.to_network());
    return true;
}

/*
 *  Pebble Dictionary
 */

bool dict_write_data(DictionaryIterator *iter, const uint32_t key, const uint8_t * const data, const uint16_t size)
{
    auto dict = (std::string*)iter->dictionary;

    size_t start = dict->size();
    dict->resize(dict->size() + 7 + size);

    char* ptr = dict->data() + start;

    put(ptr, uint32_t, key);
    put(ptr, char, TupleType::TUPLE_BYTE_ARRAY);
    put(ptr, uint16_t, size);

    memcpy(ptr, data, size);
    ptr += size;

    iter->cursor = (Tuple*)ptr;
    iter->end = iter->cursor;
    dict->data()[0] += 1; // num of dict elements

    return true;
}
