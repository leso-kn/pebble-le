#
#  SPDX-License-Identifier: MIT
#  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
#
#  pebble-le - Example program CMake build file.
#

add_executable(example
    main.cpp)

target_link_libraries(example PRIVATE
    pebble-le)
