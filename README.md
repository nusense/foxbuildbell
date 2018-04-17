# foxbuildbell

## Purpose

This is the code for programming the esp8266s used in the button & bell at Fox.Build.

Both the "button" and the "bell" send regular (10s intervals) heartbeat messages to the MQTT server on the `/fox/build/heartbeat` topic (payload is the nth heartbeat sent since boot).

If the "button" is pushed then a MQTT message is sent out to the topic `/fox/build/bell`; code prevents mashing down and holding the button continuously but only allowing a retrigger every 5s.

If the "bell" receives a `1` on the `/fox/build/bell` topic, then it pulls pin 5 HIGH for 50ms.

## Hardware

The "button" is wired to pin 2 of the esp8266 (and uses the internal pullup resistor).  
The "bell" relay is wired to pin 5 of it's respective esp8266 (goes HIGH on receiving a message).  

## How to Flash

If you are programming the bell leave `//#define ISBUTTON` commented out  
If you are programming the button uncomment `#define ISBUTTON`  

