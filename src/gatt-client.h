/* 
*  SPDX-License-Identifier: MIT
*  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
*
*  pebble-le - Library-internal PPoGATT client interface.
*/

#include <simpleble/SimpleBLE.h>
#include <mutex>

//

class PebblePPoGATTClient
{
public:
    PebblePPoGATTClient() = default;
    PebblePPoGATTClient(std::string bt_addr);

    void start();
    static bool is_pebble(SimpleBLE::Peripheral p);

private:
    bool connect_pebble(SimpleBLE::Peripheral p);

    void pair_pebble();
    void subscribe_ppogatt();

    std::mutex *_run;
    SimpleBLE::Peripheral peripheral;
    std::string filter_bt_addr;

    unsigned char sequence_no;
    unsigned char trans_id;
};
