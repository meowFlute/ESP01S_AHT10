# ESP01S_AHT10
Combining some of Amazon's cheapest boards to get temperature and humidity sensing over WiFi!

## AHT10
The AHT10 is widely available for cheap (around $1 at the time of writing) in the form of breakout boards to be communicated with over i2c.

![AHT10](https://github.com/meowFlute/ESP01S_AHT10/assets/11841186/54d7a91d-25e8-4859-8f39-2bc8b368374d)

This is the type of board that I bought, I got 12 of them for $16.99 on Amazon.

## ESP-01S (ESP8266EX breakout)
The ESP8266EX is a WiFi enabled chip from [expressif](https://www.espressif.com/en/products/socs/esp8266). It is an SoC that has a Tensilica L106 MCU, which is a 32-bit RISC unit. 

This thing is similarly cheap and very robust/capable. 

![ESP8266EX_ESP-01S](https://github.com/meowFlute/ESP01S_AHT10/assets/11841186/939da395-da4b-42db-aef8-859b49a650a8)

## ESP8266 RTOS SDK 
I will be using their RTOS SDK, which comes with ***not only*** a FreeRTOS implementation, but also what appears to be handy utilities like configurable OTA support, SSH debugging, etc. 

More info about that [here](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/)

I used their quick setup guide for Linux. Only gotchas I ran into was that I don't want python2 on my computer, so I created a symlink so their utility could access python via python3 (i.e. `sudo ln -s /usr/bin/python3 /usr/bin/python`). This was a shot in the dark, but hey, it worked.

I'm running Ubuntu 22.04.2 and everything else just worked. Set it up on /dev/ttyUSB0 and was off to the races for configuring, building the "hello_world" example, and flashing software.

I also use vim for programming, not their suggested use of Eclipse, so you'll see shell scripts and such related to generating a .json database of the code base and ctags for the codebase as well.

## Flashing
For flashing I picked up yet another cheap amazon purchase, one of many USB to TTL serial converters

![USB_TTL_converter](https://github.com/meowFlute/ESP01S_AHT10/assets/11841186/0cf02151-c0aa-4bc4-a22d-60e2c190f426)

There are plenty of online tutorials for hooking up an ESP8266-based board to a USB-to-TTL converter. I did the following:
- 3.3V from TTL to VCC and EN pins on the ESP-01S
- Ground from TTL to GND and GPIO0 on the ESP-01S
- RX from the TTL to TX from the ESP-01S
- TX from the TTL to RX from the ESP-01S
- Nothing on the 5V USB line

To flash compiled software, I used expressifs `make flash` command which worked out of the box
