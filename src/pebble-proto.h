/* 
*  SPDX-License-Identifier: MIT
*  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
*
*  pebble-le - Pebble communicaton protocol interface.
*/

#include <string>
#include <cstring>
#include <inttypes.h>
#include <arpa/inet.h>

#include "../include/pebble-le/pebble-types.h"

#include "util.h"

//

struct UUID
{
    UUID() { };
    UUID(const std::string &uuid);

    bool operator==(const UUID &other);
    operator std::string() const;
private:
    unsigned char _data[16];
};

// From GadgetBridge
enum PebbleEndpoint : signed short
{
    TIME = 11,
    FIRMWAREVERSION = 16,
    PHONEVERSION = 17,
    SYSTEMMESSAGE = 18,
    MUSICCONTROL = 32,
    PHONECONTROL = 33,
    APPLICATIONMESSAGE = 48,
    LAUNCHER = 49,
    APPRUNSTATE = 52,                   // FW >=3.x
    LOGS = 2000,
    PING = 2001,
    LOGDUMP = 2002,
    RESET = 2003,
    APP = 2004,
    APPLOGS = 2006,
    NOTIFICATION = 3000,                // FW 1.x-2-x
    EXTENSIBLENOTIFS = 3010,            // FW 2.x
    RESOURCE = 4000,
    SYSREG = 5000,
    FCTREG = 5001,
    APPMANAGER = 6000,
    APPFETCH = 6001,                    // FW >=3.x
    DATALOG = 6778,
    RUNKEEPER = 7000,
    SCREENSHOT = 8000,
    AUDIOSTREAM = 10000,
    VOICECONTROL = 11000,
    NOTIFICATIONACTION = 11440,         // FW >=3.x, TODO: find a better name
    APPREORDER = (signed short)0xabcd,  // FW >=3.x
    BLOBDB = (signed short)0xb1db,      // FW >=3.x
    PUTBYTES = (signed short)0xbeef,
};

#include <map>
extern std::map<PebbleEndpoint, std::string> ep_names;

enum PhoneversionRemoteOS : signed char
{
    UNKNOWN = 0,
    IOS = 1,
    ANDROID = 2,
    OSX = 3,
    LINUX = 4,
    WINDOWS = 5,
};

std::string packet_to_network(PebbleEndpoint endpoint, const std::string &data);

struct PebblePacket
{
    unsigned short payload_size;
    PebbleEndpoint endpoint;

    const uint8_t *data;
    unsigned int data_len;

    PebblePacket(const uint8_t *data, unsigned int data_len);

    std::string reply(std::string data);
};

struct __attribute__ ((packed)) PhoneversionPacket
{
    char command = 0x01; // AppVersionResponse
    uint32_t protocol_version = -1;
    int32_t session_caps = 0;

    int32_t os;

    char ver_magic = 2;
    char ver_major = 4;
    char ver_minor = 1;
    char ver_patch = 1;

    int64_t flags = 0x00000000000029af;

    PhoneversionPacket(PhoneversionRemoteOS os);

    std::string to_network();
    void from_network();
};

struct AppMessagePacket
{
    char command = sizeof(PhoneversionPacket);
    char last_id;

    UUID app_uuid;

    const uint8_t *data;
    unsigned int data_len;

    AppMessagePacket(const uint8_t *data, unsigned int data_len);
    AppMessagePacket(const UUID &app_uuid, DictionaryIterator *iterator);

    std::string ack(bool ok);
    std::string to_network();
};
