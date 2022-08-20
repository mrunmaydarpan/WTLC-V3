// #include <LittleFS.h>
#include <version.h>
#include <setting.h>

// #define WM_ASYNC
#if debugData
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <ESP8266HTTPUpdateServer.h>
#include <EspHtmlTemplateProcessor.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>
#if HA_INIT
#include <ArduinoHA.h>
#else
#endif
#include <JC_Button.h>
#include <EEPROM.h>
#if SENSOR_1
#include <Ultrasonic.h>
#endif
#if OLED
#include <Adafruit_SSD1306.h>
#else
#include <LiquidCrystal_I2C.h>
#endif
#include <SoftwareSerial.h>
#include <Timer.h>
#include <Smoothed.h>
#include <pins.h>
#ifdef WM_SET
#include <AsyncWiFiManager.h>
#endif

#if SENSOR_1
Ultrasonic Sonar(TriggerPin, EchoPin);
#elif SENSOR_2
SoftwareSerial sensorSerial(sensorRX, sensorTX);
#elif SENSOR_3
#endif
#if OLED
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#else
LiquidCrystal_I2C lcd(0x27, 16, 2);
#endif
Button button(PB);
Button modeButton(_Mode);
Timer t;
Smoothed<uint8_t> mySensor;
DNSServer dnsServer;
WiFiClient client;
ESP8266WebServer server(80);
EspHtmlTemplateProcessor TP(&server);
ESP8266HTTPUpdateServer httpUpdater;

#if HA_INIT
byte mac[WL_MAC_ADDR_LENGTH];
#endif

#ifdef WM_SET
AsyncWiFiManager wm;
#if HA_INIT
char BROKER_ADDR[40];
AsyncWiFiManagerParameter custom_mqtt_server("server", "mqtt server", BROKER_ADDR, 40);
#endif
#endif

#if SENSOR_1
#define SENSOR_DISP 1
#elif SENSOR_2
#define SENSOR_DISP 2
#elif SENSOR_3
#define SENSOR_DISP 3
#endif
