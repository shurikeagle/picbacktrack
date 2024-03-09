cmake_minimum_required(VERSION 3.12)

# Name project
SET(PROJECT_NAME picbacktrack)

include(pico_sdk_import.cmake)

project(${PROJECT_NAME})

set(CMAKE_C_STANDARD 11)

# https://github.com/kosma/minmea/blob/master/README.md#limitations
add_compile_options(-Dtimegm=mktime)

pico_sdk_init()

add_executable(${PROJECT_NAME}
    vendor/minmea/minmea.c
    vendor/pico-ssd1306/ssd1306.c
    gps_uart.c
    geo.c
    disp_i2c.c
    main.c)

# to get required header files in current dir
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(
    ${PROJECT_NAME}
    pico_stdlib
    pico_multicore
    hardware_uart
    hardware_i2c
    )

pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

pico_add_extra_outputs(${PROJECT_NAME})