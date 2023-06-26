/* 
*  SPDX-License-Identifier: MIT
*  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
*
*  pebble-le - Library-internal PPoGATT client implementation.
*/

#include "gatt-client.h"

#include <iostream>
#include <functional>
#include <simplebluez/Bluez.h>

#include "pebble-gatt.h"
#include "util.h"

//

namespace PebblePPoGATTServer { extern void (*watch_connectivity_callback)(const char *bt_addr, bool connected); }

void PebblePPoGATTClient::pair_pebble()
{
    peripheral.unpair();
    ((SimpleBluez::Device*) peripheral.underlying())->pair();
    peripheral.write_request(le_service_pebble_general, le_characteristic_pebble_pairing_trigger, {0x09});
}

bool PebblePPoGATTClient::is_pebble(SimpleBLE::Peripheral p)
{
    return p.identifier().find("Pebble ") == 0;
}

bool PebblePPoGATTClient::connect_pebble(SimpleBLE::Peripheral p)
{
    peripheral = p;
    peripheral.connect(); // TODO: Add try catch for this call. Throws when watch fails to connect

    filter_bt_addr = peripheral.address();

    peripheral.set_callback_on_disconnected([&]()
    {
        log("\033[90m[\033[32mdisconnected\033[90m] Pebble %s\033[0m\n", filter_bt_addr.c_str());

        if (PebblePPoGATTServer::watch_connectivity_callback != NULL)
        PebblePPoGATTServer::watch_connectivity_callback(filter_bt_addr.c_str(), false);

        _run->unlock();
    });

    if (!peripheral.services().size()) return false;

    log("\033[90m[\033[32mconnected\033[90m] Pebble %s\033[0m\n", filter_bt_addr.c_str());

    peripheral.notify(le_service_pebble_general, le_characteristic_connection_parameters, [&](SimpleBLE::ByteArray bytes)
    {
        log("\033[90m[\033[35mnotify\033[90m] update received for: Connection Parameters.\033[0m\n");
    });
    peripheral.notify(le_service_pebble_general, le_characteristic_connectivity, [&](SimpleBLE::ByteArray bytes)
    {
        log("\033[90m[\033[35mnotify\033[90m] update received for: Connectivity.\033[0m\n");
    });
    peripheral.notify(le_service_pebble_general, le_characteristic_mtu, [&](SimpleBLE::ByteArray bytes)
    {
        log("\033[90m[\033[35mnotify\033[90m] update received for: MTU.\033[0m\n");
    });

    // Set MTU
    peripheral.write_request(le_service_pebble_general, le_characteristic_mtu, {0x01, 0x53}); // 339

    return true;
}

void PebblePPoGATTClient::subscribe_ppogatt()
{
    peripheral.notify(le_service_pebble_ppogatt, le_characteristic_pebble_ppogatt_read, [&](SimpleBLE::ByteArray bytes) { });
}

PebblePPoGATTClient::PebblePPoGATTClient(std::string bt_addr) { filter_bt_addr = bt_addr; }

void PebblePPoGATTClient::stop()
{
    if (peripheral.is_connected()) peripheral.disconnect();
    else if (_run) _run->unlock();
}

void PebblePPoGATTClient::start()
{
    // GATT Client
    auto adapter = SimpleBLE::Adapter::get_adapters()[0];

    adapter.set_callback_on_scan_found([&](SimpleBLE::Peripheral p)
    {
        log("\033[90m[discover] found device: %s [%s]\033[0m\n", p.identifier().c_str(), p.address().c_str());

        if ((filter_bt_addr.empty() || p.address() == filter_bt_addr) && is_pebble(p))
        {
            log("\033[90m[\033[33mdiscover\033[90m] device is a Pebble!\n");
            if (connect_pebble(p))
            {
                adapter.scan_stop();

                // Pair Watch
                pair_pebble();
                subscribe_ppogatt();
            }
        }
    });

    adapter.set_callback_on_scan_updated([&](SimpleBLE::Peripheral p)
    {
        if ((filter_bt_addr.empty() || p.address() == filter_bt_addr) && is_pebble(p))
        {
            if (!p.is_paired()) peripheral = p;

            if (connect_pebble(p))
            {
                adapter.scan_stop();

                // Pair Watch
                pair_pebble();
                subscribe_ppogatt();
            }
        }
    });

    _run = new std::mutex();
    _run->lock();
    bool paired = false;
    for (auto &p : adapter.get_paired_peripherals())
    {
        if ((filter_bt_addr.empty() || p.address() == filter_bt_addr) && is_pebble(p))
        {
            if (connect_pebble(p))
            {
                paired = true;
                subscribe_ppogatt();
            }
        }
    }

    if (!paired)
    {
        // Not Yet Paired. Scan And Pair
        log("\033[90m[\033[35mdiscover\033[90m] BLE Scan For Pebble%s...\n", filter_bt_addr.empty() ? "" : (" " + filter_bt_addr).c_str());
        adapter.scan_start();
    }

    _run->lock();
}
