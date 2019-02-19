// ================================================= //
// ESP-01 pinout:
//
// VCC RST CHPD TXD0
//   8   6   4   2
//   7   5   3   1    ----------------------------
// RXD0 IO0 IO2 GND   \/ Antenna this direction \/
//                    ----------------------------
//
// Wiring between TGAM1 and ESP-01:
//
//   TGAM TX ->|- GPIO2
//   TGAM RX -|<- GPIO0
//
// (Diodes need to be fast Schottkys. The choice of pins
// is such that both, programming and serial console of
// cheap ESP-01 USB programmers off of eBay still works)
//
// ================================================= //
// This code is intended for devices where the TGAM1 //
// TX rate has been configured to 57600 baud through //
// connection of solder point B1 to VCC and removal  //
// of the 10k resistor northwest of B1!              //
// ================================================= //

#include "config.h"

#define GPIO0 0 // HIGH to run, LOW to program
#define GPIO1 1 // TXD0 for hardware serial #1
#define GPIO2 2 // no special functions; free pin
#define GPIO3 3 // RXD0 for hardware serial #1

#ifdef DEBUG
#define debug(x) Serial.print(x)
#define debugln(x) do {Serial.println(x); Serial.flush();} while (false)
#else
#define debug(x)
#define debugln(x)
#endif // DEBUG

#include <SoftwareSerial.h>
SoftwareSerial TGTX(-1, GPIO0);
SoftwareSerial TGRX(GPIO2, -1);

#include <ThinkGearStreamParser.h>
ThinkGearStreamParser TGAMParser;

#define MAX_BUFFER_LENGTH 24
char buffer[MAX_BUFFER_LENGTH]; // to hold bytes received
uint16_t pbValues[8]; // EEG power band values

#ifdef WIFI
#define MQTT
#endif // WIFI

#ifdef MQTT
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
WiFiClient wifi_client;

#include <PubSubClient.h>
const char* mqtt_server = MQTT_SERVER;
const int mqtt_port = MQTT_PORT;
PubSubClient mqtt_client(wifi_client);

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("MQTT message received");  
  (void)topic; (void)payload; (void)length; // to prevent the compiler from warning about unused parameters
}

void mqtt_reconnect() {
  Serial.print("Connecting to MQTT... ");

  if (mqtt_client.connect("MindFlex")) {
    Serial.print("connected... ");
    mqtt_client.publish(MQTT_TOPIC_DEBUG, "connected");
    Serial.println("first message published!");
  } else {
    Serial.print("failed with state [");
    Serial.print(mqtt_client.state());
    Serial.println("] :(");
    delay(10000);
  }
}
#endif // MQTT

class EEGData {
  private:
    uint8_t _poor_quality;
    uint8_t _attention; uint8_t  _meditation;
    int16_t _raw_signal;
    uint16_t _delta; uint16_t _theta;
    uint16_t _low_alpha; uint16_t _high_alpha;
    uint16_t _low_beta; uint16_t _high_beta;
    uint16_t _low_gamma; uint16_t _mid_gamma;
    char _csv_buffer[3*1 + 1*2 + 8*3 + 8*1 + 1];
    char _raw_buffer[5*1 + 1*1 + 1];

  public:
    EEGData();
    void poorQuality(uint8_t value); 
    void attention(uint8_t value);
    void meditation(uint8_t value);
    void rawSignal(int16_t value);
    void eegPowerInt(uint16_t values[]);
    void update();
    const char *getCSVLine() const;
    const char *getRawSignal() const;

}; EEGData eeg_data;

EEGData::EEGData() {
  _poor_quality = 200; // == "no signal"
  _attention = 0; _meditation = 0;
  _raw_signal = 0;
  _delta = 0; _theta = 0;
  _low_alpha = 0; _high_alpha = 0;
  _low_beta = 0; _high_beta = 0;
  _low_gamma = 0; _mid_gamma = 0;
}

void EEGData::poorQuality(uint8_t value) {
  _poor_quality = value;
}

void EEGData::attention(uint8_t value) {
  _attention = value;
}

void EEGData::meditation(uint8_t value) {
  _meditation = value;
}

void EEGData::rawSignal(int16_t value) {
  _raw_signal = value;
}

void EEGData::eegPowerInt(uint16_t values[]) {
  _delta      = values[0];
  _theta      = values[1];
  _low_alpha  = values[2];
  _high_alpha = values[3];
  _low_beta   = values[4];
  _high_beta  = values[5];
  _low_gamma  = values[6];
  _mid_gamma  = values[7];
}

void EEGData::update() {  
#if defined(CSV_OVER_SERIAL) || defined(CSV_OVER_MQTT)
  snprintf(_csv_buffer, sizeof(_csv_buffer), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
    _poor_quality,
    _attention, _meditation,
    _raw_signal,
    _delta, _theta, _low_alpha, _high_alpha, _low_beta, _high_beta, _low_gamma, _mid_gamma
  );
#endif // CSV_OVER_SERIAL || CSV_OVER_MQTT

  snprintf(_raw_buffer, sizeof(_raw_buffer), "%d", _raw_signal);
}

const char *EEGData::getCSVLine() const {
  return &_csv_buffer[0];
}

const char *EEGData::getRawSignal() const {
  return &_raw_buffer[0];
}

// "a single big-endian 16-bit two's-compliment signed value 
//  (high-order byte followed by low-order byte) (-32768 to 32767)"
// http://developer.neurosky.com/docs/doku.php?id=thinkgear_communications_protocol#code_definitions_table
        // a 'short' on an ESP8266, anyways
int16_t to_short(unsigned char hbyte, unsigned char lbyte) {
  union Bytes {
    byte b[2];
    int16_t value;
  }; union Bytes result; // HACK! Union are not supposed to be used like that...

  result.b[0] = hbyte;
  result.b[1] = lbyte;

  return result.value;
}

// "eight big-endian 3-byte unsigned integer values representing
//  delta, theta, low-alpha, high-alpha, low-beta, high-beta,
//  low-gamma, and mid-gamma EEG band power values"
// http://developer.neurosky.com/docs/doku.php?id=thinkgear_communications_protocol#code_definitions_table
uint16_t to_ushort(unsigned char hbyte, unsigned char mbyte, unsigned char lbyte) {
  union Bytes {
    byte b[3];
    uint16_t value;
  }; union Bytes result; // ... I'm lazy though, and don't feel like lots of << and |.
                         //  `"For every problem there is a solution that is simple, neat and wrong."'
                         //     --timemage#arduino@freenode
  result.b[0] = hbyte;
  result.b[1] = mbyte;
  result.b[2] = lbyte;

  return result.value;
}

void extract_power_band_values()
{
  uint8_t pos;
  
  for (uint8_t i=0; i<8; i++) {
    pos = 3 * i;
    pbValues[i] = to_ushort(buffer[pos+0], buffer[pos+1], buffer[pos+2]);
  }
}

void data_send() {
  debug("Sending data... ");
  eeg_data.update();

#ifdef SERIAL_DATA
#ifdef CSV_OVER_SERIAL
  Serial.println(eeg_data.getCSVLine());
#else
  Serial.println(eeg_data.getRawSignal());
#endif // CSV_OVER_SERIAL
#endif // SERIAL_DATA

#ifdef MQTT
#ifdef CSV_OVER_MQTT
  mqtt_client.publish(MQTT_TOPIC_CSV, eeg_data.getCSVLine());
#else
  mqtt_client.publish(MQTT_TOPIC_RAW, eeg_data.getRawSignal());
#endif // CSV_OVER_MQTT
#endif // MQTT

  debugln("success");
}

void data_handler(unsigned char extendedCodeLevel,
                  unsigned char code,
                  unsigned char numBytes,
                  const unsigned char *value,
                  void *customData) {
  if (extendedCodeLevel == 0) { // the only one supposed to be used with either F/W Rev. 1.6 or 1.7
    uint8_t bpos = 0;           // http://developer.neurosky.com/docs/doku.php?id=thinkgear_communications_protocol#code_definitions_table
    uint8_t blen = numBytes;
    
    while(numBytes) {
      bpos = bpos % MAX_BUFFER_LENGTH; // buffer overflow safeguard
      buffer[bpos++] = *value;
      ++value;
      --numBytes;
    };

    switch (code) {
      case PARSER_CODE_POOR_QUALITY:
      debug("Quality (lower is better): ");
      debugln((uint8_t)buffer[0]);
      eeg_data.poorQuality((uint8_t)buffer[0]);
      break;

      case PARSER_CODE_ATTENTION:
      debug("eSense attention value: ");
      debugln((uint8_t)buffer[0]);
      eeg_data.attention((uint8_t)buffer[0]);
      break;

      case PARSER_CODE_MEDITATION:
      debug("eSense meditation value: ");
      debugln((uint8_t)buffer[0]);
      eeg_data.meditation((uint8_t)buffer[0]);
      break;

      case PARSER_CODE_RAW_SIGNAL:
      int16_t raw_signal;
      raw_signal = to_short(buffer[0], buffer[1]);
      debugln(raw_signal);
      eeg_data.rawSignal(raw_signal);
      break;

      // F/W 1.6 only
      case PARSER_CODE_ASIC_EEG_POWER_INT:
      debugln("ASIC-calculated EEG powers");
      extract_power_band_values();
      debug("\tdelta: ");      debugln(pbValues[0]);
      debug("\ttheta: ");      debugln(pbValues[1]);
      debug("\tlow alpha: ");  debugln(pbValues[2]);
      debug("\thigh alpha: "); debugln(pbValues[3]);
      debug("\tlow beta: ");   debugln(pbValues[4]);
      debug("\thigh beta: ");  debugln(pbValues[5]);
      debug("\tlow gamma: ");  debugln(pbValues[6]);
      debug("\tmid gamma: ");  debugln(pbValues[7]);
      eeg_data.eegPowerInt(pbValues);
      break;

      // F/W 1.7 only
      case PARSER_CODE_BATTERY:
      case PARSER_CODE_8BITRAW_SIGNAL:
      case PARSER_CODE_RAW_MARKER:
      case PARSER_CODE_EEG_POWERS:
      debugln("WARNING: F/W Rev. 1.7 device, undefined state!");
      
      default:
      debug("WARNING: ");
      debug(code);
      debugln(": Unknown packet code!");
      debug(" (");
      debug(blen);
      debugln(" bytes)");
      break;
    }

    data_send(); // output to serial console and/or publish via MQTT
    (void)customData; (void)blen; // to keep the compiler from warning about unused variables
  }
}

// specifically for a H/W Rev. 2.8, F/W Rev. 1.7 device that
// has had hardware configuration for 57.6k baud already.
// http://developer.neurosky.com/docs/doku.php?id=thinkgear_communications_protocol#firmware_16_command_byte_table
//
// TODO: TGAM_RAW_ONLY values don't work, it keeps sending the power band values anyways - why???
void tgam1_28_16_init() {
  TGTX.begin(57600);
#ifdef TGAM_RECONFIGURE
#ifdef TGAM_RAW_ONLY
  TGTX.write((uint8_t)(0x00 | 0x04 | 0x08));
#else
  TGTX.write((uint8_t)(0x00 | 0x01 | 0x02 | 0x04 | 0x08)); // page 0; eSense attention, eSense meditation, raw, 57.6k baud
#endif // TGAM_RAW_ONLY
  TGTX.flush();
  TGTX.end();
  delay(1500);
  TGTX.begin(57600); // try to reconnect
#ifdef TGAM_RAW_ONLY
  TGTX.write((uint8_t)(0x10 | 0x01));
#else
  TGTX.write((uint8_t)(0x10 | 0x01 | 0x02)); // page 1; raw + pre-calc'd EEG powers
#endif // TGAM_RAW_ONLY
  TGTX.end();
  delay(1000);
#endif // TGAM_RECONFIGURE
}

// specifically for a H/W Rev. 2.8, F/W Rev. 1.7 device that
// has had hardware configuration for 57.6k baud already.                             
// http://developer.neurosky.com/docs/doku.php?id=thinkgear_communications_protocol#firmware_17_command_byte_table
/*void tgam1_28_17_init() {
  TGTX.begin(57600);
  TGTX.write((uint8_t)(0x60 | 0x03)); // page 6; set speed (57.6k baud)
  TGTX.flush();
  TGTX.end();
  delay(1500);
  TGTX.begin(57600); // try to reconnect
  TGTX.write((uint8_t)(0x02)); // page 0; set speed and mode (normal+raw) once more
  TGTX.flush();
  delay(1500);
  TGTX.write((uint8_t)(0x12 | 0x01)); // page 1; 10/8 bits for raw / eeg powers
  TGTX.write((uint8_t)(0x20 | 0x01 | 0x02 | 0x04)); // page 2; quality, int powers, battery
  TGTX.write((uint8_t)(0x30 | 0x01 | 0x02)); // page 3; eSense attention, eSense meditation
  TGTX.flush();
  TGTX.end();
  delay(1000);
}*/

void tgam1_28_receive() {
  TGRX.begin(57600);
  TGRX.listen(); // just to be safe, as there are two instances of SoftwareSerial now!
}

#ifdef WIFI
void wifi_connect() {
  Serial.println("Connecting to WiFi network... ");
  WiFiManager wifiManager;
  wifiManager.setTimeout(180);
  if (!wifiManager.autoConnect(WIFI_SSID, WIFI_PASSWORD)) {
    Serial.println("timed out :(");
    delay(2500);
    ESP.reset();
  }
  Serial.println("connected!");
}
#endif // WIFI

#ifdef MQTT
void mqtt_connect() {
  mqtt_client.setServer(mqtt_server, mqtt_port);
  mqtt_client.setCallback(mqtt_callback);
}
#endif // MQTT

void setup() {
  delay(3500); // to give myself some time to re-plug cables if needed
  
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  Serial.println("Hello there!");

#ifdef MQTT
  wifi_connect();
  mqtt_connect();
#endif

  tgam1_28_16_init();
  tgam1_28_receive();

  // parser, parserType, handleDataValueFunc(extendedCodeLevel, code, numBytes, value, customData), customData
  THINKGEAR_initParser(&TGAMParser, PARSER_TYPE_PACKETS, data_handler, NULL);
}

void loop() {
  if (TGRX.available()) {
    THINKGEAR_parseByte(&TGAMParser, TGRX.read());
  }

#ifdef MQTT
  if (!mqtt_client.connected()) {
    mqtt_reconnect();
  }

  mqtt_client.loop();
  yield();
#endif // MQTT
}
