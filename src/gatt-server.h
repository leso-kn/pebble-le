/* 
*  SPDX-License-Identifier: MIT
*  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
*
*  pebble-le - Library-internal PPoGATT client interface.
*/

#include <binc/application.h>
#include <binc/device.h>

#include "../include/pebble-le/pebble-types.h"

#include <string>
#include <vector>
#include <mutex>

//

class Advertisment;

struct RxPart
{
    std::vector<uint8_t> data;

    unsigned char seq;
    size_t total;
    size_t pos;
};

class PebblePPoGATTServer
{
public:
    PebblePPoGATTServer();
    ~PebblePPoGATTServer();

    void start() { g_main_loop_run(loop); };
    void send(const std::string &data, bool useChunks = true, bool block = true);

    static void (*watch_connectivity_callback)(const char *bt_addr, bool connected);
    static AppMessageInboxReceived app_message_received_callback;

    bool is_connected(std::string bt_addr);

    bool watch_connected = false;
    unsigned char sequenceNo = 0;
    bool ackmap[32];

    Adapter* bt_adapter;
    std::vector<std::pair<std::string*, std::mutex*>> tx_queue;
    std::vector<RxPart> rx_queue;
    std::mutex tx_mutex;

private:
    GMainLoop *loop;
    Application *app;

    Advertisement* bt_advertisement;
};
