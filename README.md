# mini-morserino
Stripped down version of the Morserino hardware, for use with a Hailege ESP32 OLED board

**DISCLAIMER**: You are currently on a special branch for this project, where I keep
basic infos about the used Hailege board and how to get the OLED display going. If
you're interested in the actual project, switch to the 'main' branch instead, please! 

## The board

The Hailege board '0.96" ESP32 OLED ESP-WROOM-32 ESP32 ESP-32' (H-1-1290, was available
on Amazon at https://www.amazon.de/dp/B07YKLSKG6, now distributed by hiletgo.com),
uses an ESP-32-WROOM module.

![Opened ESP-32-WROOM module](images/20230114_133319.jpg)

Inside, we find an old ESP32-DOWDQ6 with the following
specifications:

* Ordering code: ESP32-D0WDQ6 (NRND)
* Cores: Dual core
* Chip revision: v1.0/v1.1
* Embedded flash/SRAM: no
* Package: QFN 6*6
* VDD_SDIO voltage: 1.8 V/3.3 V


There is also a Flash RAM 25VQ32BTIG, which seems to be a compatible
type to T25Q32BTEGT or HM25Q128A.

The board itself is very similar to the "Wemos Lolin32" (see also https://randomnerdtutorials.com/esp32-built-in-oled-ssd1306/)


## OLED display

From the ESP32, the GPIO pins 4 (I2C_SCLK) and 5 (I2C_SDAT) have a direct connection
to the OLED display. The PIN assignment for the OLED display itself seems to follow
the datasheet of a Vishay OLED-128O064D-BPP3N00000 (https://www.vishay.com/product/37902/).

The I2C address is 0x3C.

When using these values, as is demonstrated in the SSD1306 demo shield, the display is
running fine. For uploading the program from the Arduino IDE to the board, the type
"Wemos Lolin32" can be selected. Also regard the various "gotchas" as listed in the
next section.

If everything is right, you should be greeted with the display demo on your OLED now.

![Start screen of Adafruit demo](images/20230127_182916.jpg)

![Other screen of the Adafruit demo](images/20230127_182935.jpg)

## Gotchas

Gotcha #1. You must press and hold the BOOT button for a few seconds when Arduino or Pio starts to program. Workaround: solder 1.0uF cap from pin 3 (EN) on the module to pin 1 (GND).

Gotcha #2. OLED uses the pins GPIO4 and GPIO5 for I2C. Do not use these for other purposes, else your display will not work.

Gotcha #3. GPIO12 (Pin MTDI) must be logic low on power up or else micro won't boot up. (This is already the case with this Hailege board)

Gotcha #4. GPIO0 must be logic HIGH on power up or else micro won't boot up. (This is already the case with this Hailege board)
