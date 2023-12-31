/**
   BasicHTTPSClient.ino

    Created on: 20.08.2018

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClientSecureBearSSL.h>

#define USEREMAIL       "roy@imersia.com"
#define PASSWORD        "1gelk0tt"

// Headers for the MRServer
#define DEVELOPERID     "Test"
#define LOCATION        "rcknn8df0pbs"
#define TOKEN           "Arduino"

// Fingerprint for demo URL, expires on June 2, 2019, needs to be updated well before this date
const uint8_t fingerprint[20] = {0xF6, 0x21, 0xFB, 0x5D, 0xB8, 0x0C, 0xA5, 0x11, 0xC2, 0xB1, 0x81, 0x60, 0x4E, 0x90, 0x2E, 0x73, 0x7E, 0xD9, 0x42, 0xB8}; //{0x5A, 0xCF, 0xFE, 0xF0, 0xF1, 0xA6, 0xF4, 0x5F, 0xD2, 0x11, 0x11, 0xC6, 0x1D, 0x2F, 0x0E, 0xBC, 0x39, 0x8D, 0x50, 0xE0};

ESP8266WiFiMulti WiFiMulti;

void setup() {

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("DaviesNetMain", "6ddf9f9ce4");
}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    
//    client->setFingerprint(fingerprint);

    HTTPClient https;

    https.header("useremail:" USEREMAIL);
    https.header("password:" PASSWORD);
    https.header("developerid:" DEVELOPERID);
    https.header("location:" LOCATION);
    https.header("token:" TOKEN);

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, "https://192.168.1.67/api/sessions")) {  // HTTPS

      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been sent and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }

  Serial.println("Wait 10s before next round...");
  delay(10000);
}
