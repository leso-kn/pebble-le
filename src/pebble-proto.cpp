/* 
*  SPDX-License-Identifier: MIT
*  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
*
*  pebble-le - Pebble communication protocol implementation.
*/

#include "pebble-proto.h"

//

unsigned char transId = 0;

std::map<PebbleEndpoint, std::string> ep_names =
{
    {TIME, "TIME"},
    {FIRMWAREVERSION, "FIRMWAREVERSION"},
    {PHONEVERSION, "PHONEVERSION"},
    {SYSTEMMESSAGE, "SYSTEMMESSAGE"},
    {MUSICCONTROL, "MUSICCONTROL"},
    {PHONECONTROL, "PHONECONTROL"},
    {APPLICATIONMESSAGE, "APPLICATIONMESSAGE"},
    {LAUNCHER, "LAUNCHER"},
    {APPRUNSTATE, "APPRUNSTATE"},
    {LOGS, "LOGS"},
    {PING, "PING"},
    {LOGDUMP, "LOGDUMP"},
    {RESET, "RESET"},
    {APP, "APP"},
    {APPLOGS, "APPLOGS"},
    {NOTIFICATION, "NOTIFICATION"},
    {EXTENSIBLENOTIFS, "EXTENSIBLENOTIFS"},
    {RESOURCE, "RESOURCE"},
    {SYSREG, "SYSREG"},
    {FCTREG, "FCTREG"},
    {APPMANAGER, "APPMANAGER"},
    {APPFETCH, "APPFETCH"},
    {DATALOG, "DATALOG"},
    {RUNKEEPER, "RUNKEEPER"},
    {SCREENSHOT, "SCREENSHOT"},
    {AUDIOSTREAM, "AUDIOSTREAM"},
    {VOICECONTROL, "VOICECONTROL"},
    {NOTIFICATIONACTION, "NOTIFICATIONACTION"},
    {APPREORDER, "APPREORDER"},
    {BLOBDB, "BLOBDB"},
    {PUTBYTES, "PUTBYTES"}
};

UUID::UUID(const std::string &uuid)
{
    static const char characters[] = "0123456789abcdef";
    unsigned char padding = 0;

    for (unsigned char i=0;i<16;i++)
    {
        char a = -1, b = -1;
        while (a < 0)
        {
            for (unsigned char j=0;j<16;j++)
            {
                if (uuid[i*2+padding] == characters[j])
                { a = j; break; }
            }
            if (a < 0) padding++;
        }
        while (b < 0)
        {
            for (unsigned char j=0;j<16;j++)
            {
                if (uuid[i*2+1+padding] == characters[j])
                { b = j; break; }
            }
            if (b < 0) padding++;
        }
        _data[i] = a*16+b;
    }
    return;
}

UUID::operator std::string() const
{
    std::string res;
    static const char characters[] = "0123456789abcdef";

    for (unsigned char i=0;i<16;i++)
    {
        res += characters[_data[i] / 16];
        res += characters[_data[i] % 16];

        if (i == 4 || i == 6 || i == 8 || i == 10)
        res += '-';
    }
    return res;
}

// PebblePacket
PebblePacket::PebblePacket(const uint8_t *data, unsigned int data_len)
{
    this->payload_size = ntohs(*(unsigned short*)data);
    this->endpoint = (PebbleEndpoint)ntohs(*(PebbleEndpoint*)(data+sizeof(unsigned short)));
    this->data = data+sizeof(unsigned short)+sizeof(PebbleEndpoint);
    this->data_len = data_len-sizeof(unsigned short)-sizeof(PebbleEndpoint);
}

std::string PebblePacket::reply(std::string data)
{
    return packet_to_network(endpoint, data);
}

// PhoneVersionPacket
PhoneversionPacket::PhoneversionPacket(PhoneversionRemoteOS os) { this->os = htons(os); };

std::string PhoneversionPacket::to_network()
{
    protocol_version = htonl(protocol_version);
    session_caps = htonl(session_caps);

    os = htonl(os);

    std::string res = packet_to_network(PebbleEndpoint::PHONEVERSION, std::string((const char*)this, sizeof(PhoneversionPacket)));

    from_network();
    return res;
}

void PhoneversionPacket::from_network()
{
    protocol_version = ntohl(protocol_version);
    session_caps = ntohl(session_caps);

    os = ntohl(os);
}

// AppMessagePacket
AppMessagePacket::AppMessagePacket(const uint8_t *data, unsigned int data_len)
{
    *this = *(AppMessagePacket*)data;

    const size_t data_offset = sizeof(UUID)+2;
    this->data = data+data_offset;
    this->data_len = data_len-data_offset;
}

AppMessagePacket::AppMessagePacket(const UUID &app_uuid, DictionaryIterator *iterator)
{
    command = 1; // AppMessagePush
    last_id = transId++;

    this->app_uuid = app_uuid;

    // Data
    auto dict = (std::string*)iterator->dictionary;

    data = (const uint8_t*)dict->data();
    data_len = dict->size();
}

std::string AppMessagePacket::ack(bool ok)
{
    struct
    {
        char ack;
        char target_id;
    } p;

    p.ack = ok ? 0xff : 0x7f;
    p.target_id = last_id;

    return std::string((const char*)&p, sizeof(p));
}

std::string AppMessagePacket::to_network()
{
    std::string p;
    p.resize(data_len+2+sizeof(UUID));
    char *ptr = (char*)p.data();

    put(ptr, char, command);
    put(ptr, char, last_id);

    memcpy(ptr, &app_uuid, sizeof(app_uuid));
    ptr += sizeof(app_uuid);

    memcpy(ptr, data, data_len);

    return packet_to_network(PebbleEndpoint::APPLICATIONMESSAGE, p);
}

std::string packet_to_network(PebbleEndpoint endpoint, const std::string &data)
{
    std::string p;
    p.resize(data.size()+2*sizeof(short));
    char *ptr = (char*)p.data();

    put(ptr, unsigned short, htons(data.size()));
    put(ptr, unsigned short, htons(endpoint));

    memcpy(ptr, data.data(), data.size());

    return p;
}
