cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(serpico_project)

pico_sdk_init()

add_executable(serpico
        serpico.c
        )

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(serpico pico_stdlib hardware_pwm)

pico_enable_stdio_usb(serpico 1)
pico_enable_stdio_uart(serpico 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(serpico)

target_link_libraries(serpico pico_stdlib)
