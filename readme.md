# `libpebble-le`

_A library to talk to Pebble Smartwatches via Bluetooth Low Energy (BLE)._

## Design

This library mimics the API syntax of the on-watch [AppMessage API](https://developer.rebble.io/developer.pebble.com/docs/c/Foundation/AppMessage/index.html).

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
