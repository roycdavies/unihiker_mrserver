// ---------------------------------------------------------------------------------------------------- 
// Imersia Websocket-To-Geobot connected Arduino EPS8266
// Connects by WIFI to an Imersia MRServer via WebSocket, and reads a Geobot, looking for Triggers
// that come from the Geobot programme 
// ----------------------------------------------------------------------------------------------------

#include <Arduino.h>

#include <iostream>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include <WebSocketsClient.h>
#include <WebSockets.h>

// ----------------------------------------------------------------------------------------------------
// Settings
// ----------------------------------------------------------------------------------------------------

// General Settings
#define DEBUG           false

// A couple of Wifi settings to try
#define SSID1           "DaviesNetMain"
#define SSID1PASS       "6ddf9f9ce4"
#define SSID2           "DaviesNetSub"
#define SSID2PASS       "6ddf9f9ce4"

// The domain name or IP address of the MRServer
#define MRSERVERNAME    "192.168.1.252"

// Login details for the MRServer
#define USEREMAIL       "roy@imersia.com"
#define PASSWORD        "1gelk0tt"

// Headers for the MRServer
#define DEVELOPERID     "Test"
#define LOCATION        "rcknn8df0pbs"
#define TOKEN           "Arduino"

// Geobot to wotch
#define GEOBOTID        "98a13638-30aa-11e9-8cc7-9679072ead2d"

// Trigger to respond to
#define TRIGGEREVENT    "garagedoor"
#define TRIGGEREVENT_LEN  10

// A couple of useful functions
#define STOP_TIMED_EVENTS doing_something=true
#define START_TIMED_EVENTS doing_something=false

// States and Events for the Finite State Machine
enum states   { STARTING, WIFI_CONNECTED, SESSIONID_RECVD, USERID_RECVD, WEBSOCKET_OPEN, GEOBOT_WOTCHED };
enum events   { NONE, START_SERIAL, CONNECT_WIFI, DISCONNECT_WIFI, GET_SESSIONID, GET_USERID, OPEN_WEBSOCKET, WEBSOCKET_CONNECTED, \
                CLOSE_WEBSOCKET, WOTCH_GEOBOT, SEND_PONG, TRIGGER };

// Possible Triggers
enum triggers { EMPTY, PRESS };
bool currently_pressing = false;

// ----------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------
// Global Variables
// ----------------------------------------------------------------------------------------------------

ESP8266WiFiMulti WiFiMulti;         // Wifi Connector
WebSocketsClient webSocket;         // Websocket Connector
char * sessionID;                   // Current SessionID
char * userID;                      // Current UserID

// Finite State Machine Variables
states current_state = STARTING;    // Current State
bool doing_something = false;       // Lock for timed events that will occur only when nothing else is in process.

// Event Queue
const int event_queue_length = 10;
events event_queue [event_queue_length];
int event_queue_pos = -1;

// The trigger value, when it has arrived
triggers current_trigger = EMPTY;

// ----------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------
// Kick things off.
// ----------------------------------------------------------------------------------------------------
void setup ()
{  
    // Open the Serial channel if we can.
  openSerialChannel ();
  
  // Make sure we're in the starting state.
  set_state (STARTING);

  // Cause the first event to be sent.
  START_TIMED_EVENTS;

  // Initialise the LED
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(D7, OUTPUT);
}
// ----------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------
// Main Loop
// ----------------------------------------------------------------------------------------------------
void loop ()
{
  if (event_queue_empty ())
  {
    // Nothing waiting to be processed, so do a timed event (if not doing something else).
    spark_new_event ();     
  }
  else
  {
    // Grab the next event on the queue
    events current_event = get_next_event ();

    // Double check the Wifi is still connected
    if ((WiFiMulti.run() != WL_CONNECTED) && (current_state >= WIFI_CONNECTED))
    {
      current_event = DISCONNECT_WIFI;
    }

    // Process the event to change state
    do_update_state (current_event);
  }

  // Check the web socket
  if (current_state >= USERID_RECVD) 
  {
    webSocket.loop();
  }

  // Flash the LED and wait a bit
  flash_led ();
}
// ----------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------
// Flash the LED.
// This has the effect of making it flash on when something is happening, such as reading the sessionid
// and when all is loaded, the light just pulses faintly, indicating all is well.
// ----------------------------------------------------------------------------------------------------
void flash_led ()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
}
// ----------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------
// Pulse the relay, connected to digital IO pin 6
// ----------------------------------------------------------------------------------------------------
void pulse_relay ()
{
  digitalWrite(D7, HIGH);
  delay(1500);
  digitalWrite(D7, LOW);   
}
// ----------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------
// Set the current state
// ----------------------------------------------------------------------------------------------------
void set_state (states newState)
{
  current_state = newState;
}
// ----------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------
// Queue an event.  If the Queue fills up, earlier events are dropped.
// ----------------------------------------------------------------------------------------------------
void queue_event (events newEvent)
{
  if (event_queue_pos >= event_queue_length-1)
  {
    get_next_event (); // Throw away an event
  }

  // Add the new Event
  event_queue_pos++;
  event_queue[event_queue_pos] = newEvent;
}
// ----------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------
// Test whether the event queue is empty or not
// ----------------------------------------------------------------------------------------------------
bool event_queue_empty ()
{
  return (event_queue_pos == -1);
}
// ----------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------
// Get the next event in the queue.  
// ----------------------------------------------------------------------------------------------------
events get_next_event ()
{
  if (event_queue_empty())
  {
    return NONE;
  }
  else
  {
    // Get the event at the head of the queue
    events next_event = event_queue[0];

    // Reduce the number of elements on the queue
    event_queue_pos--;
   
    // Shuffle the rest of the events down the queue
    if (event_queue_pos > -1)
    {
      for (int i = 0; i < event_queue_pos; i++)
      {
        event_queue[i] = event_queue[i+1];
      }     
    }

    // Return the event
    return next_event;    
  }
}
// ----------------------------------------------------------------------------------------------------



// +--------------------------------------------------------------------------------------------------+
// | Finite State Machine                                                                             |
// +==================================================================================================+
// | STATE                  -> EVENT (action: result)                     -> NEW STATE                |
// +==================================================================================================+
// | STARTING               -> CONNECT_WIFI (connectWifi: true)           -> WIFI_CONNECTED           |
// | STARTING               -> CONNECT_WIFI (connectWifi: false)          -> STARTING                 |
// +--------------------------------------------------------------------------------------------------+
// | WIFI_CONNECTED         -> GET_SESSIONID (getSessionID: true)         -> SESSIONID_RECVD          |
// | WIFI_CONNECTED         -> GET_SESSIONID (getSessionID: false)        -> WIFI_CONNECTED           |
// | WIFI_CONNECTED         -> DISCONNECT_WIFI                            -> STARTING                 |
// +--------------------------------------------------------------------------------------------------+
// | SESSIONID_RECVD        -> GET_USERID (getUserID: true)               -> USERID_RECVD             |
// | SESSIONID_RECVD        -> GET_USERID (getUserID: false)              -> SESSIONID_RECVD          |
// | SESSIONID_RECVD        -> DISCONNECT_WIFI                            -> STARTING                 |
// +--------------------------------------------------------------------------------------------------+
// | USERID_RECVD           -> OPEN_WEBSOCKET (openWebSocket)             -> USERID_RECVD             |
// | USERID_RECVD           -> WEBSOCKET_CONNECTED                        -> WEBSOCKET_OPEN           |
// | USERID_RECVD           -> DISCONNECT_WIFI                            -> STARTING                 |
// +--------------------------------------------------------------------------------------------------+
// | WEBSOCKET_OPEN         -> WOTCH_GEOBOT (wotchGeobot)                 -> GEOBOT_WOTCHED           |
// | WEBSOCKET_OPEN         -> PING (sendPong)                            -> WEBSOCKET_OPEN           |
// | WEBSOCKET_OPEN         -> CLOSE_WEBSOCKET                            -> WIFI_CONNECTED           |
// | WEBSOCKET_OPEN         -> DISCONNECT_WIFI                            -> STARTING                 |
// +--------------------------------------------------------------------------------------------------+
// | GEOBOT_WOTCHED         -> TRIGGER (processTrigger)                   -> GEOBOT_WOTCHED           |
// | GEOBOT_WOTCHED         -> PING (sendPong)                            -> GEOBOT_WOTCHED           |
// | GEOBOT_WOTCHED         -> CLOSE_WEBSOCKET                            -> WIFI_CONNECTED           |
// | GEOBOT_WOTCHED         -> DISCONNECT_WIFI                            -> STARTING                 |
// +==================================================================================================+


void do_update_state (events current_event)
{  
  // Print some debug stuff
  if (DEBUG) Serial.printf ("State %d -> Event %d\n", current_state, current_event);

  // Possibly do a state change, depending on the event
  switch (current_state)
  {
    case STARTING:
      switch (current_event)
      {
        case CONNECT_WIFI:
          STOP_TIMED_EVENTS;
          if (connectWifi ())
          {
            set_state (WIFI_CONNECTED);
          }
          else
          {
            set_state (STARTING);
          }
          START_TIMED_EVENTS;
          break;
        default:
          break;
      }
      break;
    case WIFI_CONNECTED:
      switch (current_event)
      {
        case GET_SESSIONID:
          STOP_TIMED_EVENTS;
          if (getSessionID ())
          {
            set_state (SESSIONID_RECVD);
          }
          else
          {
            set_state (WIFI_CONNECTED);
          }
          START_TIMED_EVENTS;
          break;
        case DISCONNECT_WIFI:
          set_state (STARTING);
          START_TIMED_EVENTS;
          break;
        default:
          break;
      }
      break;
    case SESSIONID_RECVD:
      switch (current_event)
      {
        case GET_USERID:
          STOP_TIMED_EVENTS;
          if (getUserID ())
          {
            set_state (USERID_RECVD);
          }
          else
          {
            set_state (SESSIONID_RECVD);
          }
          START_TIMED_EVENTS;
          break;
        case DISCONNECT_WIFI:
          set_state (STARTING);
          START_TIMED_EVENTS;
          break;
        default:
          break;
      }
      break;
    case USERID_RECVD:
      switch (current_event)
      {
        case OPEN_WEBSOCKET:
          STOP_TIMED_EVENTS;
          openWebSocket ();
          break;
        case WEBSOCKET_CONNECTED:
          set_state (WEBSOCKET_OPEN);
          START_TIMED_EVENTS;
          break;          
        case DISCONNECT_WIFI:
          set_state (STARTING);
          START_TIMED_EVENTS;
          break;
        default:
          break;
      }
      break;
    case WEBSOCKET_OPEN:
      switch (current_event)
      {
        case WOTCH_GEOBOT:
          STOP_TIMED_EVENTS;
          wotchGeobot ();
          set_state (GEOBOT_WOTCHED);
          break;
        case SEND_PONG:
          sendPong ();
          break;
        case CLOSE_WEBSOCKET:
          set_state (WIFI_CONNECTED);
          START_TIMED_EVENTS;
          break;       
        case DISCONNECT_WIFI:
          set_state (STARTING);
          START_TIMED_EVENTS;
          break;
        default:
          break;
      }
      break;
    case GEOBOT_WOTCHED:
      switch (current_event)
      {
        case TRIGGER:
          switch (current_trigger)
          {
            case PRESS:
              Serial.printf("PRESSING\n");
              pulse_relay ();
              current_trigger = EMPTY;
              break;
            default:
              break;
          }
          break;
        case SEND_PONG:
          sendPong ();
          break;
        case CLOSE_WEBSOCKET:
          set_state (WIFI_CONNECTED);
          START_TIMED_EVENTS;
          break;   
        case DISCONNECT_WIFI:
          set_state (STARTING);
          START_TIMED_EVENTS;
          break;
        default:
          break;     
      }
      break;
    default:
      break;
  }
}
// ----------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------
// When there are no events queued, try something, if not already doing something else
// ----------------------------------------------------------------------------------------------------
void spark_new_event ()
{
  if (!doing_something)
  {
    switch (current_state)
    {
      case STARTING:
        queue_event (CONNECT_WIFI);
        break;
      case WIFI_CONNECTED:
        queue_event (GET_SESSIONID);
        break;
      case SESSIONID_RECVD:
        queue_event (GET_USERID);
        break;
      case USERID_RECVD:
        queue_event (OPEN_WEBSOCKET);
        break;
      case WEBSOCKET_OPEN:
        queue_event (WOTCH_GEOBOT);
        break;
      default:
        break;
    }
  }
}
// ----------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------
// Useful Functions
// ----------------------------------------------------------------------------------------------------


// ----------------------------------------------------------------------------------------------------
// Open the Serial port for debugging and info
// ----------------------------------------------------------------------------------------------------
void openSerialChannel ()
{
  Serial.begin (115200);
  Serial.setDebugOutput (DEBUG);

  Serial.printf ("Opening Serial Port .");
  for (uint8_t t = 4; t > 0; t--)
  {
    Serial.printf (".");
    Serial.flush ();
    delay (1000);
  }
  Serial.printf (" OK\n");
}
// ----------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------
// Connect to the WIFI network.
// ----------------------------------------------------------------------------------------------------
bool connectWifi ()
{
  // Set up the Wifi
  Serial.printf ("Joining Wifi ...");
  
  WiFi.persistent(false);            
  WiFi.mode(WIFI_STA);               
  
  WiFiMulti.addAP (SSID1, SSID1PASS);
  WiFiMulti.addAP (SSID2, SSID2PASS);
//  WiFiMulti.addAP (SSID3, SSID3PASS);
//  WiFiMulti.addAP (SSID4, SSID4PASS);

  int counter = 0;
  while ((WiFiMulti.run() != WL_CONNECTED) && (++counter <= 10))
  {
    Serial.printf (".");
    Serial.flush ();
    delay (500);
  }
  
  if (WiFiMulti.run() == WL_CONNECTED)
  {
    Serial.printf (" OK: %s\n", WiFi.SSID().c_str());
    return true;
  }
  else
  {
    Serial.printf (" ERROR\n");
    return false;
  }
}
// ----------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------
// Get a SessionID
// ----------------------------------------------------------------------------------------------------
bool getSessionID ()
{
  Serial.printf ("Getting SessionID ...");
  bool result = false;

  // Create an https Client
  std::unique_ptr<BearSSL::WiFiClientSecure>client (new BearSSL::WiFiClientSecure);
  client->setInsecure ();  // Don't have to worry about updating the fingerprint key when the certs change
  client->setTimeout(10000);
  HTTPClient https;
  
  char * url = "https://" MRSERVERNAME "/api/sessions";

  // Begin talking to the MRServer
  if (https.begin (*client, url))
  {   
    https.setTimeout(10000);
    https.addHeader ("useremail", USEREMAIL);
    https.addHeader ("password", PASSWORD);
    https.addHeader ("developerid", DEVELOPERID);
    https.addHeader ("location", LOCATION);
    https.addHeader ("token", TOKEN);

    // start connection and send HTTP header
    int httpCode = https.GET ();

    // httpCode will be negative on error
    if (httpCode > 0)
    {
      // HTTP header has been sent and Server response header has been handled

      String payload = https.getString ();      
      if (httpCode == HTTP_CODE_OK)
      {
        const size_t capacity = JSON_OBJECT_SIZE(1) + 72;
        StaticJsonBuffer<capacity> jsonBuffer;
        
        JsonObject& root = jsonBuffer.parseObject(payload.c_str());

        if (root.success())
        {
          if (sessionID == NULL) sessionID = new char[37];
          sessionID = strcpy (sessionID, root["sessionid"]);
          Serial.printf (" OK: %s\n", sessionID);
          result = true;
        }
      }
      else
      {
        Serial.printf (" ERROR: %d %s\n", httpCode, payload.c_str());
      }
    }
    else
    {
      Serial.printf (" ERROR: %s\n", https.errorToString (httpCode).c_str());
    }

    https.end();
  }
  else
  {
    Serial.printf (" CONNECTION ERROR\n");
  }

  return result;
}
// ----------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------
// Get the UserID
// ----------------------------------------------------------------------------------------------------
bool getUserID ()
{
  Serial.printf ("Getting UserID ...");
  bool result = false;

  // Create an https Client
  std::unique_ptr<BearSSL::WiFiClientSecure>client (new BearSSL::WiFiClientSecure);
  client->setInsecure ();  // Don't have to worry about updating the fingerprint key when the certs change
  client->setTimeout(10000);
  HTTPClient https;
  
  char * url = "https://" MRSERVERNAME "/api/user";

  // Begin talking to the MRServer
  if (https.begin (*client, url))
  {  
    https.setTimeout(10000);
    https.addHeader ("useremail", USEREMAIL); 
    https.addHeader ("sessionid", sessionID);
    https.addHeader ("developerid", DEVELOPERID);
    https.addHeader ("location", LOCATION);

    // start connection and send HTTP header
    int httpCode = https.GET ();

    // httpCode will be negative on error
    if (httpCode > 0)
    {
      // HTTP header has been sent and Server response header has been handled

      String payload = https.getString ();
      if (httpCode == HTTP_CODE_OK)
      {
        char * pos = strstr(payload.c_str(), "userid");
        if (pos != NULL)
        {
          if (userID == NULL) userID = new char[37];
          strncpy(userID, pos + 9, 36);
          userID[36] = 0;
          Serial.printf (" OK: %s\n", userID);
          result = true;
        }
      }
      else
      {
        Serial.printf (" ERROR: %d %s\n", httpCode, payload.c_str());
      }
    }
    else
    {
      Serial.printf (" ERROR: %s\n", https.errorToString (httpCode).c_str());
    }

    https.end();
  }
  else
  {
    Serial.printf (" CONNECTION ERROR\n");
  }

  return result;
}
// ----------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------
// What to do with Web Socket events
// ----------------------------------------------------------------------------------------------------
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("Websocket Disconnected\n");
      queue_event(CLOSE_WEBSOCKET);
      break;
    case WStype_CONNECTED:
      // send message to server to verify connection
      webSocket.sendTXT("Connected"); 
      // Wotch the Geobot of interest
      queue_event(WEBSOCKET_CONNECTED);
      break;
    case WStype_TEXT:
      Serial.printf("Received %s\n", payload);
      
      if (strcmp((const char *) payload, "ping") == 0)
      {
        queue_event(SEND_PONG);
      }
      else
      {
        // Test if a Trigger Event, but only use those from the Geobot (not the Channel).
        char * pos_event = strstr((const char *) payload, "event");
        char * pos_channelid = strstr((const char *) payload, "channelid");
        if ((pos_event != NULL) && (pos_channelid == NULL))
        {
          if (strncmp(pos_event + 8, TRIGGEREVENT, TRIGGEREVENT_LEN) == 0)
          {
            current_trigger = PRESS;
            queue_event(TRIGGER);
          }
        }
      }
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      hexdump(payload, length);

      // send data to server
      // webSocket.sendBIN(payload, length);
      break;
    case WStype_ERROR:
      Serial.printf("Websocket Error\n");
      break;
    default:
      break;
  }
}
// ----------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------
// Open the Web Socket
// ----------------------------------------------------------------------------------------------------
void openWebSocket ()
{
  Serial.printf ("Opening Websocket ...");

  // server address, port and URL.  NB: should really use secure socket, but doesn't seem to work.
  char url[50];
  strcpy(url, "/wotcha/");
  strcat(url, userID);
  webSocket.begin(MRSERVERNAME, 80, url);

  // event handler
  webSocket.onEvent(webSocketEvent);

  // try in 5000 seconds again if connection has failed
  webSocket.setReconnectInterval(5000);

  Serial.printf (" OK: %s\n", userID);
}
// ----------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------
// Register that we want to wotch a specific Geobot
// ----------------------------------------------------------------------------------------------------
void wotchGeobot ()
{
  const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(3) + 113;
  DynamicJsonBuffer jsonBuffer(capacity);
  
  JsonObject& root = jsonBuffer.createObject();
  root["sessionid"] = sessionID;
  root["command"] = "wotcha";
  JsonObject& parameters = root.createNestedObject("parameters");
  parameters["id"] = GEOBOTID;
  // Wotch the geobot of interest
  Serial.printf("Wotching Geobot ...");
  String webSocketMessage;
  root.printTo(webSocketMessage);
  webSocket.sendTXT(webSocketMessage);
  Serial.printf(" OK: " GEOBOTID "\n");
}
// ----------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------
// Send a reciprocal 'pong' to a 'ping' that was received from the MRServer, to keep the socket open
// ----------------------------------------------------------------------------------------------------
void sendPong ()
{
  Serial.printf("Sending pong\n");
  webSocket.sendTXT("pong");
}
// ----------------------------------------------------------------------------------------------------
