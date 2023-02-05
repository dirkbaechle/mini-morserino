// Copyright (c) Heltec Automation. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "heltec.h"


Heltec_ESP32::Heltec_ESP32(){

#if defined( WIFI_Kit_32 )
      display = new SSD1306Wire(0x3c, PinSDA, PinSCL, PinResetOled, GEOMETRY_128_64);
#elif defined( Wireless_Stick )
	  display = new SSD1306Wire(0x3c, PinSDA, PinSCL, PinResetOled, GEOMETRY_64_32);
#endif
}

Heltec_ESP32::~Heltec_ESP32(){
#if defined( WIFI_Kit_32 ) || defined( Wireless_Stick )
	delete display;
#endif
}

void Heltec_ESP32::begin(bool DisplayEnable, bool LoRaEnable, bool SerialEnable, bool PABOOST, long BAND) {

	// UART
	if (SerialEnable) {
		Serial.begin(115200);
		Serial.flush();
		delay(50);
		Serial.print("Serial initial done\r\n");
	}

	// OLED
	if (DisplayEnable)
	{
#if defined( Wireless_Stick_Lite ) || defined( Wireless_Bridge )
		if(SerialEnable)
		{
			Serial.print("Wireless Stick Lite and Wireless Bridge don't have an on board display, Display option must be FALSE!!!\r\n");
		}
#endif

#if defined( WIFI_Kit_32 ) || defined( Wireless_Stick )
		display->init();
		//display->flipScreenVertically();
		display->setFont(ArialMT_Plain_10);
		display->drawString(0, 0, "OLED initial done!");
		display->display();

		if (SerialEnable){
			Serial.print("you can see OLED printed OLED initial done!\r\n");
		}
#endif
	}


#if defined( WIFI_Kit_32 ) || defined( Wireless_Stick ) || defined( Wireless_Stick_Lite ) || defined( Wireless_Bridge )

	SPI.begin(SCK,MISO,MOSI,SS);

#endif
}

Heltec_ESP32 Heltec;
