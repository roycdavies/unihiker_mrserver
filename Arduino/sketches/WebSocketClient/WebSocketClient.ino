/*
    HTTP over TLS (HTTPS) example sketch

    This example demonstrates how to use
    WiFiClientSecure class to access HTTPS API.
    We fetch and display the status of
    esp8266/Arduino project continuous integration
    build.

    Limitations:
      only RSA certificates
      no support of Perfect Forward Secrecy (PFS)
      TLSv1.2 is supported since version 2.4.0-rc1

    Created by Ivan Grokhotkov, 2015.
    This example is in public domain.
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#ifndef STASSID
#define STASSID "NSA-SPY"
#define STAPSK  "6ddf9f9ce4"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

const char* host = "192.168.1.69";
const int httpsPort = 80;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char fingerprint[] PROGMEM = "F6 21 FB 5D B8 0C A5 11 C2 B1 81 60 4E 90 2E 73 7E D9 42 B8";

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Use WiFiClientSecure class to create TLS connection
  WiFiClient client;
  Serial.print("connecting to ");
  Serial.println(host);

//  Serial.printf("Using fingerprint '%s'\n", fingerprint);
//  client.setFingerprint(fingerprint);

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  String url = "/wotcha/a466be0c-2902-11e9-94cd-35fc01973f9d";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "user-agent:ArduinoESP8266\r\n" +
               "connection:keep-alive, upgrade\r\n" +
               "upgrade:websocket\r\n" + 
               "sec-websocket-version:13\r\n" +
               "sec-websocket-key:JBN2ivI7h0QkHBA8LOQiIA==\r\n" +
               "host:" + host + "\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
    else
    {
      Serial.println(line);     
    }
  }
  
  String line = client.readStringUntil('\n');
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
}

void loop() {
}
