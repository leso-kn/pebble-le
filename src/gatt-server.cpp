/* 
*  SPDX-License-Identifier: MIT
*  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
*
*  pebble-le - Library-internal PPoGATT server implementation.
*/

#include "gatt-server.h"

#include <simpleble/SimpleBLE.h>

#include "pebble-gatt.h"
#include "pebble-proto.h"
#include "util.h"

//

void (*PebblePPoGATTServer::watch_connectivity_callback)(const char *bt_addr, bool connected) = NULL;
AppMessageInboxReceived PebblePPoGATTServer::app_message_received_callback = NULL;

PebblePPoGATTServer::PebblePPoGATTServer() : dbus_gatt::DBusGATT("pebble.ble", "/pebble_le")
{
    Services(dbus_gatt::DBusGATT::Service(
        "server",
        le_service_server,
        dbus_gatt::DBusGATT::Characteristic(
            "read",
            le_characteristic_server_read,
            dbus_gatt::kCharacteristicFlagRead | dbus_gatt::kCharacteristicFlagWrite,
            [&]() -> dbus_gatt::DBusGattVariantT {
                log("\033[90m[\033[35mPPoGATT\033[90m] received handshake\033[0m\n");
                return std::string({0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1});
            },
            [&](const uint8_t* value, size_t size) {
                return "";
            }
            ),
        dbus_gatt::DBusGATT::Characteristic(
            "write",
            le_characteristic_server_write,
            dbus_gatt::kCharacteristicFlagRead | dbus_gatt::kCharacteristicFlagWrite | dbus_gatt::kCharacteristicFlagNotify,
            [&]() -> dbus_gatt::DBusGattVariantT {
                return "";
            },
            [&](const uint8_t* value, size_t size) {
                log("\033[90m<< From Pebble: %s\033[0m\n", bytes_to_hex_string(std::string((const char*)value, size)).c_str());

                int header = value[0] & 0xff;
                int command = header & 7;
                int serial = header >> 3;

                switch (command)
                {
                    case 0x01: // ACK packet
                    break;

                    case 0x02: // Heartbeat (presumably)
                    if (size > 1) send({0x03, 0x19, 0x19});
                    else send({0x03});
                    break;

                    case 0x00: // PPoGATT packet
                    // Send ACK
                    send({(const char)(((serial << 3) | 1) & 0xff)});

                    PebblePacket p(value + 1, size - 1);
                    log("[ \033[1;36m%s\033[0m ]\t%s\n", ep_names[p.endpoint].c_str(), bytes_to_hex_string(std::string((const char*)p.data, p.data_len)).c_str());

                    switch (p.endpoint)
                    {
                        case PebbleEndpoint::APPLICATIONMESSAGE:
                        case PebbleEndpoint::LAUNCHER:
                        {
                            // App Message
                            AppMessagePacket msg(p.data, p.data_len);

                            if (msg.command == 1) // AppMessagePush
                            {
                                DictionaryIterator iter;
                                iter.dictionary = (Dictionary*)msg.data;
                                iter.cursor = (Tuple*)iter.dictionary;
                                iter.end = (Tuple*)iter.dictionary + msg.data_len;

                                send(p.reply(msg.ack(
                                    app_message_received_callback(&iter, "<watch identification not yet implemented>")
                                )));
                            }
                            break;
                        }
                        case PebbleEndpoint::PHONEVERSION:
                        {
                            PhoneversionPacket version(PhoneversionRemoteOS::LINUX);
                            send(version.to_network());

                            // Watch is now ready to receive messages
                            if (!watch_connected)
                            {
                                watch_connected = true;
                                if (PebblePPoGATTServer::watch_connectivity_callback != NULL)
                                watch_connectivity_callback("<watch identification not yet implemented>", true);
                            }

                            break;
                        }
                    }
                    break;
                }
                return "";
            }
            )
        ),
        dbus_gatt::DBusGATT::Service(
        "padding",
        le_service_padding)
    );
}

void PebblePPoGATTServer::send(std::string data)
{
    log("\033[33m>> To Pebble:\033[90m   %s\033[0m\n", bytes_to_hex_string(data).c_str());
    setCharacteristicValue(data, "/pebble_le/server/write", false);
}

bool PebblePPoGATTServer::is_connected(std::string bt_addr)
{
    return watch_connected;
}
