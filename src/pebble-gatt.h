/* 
*  SPDX-License-Identifier: MIT
*  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
*
*  pebble-le - Pebble GATT constants.
*/

// Client UUIDs (Pebble)
#define le_characteristic_pebble_ppogatt_read "30000004-328e-0fbb-c642-1aa6699bdada"
#define le_characteristic_pebble_ppogatt_write "30000006-328e-0fbb-c642-1aa6699bdada"

#define le_characteristic_pebble_pairing_trigger "00000002-328e-0fbb-c642-1aa6699bdada"

#define le_characteristic_connection_parameters "00000005-328e-0fbb-c642-1aa6699bdada"
#define le_characteristic_connectivity "00000001-328e-0fbb-c642-1aa6699bdada"
#define le_characteristic_mtu "00000003-328e-0fbb-c642-1aa6699bdada"

#define le_service_pebble_ppogatt "30000003-328e-0fbb-c642-1aa6699bdada"
#define le_service_pebble_general "0000fed9-0000-1000-8000-00805f9b34fb"

// Server UUIDs (libpebble-le host)
#define le_service_server "10000000-328e-0fbb-c642-1aa6699bdada"
#define le_service_padding "badbadba-dbad-badb-adba-badbadbadbad"

#define le_characteristic_server_write "10000001-328e-0fbb-c642-1aa6699bdada"
#define le_characteristic_server_read "10000002-328e-0fbb-c642-1aa6699bdada"
