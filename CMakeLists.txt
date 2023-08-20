#
#  SPDX-License-Identifier: MIT
#  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
#
#  pebble-le - Main CMake build file.
#

cmake_minimum_required(VERSION 3.0)
project(pebble-le VERSION 0.2.2)

# Build Options
option(BUILD_UNIT_TESTS "Enable unit tests" OFF)
option(PEBBLE_LE_BUILD_EXAMPLE "Build example program" ON)

# Sources
set(pebble_le_SOURCES
    src/pebble-le.cpp
    src/pebble-proto.cpp
    src/gatt-client.cpp
    src/gatt-server.cpp
    src/util.cpp)

# Third-Party Dependencies
add_subdirectory(third_party/SimpleBLE/simpleble EXCLUDE_FROM_ALL)
add_subdirectory(third_party/binc/binc)

set(SimpleBLE_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/third_party/SimpleBLE/simpleble/include
    ${CMAKE_SOURCE_DIR}/third_party/SimpleBLE/simplebluez/include
    ${CMAKE_SOURCE_DIR}/third_party/SimpleBLE/simpledbus/include
    ${CMAKE_BINARY_DIR}/third_party/SimpleBLE/simpleble/export)

set(Binc_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/third_party/binc)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(GIO REQUIRED gio-2.0)
pkg_search_module(DBus1 REQUIRED dbus-1)

# libpebble-le
add_library(pebble-le_o OBJECT ${pebble_le_SOURCES})

add_library(pebble-le SHARED)
add_library(pebble-le_static STATIC)

target_link_libraries(pebble-le pebble-le_o)
target_link_libraries(pebble-le_static pebble-le_o)

set_target_properties(pebble-le PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

set_target_properties(pebble-le_static PROPERTIES
    OUTPUT_NAME pebble-le)

target_include_directories(pebble-le_o PRIVATE ${Binc_INCLUDE_DIRS} ${SimpleBLE_INCLUDE_DIRS} ${DBus1_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})
target_include_directories(pebble-le_o PUBLIC include ${CMAKE_BINARY_DIR}/exports)

target_link_libraries(pebble-le_o PUBLIC
    Binc
    simpleble
    ${DBus1_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES})

include(GenerateExportHeader)
generate_export_header(pebble-le_o
    EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/exports/pebble-le-config.h
    BASE_NAME PEBBLE_LE)

set_property(TARGET pebble-le_o PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET Binc PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(Binc PRIVATE ${GLIB_INCLUDE_DIRS})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pebble-le.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/pebble-le.pc @ONLY)

if (PEBBLE_LE_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

install(
    TARGETS pebble-le
    EXPORT pebble-le-config
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
    TARGETS pebble-le_static
    EXPORT pebble-le-config
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include/pebble-le/ ${CMAKE_BINARY_DIR}/exports/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pebble-le)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/pebble-le.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

install(
    FILES src/dbus-policy.conf
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/dbus-1/system.d
    RENAME pebble-le.conf)
