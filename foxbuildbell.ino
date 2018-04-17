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
//#define ISBUTTON

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

/*
const char* ssid = "";        // wifi
const char* password = "";
const char* mqtt_server = ""; // mqtt
const char* username = "";
const char* mqttpass = "";
*/
#include <secrets.h>

WiFiClient espClient;
PubSubClient client(espClient);

long lastHeartbeat = 0;
long heartbeatTime = 10000; // 10 sec between heartbeats (for now)
char msg[50];
int value = 0;

#ifdef ISBUTTON
long lastButtonPush = 0;
long buttonPushRetriggerTime = 5000; // only every seconds
const char* clientid = "FoxBuildButton";
#else
const char* clientid = "FoxBuildBell";
#endif

void setup() {
  pinMode(5, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

#ifdef ISBUTTON
  pinMode(2, INPUT_PULLUP); // This is the GPIO for the button. Not sure if the INPUT_PULLUP is necessary or effective.
                            // There is an input pullup but I am not sure if it can be defined by hardware like on an UNO.
                            // This is GPIO 2 which is D4 on the Wemos. https://www.wemos.cc/sites/default/files/2016-09/mini_new_V2.pdf
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

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(5, HIGH);// Turn the bell relay
    delay(50);            // this is the "duration" of the bell
    digitalWrite(5, LOW);
  } else {
   //DigitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  Serial.print("message was not 1");
  }

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
#else
      client.publish("/fox/build", "The bell is now online");
      // ... and (re)subscribe
      client.subscribe("/fox/build/bell");
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
  int sensorVal = digitalRead(2);
  if ( ! sensorVal ) {
      snprintf(msg, 75, "%d", !sensorVal);
      //Serial.print("button pushed: ");
      //Serial.println(msg);
      if (now - lastButtonPush > buttonPushRetriggerTime ) {
        lastButtonPush = now;
        Serial.print("mqtt send button pushed: ");
        Serial.println(msg);
        client.publish("/fox/build/bell", msg);
      }
    }
#endif
}