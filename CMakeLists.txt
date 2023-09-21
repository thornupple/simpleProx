# PROJECT prox
cmake_minimum_required(VERSION 3.22)
include(pico_sdk_import.cmake)
project(simpleProx)
set(PICO_BOARD "pico_w")
pico_sdk_init()

add_executable(simpleProx
    server.c 
    startup.c
    )
target_link_libraries(simpleProx
    pico_stdlib
    pico_btstack_ble
    pico_btstack_cyw43
    pico_cyw43_arch_none
    hardware_gpio
    hardware_timer
    )
target_include_directories(simpleProx PRIVATE
    ${CMAKE_CURRENT_LIST_DIR} # For btstack config
    )
pico_btstack_make_gatt_header(simpleProx PRIVATE "${CMAKE_CURRENT_LIST_DIR}/simpleProx.gatt")

pico_enable_stdio_usb(simpleProx 1)
pico_enable_stdio_uart(simpleProx 0)
pico_add_extra_outputs(simpleProx)

