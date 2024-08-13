file(GLOB_RECURSE COMMON_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.c")
file(GLOB_RECURSE COMMON_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/Inc/*.h")


add_library(Common STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})

target_sources(Common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/circular_buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/eeprom.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/eeprom_evt_queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/helpers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/log.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/lte.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/mktime.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/msg_protocol.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/rfid.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/rs485.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/rtc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/sara.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/uart_dma.c
)

target_include_directories(Common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/STM32L0xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32L0xx/Include
    ${CMAKE_SOURCE_DIR}/Inc
)
