/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

/* define this for the button, undef for the bell */
// by default we're programming a "button" device, not the bell
#define ISBUTTON

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

// this file is my version of the 5 lines below
#include "secrets.h"
/*
// NOTE:  everyone else remove the secrets.h line above
//        and uncomment this block (remove the * and * lines)
const char* ssid = "";        // wifi
const char* password = "";
const char* mqtt_server = ""; // mqtt
const char* username = "";
const char* mqttpass = "";
*/

// NOTE:  make these clientid's _unique_
//        or you'll interfere with the real button/bell
#ifdef ISBUTTON
long lastButtonPush = 0;
const char* clientid = "FoxBuildButton";
#else
const char* clientid = "FoxBuildBell";
#endif

#ifdef ISBUTTON
  // NOTE: wire from Wemos D1 device pin D4 to switch to ground
  // on this device (Wemos D1) GPIO 2 is D4
  // https://www.wemos.cc/sites/default/files/2016-09/mini_new_V2.pdf
  #define BUTTON_PIN D4
#endif


WiFiClient   espClient;
PubSubClient client(espClient);

long lastHeartbeat = 0;
long heartbeatTime = 10000; // 10 sec between heartbeats (for now)
char msg[50];
int value = 0;

long buttonPushRetriggerTime = 5000; // only every seconds
long bellDuration          =   250; // in ms
const long bellDurationMin =   100; // 0.1 s
const long bellDurationMax =  1000; // 1.0 s

void setup() {
  pinMode(5, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);   // NOTE:  If using the Arduino Serial monitor
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

#ifdef ISBUTTON
  pinMode(BUTTON_PIN, INPUT_PULLUP);
#endif

}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {

  payload[length] = '\0'; // force 0 string ending (is this legal)
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length+1; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  long ipayload = atoi((char*)payload);
  Serial.print("ipayload ");
  Serial.println(ipayload);

  char* pnull = 0;
#ifdef ISBUTTON
  if ( strstr(topic,"retrigger") != pnull ) {
    buttonPushRetriggerTime = ipayload;
    Serial.print("change retrigger to ");
    Serial.print(buttonPushRetriggerTime);
    Serial.println("ms");
  }
#else
  if ( strstr(topic,"ring") != pnull ) {
    Serial.println("ring?");
    // Switch on the LED if an 1 was received as first character
    if ( ipayload == 1 ) {
      Serial.println("RING");
      digitalWrite(5, HIGH);   // Turn the bell relay
      delay(bellDuration);     // this is the "duration" of the bell
      digitalWrite(5, LOW);
    } else {
      // Turn the LED off by making the voltage HIGH
      //DigitalWrite(BUILTIN_LED, HIGH);
      Serial.println("message was not 1");
    }
  } // ring
  if ( strstr(topic,"duration") != pnull ) {
    bellDuration = max(bellDurationMin,min(ipayload,bellDurationMax));
    Serial.print("change duration to ");
    Serial.print(bellDuration);
    Serial.println("ms");
  }
#endif

}

void reconnect() {
  // Loop until we're reconnected
  while ( ! client.connected() ) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if ( client.connect(clientid,username,mqttpass) ) {
      Serial.println("connected");
      // Once connected, publish an announcement...
#ifdef ISBUTTON
      client.publish("/fox/build", "The button is now online");
      client.subscribe("/fox/build/button/#");
#else
      client.publish("/fox/build", "The bell is now online");
      // ... and (re)subscribe
      client.subscribe("/fox/build/bell/#");
#endif
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

void loop() {

  if ( ! client.connected() ) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastHeartbeat > heartbeatTime ) {
    lastHeartbeat = now;
    ++value;
#ifdef ISBUTTON
    snprintf (msg, 75, "button #%ld", value);
#else
    snprintf (msg, 75, "bell #%ld", value);
#endif
    Serial.print("Publish /fox/build/heartbeat: ");
    Serial.println(msg);
    client.publish("/fox/build/heartbeat", msg);
  }

#ifdef ISBUTTON
  int sensorVal = digitalRead(BUTTON_PIN);
  if ( ! sensorVal ) {
      snprintf(msg, 75, "%d", !sensorVal);
      //Serial.print("button pushed: ");
      //Serial.println(msg);
      if (now - lastButtonPush > buttonPushRetriggerTime ) {
        lastButtonPush = now;
        Serial.print("mqtt send button pushed: ");
        Serial.println(msg);
        client.publish("/fox/build/bell/ring", msg);
      }
    }
#endif
}
