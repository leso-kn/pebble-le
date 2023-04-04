/* 
*  SPDX-License-Identifier: MIT
*  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
*
*  pebble-le - Library-internal PPoGATT client interface.
*/

#include <dbus_gatt/dbus_gatt.h>

#include "../include/pebble-le/pebble-types.h"

//

class PebblePPoGATTServer : public dbus_gatt::DBusGATT
{
public:
    PebblePPoGATTServer();
    void send(std::string data);

    static void (*watch_connectivity_callback)(const char *bt_addr, bool connected);
    static AppMessageInboxReceived app_message_received_callback;

    bool is_connected(std::string bt_addr);

private:
    bool watch_connected = false;
};
