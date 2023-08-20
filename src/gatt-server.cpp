/* 
*  SPDX-License-Identifier: MIT
*  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
*
*  pebble-le - Library-internal PPoGATT server implementation.
*/

#include "gatt-server.h"

#include <thread>
#include <simpleble/SimpleBLE.h>

#include <binc/advertisement.h>
#include <binc/agent.h>
#include <binc/logger.h>
#include <binc/utility.h>

#include "pebble-gatt.h"
#include "pebble-proto.h"
#include "util.h"

//

void (*PebblePPoGATTServer::watch_connectivity_callback)(const char *bt_addr, bool connected) = NULL;
AppMessageInboxReceived PebblePPoGATTServer::app_message_received_callback = NULL;
std::map<const Application*, PebblePPoGATTServer*> servers;
std::map<const Device*, PebblePPoGATTServer*> device_servers;

extern unsigned long transId;

void PebblePPoGATTServer_write_char(const Application *app, const char* service_uuid, const char* char_uuid, const std::string &data)
{
    GByteArray *byteArray = g_byte_array_sized_new(data.size());
    g_byte_array_append(byteArray, (guint8*)data.data(), data.size());

    binc_application_set_char_value(app, service_uuid, char_uuid, byteArray);
    binc_application_notify(app, service_uuid, char_uuid, byteArray);
}

void PebblePPoGATTServer_on_connection_state_change(Device *device, ConnectionState state, const GError *error)
{
    if (state == ConnectionState::DISCONNECTED)
    {
        device_servers[device]->watch_connected = false;
        device_servers.erase(device);
    }
}

const char* PebblePPoGATTServer_on_char_read(const Application *app, const char *address, const char* service_uuid, const char* char_uuid)
{
    if (g_str_equal(service_uuid, le_service_server) && g_str_equal(char_uuid, le_characteristic_server_read))
    {
        log("\033[90m[\033[35mPPoGATT\033[90m] received handshake\033[0m\n");
        std::string data({0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1});
        PebblePPoGATTServer_write_char(app, le_service_server, le_characteristic_server_read, data);
        return NULL;
    }
    return BLUEZ_ERROR_REJECTED;
}

const char* PebblePPoGATTServer_on_char_write(const Application *app, const char *address, const char *service_uuid, const char *char_uuid, GByteArray *byteArray)
{
    if (g_str_equal(service_uuid, le_service_server) && g_str_equal(char_uuid, le_characteristic_server_write))
    {
        PebblePPoGATTServer &srv = *servers[app];
        std::string value((char*)byteArray->data, (char*)byteArray->data+byteArray->len);

        log("\033[90m<< From Pebble: %s\033[0m\n", bytes_to_hex_string(value).c_str());

        int header = value[0] & 0xff;
        int command = header & 7;
        int serial = header >> 3;

        switch (command)
        {
            case 0x01: // ACK packet
            srv.ackmap[serial] = 1;
            break;

            case 0x02: // BLE Full Reset
            if (value.size() > 1) srv.send({0x03, 1, 1}, false); // Watch supports window negotiation (PPoGATT v1). Negotiate to acknowledge 1 rx and 1 tx packet at once (disables ack-coalescing)
            else srv.send({0x03}, false);
            srv.sequenceNo = 0;
            break;

            case 0x00: // PPoGATT packet
            // Send ACK
            srv.send({(const char)(((serial << 3) | 1) & 0xff)}, false);

            // Process Chunks
            size_t i = 1; // skip PPoGATT serial (sequence number)
            while (i < value.size())
            {
                bool continue_with_next = false;
                auto part = srv.rx_queue.begin();

                for (; part != srv.rx_queue.end(); part++)
                {
                    if (serial == (part->seq + 1) % 32)
                    {
                        // Continuation
                        for (; i < value.size() && part->pos < part->total; part->pos++)
                        {
                            part->data[part->pos] = value[i];
                            i++;
                        }

                        if (part->pos >= part->total)
                        break;

                        part->seq = serial;

                        // Packet yet incomplete
                        continue_with_next = true;
                        break;
                    }
                }
                if (continue_with_next) continue; // Packet yet incomplete, it is not a New Packet

                if (part == srv.rx_queue.end())
                {
                    // New Packet
                    PebblePacket header((const uint8_t*)value.data() + i, value.size() - i);

                    srv.rx_queue.push_back(RxPart());
                    part = srv.rx_queue.begin()+srv.rx_queue.size()-1;

                    part->total = header.payload_size + sizeof(short) * 2;
                    part->data.resize(part->total);
                    part->seq = serial;
                    part->pos = 0;

                    for (; i < value.size() && part->pos < part->total; part->pos++)
                    {
                        part->data[part->pos] = value[i];
                        i++;
                    }

                    if (part->pos < part->total)
                    continue;
                }

                // Process Packet
                PebblePacket p(part->data.data(), part->total);

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
                            std::string uuid = msg.app_uuid;
                            DictionaryIterator iter;

                            iter.dictionary = (Dictionary*)msg.data;
                            iter.cursor = (Tuple*)((char*)iter.dictionary+1);
                            iter.end = (Tuple*)(((uint8_t*)iter.dictionary) + msg.data_len);

                            bool ack = true;
                            if (p.endpoint == PebbleEndpoint::APPLICATIONMESSAGE)
                            {
                                ack = srv.app_message_received_callback(&iter, uuid.c_str(), address);
                            }
                            srv.send(p.reply(msg.ack(ack)), true, false);
                        }
                        break;
                    }
                    case PebbleEndpoint::PING:
                    {
                        if (p.data[0] == 0)
                        {
                            // Ping
                            char *pong = (char*)p.data;
                            pong[0] = 1; // command: 1 = pong
                            srv.send(std::string(pong, p.data_len), false, false);
                        }
                        break;
                    }
                    case PebbleEndpoint::PHONEVERSION:
                    {
                        PhoneversionPacket version(PhoneversionRemoteOS::LINUX);
                        srv.send(version.to_network(), true, false);

                        // Watch is now ready to receive messages
                        if (!srv.watch_connected)
                        {
                            auto bt_device = binc_adapter_get_device_by_address(srv.bt_adapter, address);
                            device_servers[bt_device] = &srv;
                            binc_device_set_connection_state_change_cb(bt_device, &PebblePPoGATTServer_on_connection_state_change);

                            srv.watch_connected = true;

                            if (PebblePPoGATTServer::watch_connectivity_callback != NULL)
                            srv.watch_connectivity_callback("<watch identification not yet implemented>", true);
                        }

                        break;
                    }
                }

                srv.rx_queue.erase(part);
            }
            break;
        }
        return BLUEZ_ERROR_REJECTED;
    }
    return BLUEZ_ERROR_REJECTED;
}

PebblePPoGATTServer::PebblePPoGATTServer()
{
    log_set_level(LOG_WARN);
    GDBusConnection *dbusConnection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
    g_bus_own_name_on_connection(dbusConnection, "pebble.ble", G_BUS_NAME_OWNER_FLAGS_NONE, NULL, NULL, NULL, NULL);

    loop = g_main_loop_new(NULL, FALSE);
    bt_adapter = binc_adapter_get_default(dbusConnection);

    if (!bt_adapter)
    {
        printf("fatal: failed to get default bluetooth adapter\n");
        return;
    }

    GPtrArray *adv_services = g_ptr_array_new();
    g_ptr_array_add(adv_services, (gpointer*)le_service_server);

    bt_advertisement = binc_advertisement_create();
    binc_advertisement_set_local_name(bt_advertisement, "");
    binc_advertisement_set_services(bt_advertisement, adv_services);
    g_ptr_array_free(adv_services, true);

    binc_adapter_start_advertising(bt_adapter, bt_advertisement);

    app = binc_create_application(bt_adapter);
    servers[app] = this;

    binc_application_add_service(app, le_service_server);
    binc_application_add_characteristic(app, le_service_server, le_characteristic_server_read, GATT_CHR_PROP_READ);
    binc_application_add_characteristic(app, le_service_server, le_characteristic_server_write, GATT_CHR_PROP_WRITE_WITHOUT_RESP | GATT_CHR_PROP_NOTIFY);
    binc_application_add_descriptor(app, le_service_server, le_characteristic_server_write, le_descriptor_configuration, GATT_CHR_PROP_WRITE);

    binc_application_set_char_read_cb(app, &PebblePPoGATTServer_on_char_read);
    binc_application_set_char_write_cb(app, &PebblePPoGATTServer_on_char_write);

    binc_adapter_register_application(bt_adapter, app);

    // TX queue
    new std::thread([&]()
    {
        const size_t MTU = 339-4;

        while (1)
        {
            if (tx_queue.size() <= 0)
            {
                tx_mutex.lock();
                continue;
            }

            // Send chunks
            auto pkt = tx_queue.front();

            for (size_t i = 0; i < pkt.first->size(); i += MTU)
            {
                std::string chunk;
                chunk.resize(1+((i+MTU<pkt.first->size()) ? MTU : pkt.first->size()-i));

                chunk[0] = (sequenceNo << 3) & 0xff;
                memcpy(chunk.data()+1, pkt.first->data()+i, chunk.size()-1);

                ackmap[sequenceNo] = 0;
                PebblePPoGATTServer_write_char(app, le_service_server, le_characteristic_server_write, chunk);

                log("\033[33m>> To Pebble (chunk):\033[90m   %s\033[0m\n", bytes_to_hex_string(chunk).c_str());

                unsigned long mms = 0;
                while (!ackmap[sequenceNo])
                {
                    mms++;
                    usleep(10000);
                    if (mms > 150) break;
                }

                sequenceNo = (sequenceNo + 1) % 32;
            }

            if (pkt.second) pkt.second->unlock();
            else free(pkt.first);

            tx_queue.erase(tx_queue.begin());
        }
    });
}

PebblePPoGATTServer::~PebblePPoGATTServer()
{
    binc_adapter_stop_advertising(bt_adapter, bt_advertisement);
    binc_advertisement_free(bt_advertisement);
    binc_adapter_free(bt_adapter);
}

void PebblePPoGATTServer::send(const std::string &data, bool useChunks, bool block)
{
    if (!useChunks)
    {
        log("\033[33m>> To Pebble:\033[90m   %s\033[0m\n", bytes_to_hex_string(data).c_str());
        PebblePPoGATTServer_write_char(app, le_service_server, le_characteristic_server_write, data);
        return;
    }

    // Split into chunks
    std::mutex m;
    m.lock();

    tx_queue.push_back({(std::string*)(block ? &data : new std::string(data)), &m});
    auto &m_ptr = tx_queue.back().second;

    tx_mutex.unlock();

    if (block) m.lock();
    m_ptr = 0;
}

bool PebblePPoGATTServer::is_connected(std::string bt_addr)
{
    return watch_connected;
}
