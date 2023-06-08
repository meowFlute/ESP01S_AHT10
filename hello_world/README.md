# Hello World Example

I ripped this from [The ESP8266 RTOS SDK repo](https://github.com/espressif/ESP8266_RTOS_SDK/) as suggested in the [ESP8266 RTOS SDK docs](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/) for a toolchain validation project

## main/hello_world_main.c
All it does it kick off a FreeRTOS process that looks at the chip information, and then counts down from 10 on 1 second intervals and restarts the SoC

## Monitor
You can watch the process using `make monitor`
