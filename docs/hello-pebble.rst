Hello Pebble
============

.. note::
    We can use `CMake <https://cmake.org>`_ to compile the below example program. Check out the :ref:`example CMakeLists.txt <CMakeLists.txt>` at the bottom of this page.

The *Hello Pebble* example will connect to a nearby watch and send an `AppMessage <https://developer.rebble.io/developer.pebble.com/docs/c/Foundation/AppMessage/index.html>`_ with the contents ``"Hello :)"`` to a WatchApp of our choice.

After adjusting the target WatchApp's UUID in the code (``PBL_APP_UUID``), we can send a message to it by building and running the example code:

.. code-block:: sh

    > cmake -B bin .
    > cmake --build bin

    > bin/example

main.cpp
********

.. literalinclude:: ../example/main.cpp
    :language: c
    :lines: 8-

.. _CMakeLists.txt:

CMakeLists.txt
**************

.. literalinclude:: ../example/CMakeLists.txt
    :language: cmake
    :lines: 8-
