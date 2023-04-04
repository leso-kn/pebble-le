#
#  SPDX-License-Identifier: MIT
#  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
#
#  pebble-le - Example program CMake build file.
#

set(pebble_le_example_SOURCES
    main.cpp)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(DBus1 REQUIRED dbus-1)

add_executable(example ${pebble_le_example_SOURCES})

add_dependencies(example pebble-le)

target_link_libraries(example PRIVATE
    pebble-le)
