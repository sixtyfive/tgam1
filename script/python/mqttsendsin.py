#!/usr/bin/env python3

import sys
import paho.mqtt.client as mqtt
import time
import numpy as np


if len(sys.argv) < 2:
    print('Usage: ./mqttsendsin.py <MQTT server address>\n')
    exit()


mqtt_address = sys.argv[1]
mqtt_port = 1883


def mqtt_connect():
    print('Connecting to MQTT message broker... ', end='')
    c = mqtt.Client()
    c.connect(mqtt_address, mqtt_port, 60)
    print('success')
    return c


if __name__ == '__main__':
    mqtt_client = mqtt_connect()
    print('Streaming data... Ctrl-C to interrupt.', end='')
    sys.stdout.flush()
    while True:
        number = int(np.sin(time.time()) * 16384)
        print(number)
        time.sleep(0.08)
        mqtt_client.publish('MindFlex/data', number)
