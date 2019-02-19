#ifndef TGAM1_CONFIG_H
#define TGAM1_CONFIG_H

#undef DEBUG
#undef TGAM_RECONFIGURE
#define TGAM_RAW_ONLY
#undef SERIAL_DATA
#define CSV_OVER_SERIAL
#define WIFI
#undef CSV_OVER_MQTT

#define MQTT_SERVER "test.mosquitto.org"
#define MQTT_PORT 1883
#define MQTT_TOPIC_DEBUG "MindFlex/status"
#define MQTT_TOPIC_CSV "MindFlex/data"
#define MQTT_TOPIC_RAW "MindFlex/data"
#define WIFI_SSID "TGAM1"
#define WIFI_PASSWORD "mindflex"

#endif // TGAM1_CONFIG_H
