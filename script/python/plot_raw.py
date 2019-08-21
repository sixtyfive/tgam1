#!/usr/bin/env python3

import matplotlib
matplotlib.use('GTK3Cairo')

import sys
import time
import paho.mqtt.client as mqtt
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np

if len(sys.argv) < 2:
    print('Usage: ./mqttplot <MQTT server address>\n')
    exit()

num_datapoints = 350 # theoretically they're being sent at 500 per second
update_interval = 1*1000 # milliseconds
max_adc = 32768
max_uV = 100
mqtt_address = sys.argv[1]
mqtt_port = 1883


def on_connect(client, userdata, flags, rc):
    print('Connecting to MQTT message broker... ', end='')
    client.subscribe('MindFlex/data')
    print('success')


def on_message(client, userdata, msg):
    adc_val = int(msg.payload, 10) # signed short on ESP8266 (i.e. int16_t)
    uV = (adc_val / max_adc) * max_uV # max. estimated microvolts attainable with an on-scalp EEG
    y_data.pop(0)
    y_data.append(uV)


def on_window_close(e):
    mqtt_client.loop_stop()
    print('window closed')


def get_data():
    c = mqtt.Client()
    c.on_connect = on_connect
    c.on_message = on_message
    c.connect(mqtt_address, mqtt_port, 60)
    c.loop_start()
    return c


def animate(i):
    axs.clear()
    axs.plot(x_data, y_data)
    plt.title('EEG Signal Over Time')
    plt.xlabel('t (~ms)')
    plt.ylabel('Signal (~µV)')
    plt.xlim([0,num_datapoints])
    plt.ylim([-(max_uV+10),(max_uV+10)])


if __name__ == '__main__':
    
    x_data = np.linspace(0, num_datapoints, num_datapoints)
    y_data = [0] * num_datapoints

    mqtt_client = get_data()
    fig = plt.figure(figsize=(18,5), dpi=75)
    fig.canvas.mpl_connect('close_event', on_window_close)
    axs = fig.add_subplot(111) # 1 subplot in geometry 1x1
    ani = animation.FuncAnimation(fig, animate, interval=update_interval)
    print('Streaming data... ', end='', flush=True)
    plt.show()
