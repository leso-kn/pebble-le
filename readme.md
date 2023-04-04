# `libpebble-le`

_A library to talk to Pebble Smartwatches via Bluetooth Low Energy (BLE)._

## Design

This library mimics the API syntax of the on-watch [AppMessage API](https://developer.rebble.io/developer.pebble.com/docs/c/Foundation/AppMessage/index.html).

The backend currently uses the BlueZ v5.48+ D-Bus API (Linux).

## Usage

See [example/main.cpp](example/main.cpp) for example usage.

## License

libpebble-le is licensed under the MIT License.

Internally it uses [dbus_gatt](https://github.com/RomaSo/dbus_gatt) and [SimpleBLE](https://github.com/OpenBluetoothToolbox/SimpleBLE) which are both licensed under the MIT License as well.

Credits also go to [Gadgetbridge](https://codeberg.org/Freeyourgadget/Gadgetbridge) (AGPL-3.0), which provided major insights into the pebble communications protocol, [pebble-proto.h](src/pebble-proto.h) uses some data structures from Gadgetbridge.

## Changelog

### v0.1.0

* Initial version of the library
  * Connect and disconnect to and from watch
  * Scanning and pairing
  * Send and receive [AppMessages](https://developer.rebble.io/developer.pebble.com/docs/c/Foundation/AppMessage/index.html)
    * Pebble dict-implementation currently doesn't parse incoming dictionaries and only supports byte arrays for outgoing messages
  * Callbacks for connectivity events and incoming AppMessages
  * Optional communication logging (disabled by default)
* [Basic example program](example/main.cpp) to demonstrate the API usage
