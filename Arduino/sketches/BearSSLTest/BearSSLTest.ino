#include <WiFiClientSecureBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#define USE_SERIAL Serial
ESP8266WiFiMulti WiFiMulti;

void setup() {
  USE_SERIAL.begin(115200);

  //Serial.setDebugOutput(true);
  USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for(uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  WiFiMulti.addAP("NSA-SPY", "6ddf9f9ce4");
  WiFiMulti.addAP("DaviesNetMain", "6ddf9f9ce4");

//  WiFi.disconnect();
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }
  
  BearSSL::WiFiClientSecure client;
  if (!client.connect("192.168.1.67", 443)) {
    USE_SERIAL.printf("connection failed");
  }
}

void loop() {

}
