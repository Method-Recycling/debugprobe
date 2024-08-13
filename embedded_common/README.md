# embedded_common
Shared embedded code between multiple code bases

## Directories

| Directory | Description |
| - | - |
| Inc | Header files |
| ST25R3911 | Contains header and source files for the ST25R3911 RFID IC (Developed by ST) |
| Src | Source files |
| rfal | Contains header and source files for high level RFID operations (Developed by ST) |

## Files / "Classes"

| Source | Header | Description |
| - | - | - |
| | `b2b_defines.h` | Message type definitions for board to board comms |
|  `circular_buffer.c` | `circular_buffer.h` | Basic implementation of a circular buffer with functions to put, get, peek and check fullness |
| `eeprom.c` | `eeprom.h` | Definitions of EEPROM boundaries and size, EEPROM vars to hold important device info like UID, type. Functions to actually read/write/clear EEPROM |
| `eeprom_evt_queue.c` | `eeprom_evt_queue.h` | Wraps `circular_buffer` to be EEPROM friendly and specifically to store events including a `last_sent_evt` field to record the last event that was sent before the buffer was cleared |
| | `global_defines.h` | Definitions of device types, message types shared across all devices through to the cloud |
| `helpers.c` | `helpers.h` | Little helper functions, primarily functions to pack/unpack various variable types |
| `i2c.c` | `i2c.h` | Helper functions for I2C send/receive in the format: `address, register, data` |
| `log.c` | `log.h` | Nicely formatted logging including function and line it was printed from |
| `lte.c` | `lte.h` | Wraps `sara` in an attempt to make interfacing with the radio hardware abstracted |
| `mktime.c` | `mktime.h` | Helper to convert the RTC's day, month, year, hours, minutes, seconds into a unix timestamp |
| `msg_protocol.c` | `msg_protocol.h` | Serial line protocol similar to PPP, encapsulates data with delimiters and escapes data, also checksums message |
| | `ntp.h` | Definition of the Network Time Protocol response packet |
| `rfid.c` | `rfid.h` | Wraps the ST RFID lib to provide hardware abstracted function to read an RFID tag |
| `rs485.c` | `rs485.h` | Driver for the ST3485 RS485 IC and some basic header packing/unpacking |
| `rtc.c` | `rtc.h` | Helper functions to set RTC datetime and RTC alarm and get RTC time as unix timestamp |
| `sara.c` | `sara.h` | Driver for the U-Blox SARA R4 LTE radio. Contains code to handle sending and receiving of AT commands, with a large amount of edge case handling in regards to the responses of the radio due to several bugs/oddities with the SARA R4 radio |
| `si7021.c` | `si7021.h` | Driver for SI7021 temperature and humidity sensor |
| `uart_dma.c` | `uart_dma.h` | Implementation to wrap ST's hardware DMA and provide a nice single callback containing the data received and the size |


## Tests
- Test building is managed using cmake
- When building tests, TESTING (pre-proc define) is defined to make it convenient to ignore code that needs hardware