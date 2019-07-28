# foxbuildbell

## Purpose

This is the code for programming the esp8266s used in the button & bell at Fox.Build.

Both the "button" and the "bell" send regular (10s intervals) heartbeat messages to the MQTT server on the `/fox/build/heartbeat` topic (payload is the nth heartbeat sent since boot).

If the "button" is pushed then a MQTT message is sent out to the topic `/fox/build/bell`; code prevents mashing down and holding the button continuously by only allowing a retrigger every 5s (default).

If the "bell" receives a `1` on the `/fox/build/bell` topic, then it pulls pin 5 HIGH for 250ms (default).

## Hardware

The "button" is wired to pin 2 of the esp8266 (and uses the internal pullup resistor).
The "bell" relay is wired to pin 5 of it's respective esp8266 (goes HIGH on receiving a message).

## How to Flash

If you are programming the bell leave `//#define ISBUTTON` commented out\
If you are programming the button uncomment `#define ISBUTTON`

## MQTT messages system responds to:

`/fox/build/bell/ring         1`   ### ring bell\
`/fox/build/bell/duration    <ms>` ### set duration in ms\
`/fox/build/button/retrigger <ms>` ### time before next retrigger

## MQTT message system generates:

`/fox/build   "The button is now online"`\
`/fox/build   "The bell is now online"`\
`/fox/build/heartbeat  "button #<N>"`   ### <N> is the n-th heartbeat since the button came online\
`/fox/build/heartbeat  "bell #<N>"`     ### same for the bell
