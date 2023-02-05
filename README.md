# mini-morserino
Stripped down version of the Morserino hardware, for use with a Hailege board
'0.96" ESP32 OLED ESP-WROOM-32 ESP32 ESP-32' (H-1-1290, was available
on Amazon at https://www.amazon.de/dp/B07YKLSKG6, now distributed by hiletgo.com),
which uses an ESP-32-WROOM module.

**DISCLAIMER**: You are currently on the 'main' branch for this project. If you're
looking for basic infos about the used Hailege board and how to get the OLED display
going, switch to the 'board_info' branch
(https://github.com/dirkbaechle/mini-morserino/tree/board_info) instead, please! 


## Assigning new pins

Original usage of GPIO pins:

| GPIO | Type | Basic modes | Function |
| ---- | ---- | ----------- | -------- |
| 37 | I | ADC | Rotary encoder button switch |
| 38 | I | ADC | CLK for rotary encoder |
| 39 | I | ADC | DT for rotary encoder |
| 22 | I/O | GPIO | Speaker output (HF PWM) |
| 23 | I/O | GPIO | Speaker output (LF) |
| 32 | I/O | GPIO, ADC, TOUCH | Right paddle |
| 33 | I/O | GPIO, ADC, TOUCH | Left paddle |
| 0 | I/O | GPIO, ADC, TOUCH | Additonal button switch (Vol/Speed) |


If possible, we don't want to use the pins GPIO4, GPIO5, GPIO12 and GPIO0.

So a new assignment for the Hailege board could be:

| GPIO | Type | Basic modes | Function |
| ---- | ---- | ----------- | -------- |
| 36/SVP | I | ADC | Rotary encoder button switch |
| 25 | I | ADC | DT for rotary encoder |
| 26 | I | ADC | CLK for rotary encoder |
| 16 | I/O | GPIO | Speaker output (HF PWM) |
| 39 | I/O | GPIO | Speaker output (LF) |
| 2 | I/O | GPIO, ADC, TOUCH | Right paddle |
| 14 | I/O | GPIO, ADC, TOUCH | Left paddle |
| 15 | I/O | GPIO, ADC, TOUCH | Additonal button switch (Vol/Speed) |
