// Copyright (c) Heltec Automation. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "heltec.h"


Heltec_ESP32::Heltec_ESP32(){

#if defined( WIFI_Kit_32 ) || defined( WIFI_LoRa_32 ) || defined( WIFI_LoRa_32_V2 )
      display = new SSD1306Wire(0x3c, PinSDA, PinSCL, PinResetOled, GEOMETRY_128_64);
#elif defined( Wireless_Stick )
	  display = new SSD1306Wire(0x3c, PinSDA, PinSCL, PinResetOled, GEOMETRY_64_32);
#endif
}

Heltec_ESP32::~Heltec_ESP32(){
#if defined( WIFI_Kit_32 ) || defined( WIFI_LoRa_32 ) || defined( WIFI_LoRa_32_V2 ) || defined( Wireless_Stick )
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

#if defined( WIFI_Kit_32 ) || defined( WIFI_LoRa_32 ) || defined( WIFI_LoRa_32_V2 ) || defined( Wireless_Stick )
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

	// LoRa INIT
	if (LoRaEnable)
	{
#if defined(WIFI_Kit_32)
		if(SerialEnable && WIFI_Kit_32){
			Serial.print("The WiFi Kit 32 not have LoRa function, LoRa option must be FALSE!!!\r\n");
		}
#endif


#if defined( WIFI_LoRa_32 ) || defined( WIFI_LoRa_32_V2 ) || defined( Wireless_Stick ) || defined( Wireless_Stick_Lite ) || defined( Wireless_Bridge )
		//LoRaClass LoRa;

		SPI.begin(SCK,MISO,MOSI,SS);
		// TODO Check and fix compile error LoRa.setPins(SS,RST_LoRa,DIO0);
		if (!LoRa.begin(BAND,PABOOST))
		{
			if (SerialEnable){
				Serial.print("Starting LoRa failed!\r\n");
			}
#if defined( WIFI_Kit_32 ) || defined( WIFI_LoRa_32 ) || defined( WIFI_LoRa_32_V2 ) || defined( Wireless_Stick )
			if(DisplayEnable){
				display->clear();
				display->drawString(0, 0, "Starting LoRa failed!");
				display->display();
				delay(300);
			}
#endif
			while (1);
		}
		if (SerialEnable){
			Serial.print("LoRa Initial success!\r\n");
		}
#if defined( WIFI_Kit_32 ) || defined( WIFI_LoRa_32 ) || defined( WIFI_LoRa_32_V2 ) || defined( Wireless_Stick )
		if(DisplayEnable){
			display->clear();
			display->drawString(0, 0, "LoRa Initial success!");
			display->display();
			delay(300);
		}
#endif

#endif
	}
}

Heltec_ESP32 Heltec;
