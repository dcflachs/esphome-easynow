# Easynow - an ESPHome component to make ESPNow easy in ESPHome.
### Version 1.1.0
[Check out my blog where im measuring how much power it saves and writing up progress on this project](https://ripnetuk.blogspot.com/)

## Example Config Files
If you have a pair of ESP32s and are running ESPHome you can try this right away thanks to ESPHome's ability to pull external components from a git repo.

Just add the following to your sender and receiver ESPHome YAML and fix up the MAC addresses

You can get the mac address from the logs by having an empty proxy_transmitter or proxy_receiver (you will need to supply empty lists for the peer devices/sensors)

Once these are running, you will see the sensors which are physically on the transmitter appear as entities in HomeAssistant on the receiver.

### Transmitter(s)

```
external_components:
  source:
    type: git
    url: https://gitlab.com/ripnetuk-public/espnow/ripnetuk-esphome-easynow.git
    ref: v1.0.0
  refresh: 1min

sensor:
  - platform: esp32_hall
    id: "sensor_hall"
    update_interval: 1s
  - platform: adc
    pin: 34
    id: "sensor_adc"
    update_interval: 1s

proxy_transmitter:
  espnow_channel: 11
  receiver_mac_address: "34:86:5D:FD:84:58"
  sensors:
    - sensor: "sensor_adc"
      proxy_id: "now_transmitter_adc"
    - sensor: "sensor_hall"
      proxy_id: "now_transmitter_hall"
```
### Receiver

```
external_components:
  source:
    type: git
    url: https://gitlab.com/ripnetuk-public/espnow/ripnetuk-esphome-easynow.git
    ref: v1.0.0
  refresh: 1min

proxy_receiver:
  espnow_channel: 11
  transmitters:
    - mac_address: "34:86:5D:FC:5E:E0"
      name: now-transmitter
      proxied_sensors:
        - proxy_id: "now_transmitter_hall"
          name: "Now Transmitter Hall"
          unit_of_measurement: "µT"
        - proxy_id: "now_transmitter_adc"
          name: "Now Transmitter ADC"
          unit_of_measurement: "V"

    - mac_address: "C8:F0:9E:51:31:E4"
      name: another-transmitter
      proxied_sensors:
        - proxy_id: "t_c1_hall"
          name: "another-transmitter Hall"
          unit_of_measurement: "µT"
        - proxy_id: "t_c1_adc"
          name: "another-transmitter ADC"
          unit_of_measurement: "V"          
```
## Overview

Easynow is a pair of components which allow 1 or more battery powered ESP32 devices running [ESPHome](https://esphome.io/) to proxy sensor data over the [ESPNow](https://www.espressif.com/en/products/software/esp-now/overview ) protocol to a grid powered receiver ESP32 device also running ESPHome.

Once configured in the ESPHome YAML (see above for examples) the receiver will have a regular sensor component created for each relayed sensor.

The sensor states will be updated via ESPNow from the remote device(s).

If the receiver is linked to [HomeAssistant](https://www.home-assistant.io/) the proxied sensors will appear automatically in the dashboard, along with a switch for each remote device to set it into OTA mode next time it wakes up.

## What works so far
- Sensor proxying from 1 or more transmitters to 1 receiver
- Configuration using YAML in the usual ESPHome fashion
- Sensors appear on HomeAssistant without any config outside of the 2 components
- A "ota mode" switch appears on HomeAssistant for each remote device.

## What doesnt work yet (in priority order)

- Actually going to sleep - for now it just waits for 5s before starting the next update.
- Turning off the regular wifi on the transmitters 
- Ability to use a unconfigured instance of these components to determine mac address from log
- Support for esp8266 devices (low priority, but no reason to think its a lot of work, it might already be OK)

## Authors and acknowledgment
Written by George Styles (george@ripnet.co.uk)

## License
This project is released under the MIT License.

## How it works
- The components contain a state machine, with states like READY, AWAIT_CHECKIN_ACK etc
- This is needed because ESPHome uses co-operative multi tasking, so we cannot spend more than 20-30ms in the loop
- So we must write it in a way that it does what it has to do, then yields to other components. We cannot wait in the usual maner by using sleep, or busy waiting while loops
- When a transmitter starts up, it sends a CHECKIN messsage to its receiver.
- The receiver responds with a acknowledgment containing a 'go into OTA mode' flag (which it gets from a switch component in the receiver, which in turn is set from the HomeAssistant UI if connected)
- Once the transmitter has got the acknowledgment, it will reboot into OTA mode if the flag is set
- Otherwise, it will set a 'not read' flag for each of its sensor.
- It hooks the updated state callback on each sensor, and as they are read (in whatever way the sensor usually reads states)
- When it receives a callback it marks that sensor as read and captures the state of the sensor
- Once all sensors are read, it moves onto sensing the states back to the receiver
- It sends each state as an ESPNow message along with a proxy_id which links the real sensor in the transmitter to the proxied sensor in the receiver
- Once its done this, it

(in the next version) WILL GO TO DEEP SLEEP - AT THE MOMENT IT JUST DOESNT DO ANYTHING FOR 5s FOR TESTING

- When it wakes up, it starts the cycle again
## Version History

### 1.0.0
 - Initial public released
 - Sensors relay but with no units

### 1.1.0
 - Receiver sensors are now proper Sensor::sensor objects so have all the YAML config that any other sensor HomeAssistant
 - ESPNow messages are now put into a queue before sending / processing in line with Expresif documentation
 - Readme added
