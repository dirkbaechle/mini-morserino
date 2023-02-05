#ifndef _HELTEC_H_
#define _HELTEC_H_

#include "morsedefs.h"

#define ESP32
#define WIFI_Kit_32

#if defined(ESP32)

#include <Arduino.h>
#if defined( WIFI_Kit_32 ) || defined( Wireless_Stick )
#include <Wire.h>
#include "oled/SSD1306Wire.h"
#include <SPI.h>
#endif


class Heltec_ESP32 {

 public:
    Heltec_ESP32();
	~Heltec_ESP32();

    void begin(bool DisplayEnable=true, bool LoRaEnable=false, bool SerialEnable=true, bool PABOOST=false, long BAND=470E6);

#if defined( WIFI_Kit_32 ) || defined( Wireless_Stick )
    SSD1306Wire *display;
#endif

};

extern Heltec_ESP32 Heltec;

#else
#error "This library only supports boards with ESP32 processor."
#endif


#endif
