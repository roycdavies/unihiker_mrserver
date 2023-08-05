/*
 * WebSocketClient.ino
 *
 *  Created on: 24.05.2015
 *
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>

#include <WebSocketsClient.h>
#include <WebSockets.h>
#include <ESP8266HTTPClient.h>

#include <WiFiClientSecureBearSSL.h>

#include <Hash.h>
#include <SPI.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

#define USE_SERIAL Serial

void getSessionID (uint8_t * sessionID) {
  
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

//  DynamicJsonBuffer jsonBuffer(length+60);
//  JsonObject& root = jsonBuffer.parseObject(payload);

	switch(type) {
		case WStype_DISCONNECTED:
			USE_SERIAL.printf("Disconnected\n");
			break;
		case WStype_CONNECTED:
      // send message to server to verify connection
      webSocket.sendTXT("Connected");
      
			USE_SERIAL.printf("Connected to %s\n", payload);

      // Wotch the channel of interest
      USE_SERIAL.printf("Wotching Channel\n");
      webSocket.sendTXT("{\"sessionid\":\"e9f31182-2a79-11e9-bc43-e5a8722d15ac\", \"command\":\"wotcha\", \"parameters\":{\"id\":\"da1ad352-291c-11e9-bbfc-e5a829982c85\"}}");
			break;
		case WStype_TEXT:
			USE_SERIAL.printf("Received %s\n", payload);
      
      if (strcmp((const char *) payload, "ping") == 0)
      {
        USE_SERIAL.printf("Sending pong\n");
        webSocket.sendTXT("pong");
      }
//      else
//      {
//        root["sensor"].as<char*>()
//      }

			// send message to server
			// webSocket.sendTXT("message here");
			break;
		case WStype_BIN:
			USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
			hexdump(payload, length);

			// send data to server
			// webSocket.sendBIN(payload, length);
			break;
	}

}

void setup() {
	USE_SERIAL.begin(115200);

	USE_SERIAL.setDebugOutput(true);
//
//	USE_SERIAL.println();
//	USE_SERIAL.println();
//	USE_SERIAL.println();
//
//  USE_SERIAL.printf("Connecting");
//	for(uint8_t t = 4; t > 0; t--) {
//		USE_SERIAL.printf(".", t);
//		USE_SERIAL.flush();
//		delay(1000);
//	}
//  USE_SERIAL.printf("\n");
  
  WiFiMulti.addAP("NSA-SPY", "6ddf9f9ce4");
  WiFiMulti.addAP("DaviesNetMain", "6ddf9f9ce4");

	//WiFi.disconnect();
	while(WiFiMulti.run() != WL_CONNECTED) {
		delay(100);
	}

	// server address, port and URL
  webSocket.begin("192.168.1.67", 80, "/wotcha/a466be0c-2902-11e9-94cd-35fc01973f9d");

	// event handler
	webSocket.onEvent(webSocketEvent);

	// try ever 5000 again if connection has failed
	webSocket.setReconnectInterval(5000);

}

void loop() {
	webSocket.loop();
}
