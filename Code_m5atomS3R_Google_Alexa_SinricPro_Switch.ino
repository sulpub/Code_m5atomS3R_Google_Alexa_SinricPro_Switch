
/*
 * Simple example for how to use SinricPro Switch device:
 * wait command for screen animation
 */
 
#include "M5AtomS3.h"

#define ESP32
#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#define DEBUG_ESP_PORT Serial
#define NODEBUG_WEBSOCKETS
#define NDEBUG
#endif

#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
#include <WiFi.h>
#endif

#include "SinricPro.h"
#include "SinricProSwitch.h"

#define WIFI_SSID "wifiSSID"
#define WIFI_PASS "wifiPASSWORD"
#define APP_KEY "SinricPro application key"
#define APP_SECRET "SinrincPro application secret"

#define SWITCH_ID_1 "SinrincPro switch ID"
#define RELAYPIN_1 39

#define BAUD_RATE 115200  // Change baudrate to your need

#define WAIT_BLINK 1000

int change = 0;
unsigned long previousMillis = 0;

void draw_function_l(LovyanGFX* gfx) {
  int x = 64;      //rand() % gfx->width();
  int y = 64;      //rand() % gfx->height();
  int r = 40;      //(gfx->width() >> 4) + 2;
  uint16_t c = 0x0000;  //rand();
  gfx->fillRect(x - r, y - r, r * 2, r * 2, c);
}

void draw_function_h(LovyanGFX* gfx) {
  int x = 64;      //rand() % gfx->width();
  int y = 64;      //rand() % gfx->height();
  int r = 40;      //(gfx->width() >> 4) + 2;
  uint16_t c = 0xFFFFFF;  //rand();
  gfx->fillRect(x - r, y - r, r * 2, r * 2, c);
}


bool onPowerState1(const String& deviceId, bool& state) {
  Serial.printf("Device 1 turned %s", state ? "on" : "off");
  digitalWrite(RELAYPIN_1, state ? HIGH : LOW);
  if (state) change = 1;
  return true;  // request handled properly
}

// setup function for WiFi connection
void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Connecting");

#if defined(ESP8266)
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setAutoReconnect(true);
#elif defined(ESP32)
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
#endif

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }

  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

// setup function for SinricPro
void setupSinricPro() {
  // add devices and callbacks to SinricPro
  pinMode(RELAYPIN_1, OUTPUT);

  SinricProSwitch& mySwitch1 = SinricPro[SWITCH_ID_1];
  mySwitch1.onPowerState(onPowerState1);


  // setup SinricPro
  SinricPro.onConnected([]() {
    Serial.printf("Connected to SinricPro\r\n");
  });
  SinricPro.onDisconnected([]() {
    Serial.printf("Disconnected from SinricPro\r\n");
  });

  SinricPro.begin(APP_KEY, APP_SECRET);
}

// main setup function
void setup() {
  auto cfg = M5.config();
  AtomS3.begin(cfg);

  int textsize = AtomS3.Display.height() / 60;
  if (textsize == 0) {
    textsize = 1;
  }
  AtomS3.Display.setTextSize(textsize);

  Serial.begin(BAUD_RATE);
  Serial.printf("\r\n\r\n");
  setupWiFi();
  setupSinricPro();
}

void loop() {
  SinricPro.handle();

  if (millis() >= previousMillis) {
    previousMillis = millis() + WAIT_BLINK;
    if (change == 2) {
      change = 0;
      int x = 64;      //rand() % AtomS3.Display.width();
      int y = 64;      //rand() % AtomS3.Display.height();
      int r = 40;      //(AtomS3.Display.width() >> 4) + 2;
      uint16_t c = 0;  //rand();
      AtomS3.Display.fillCircle(x, y, r, c);
      //draw_function_l(&AtomS3.Display);
    }

    if (change == 1) {
      change = 2;
      int x = 64;      //rand() % AtomS3.Display.width();
      int y = 64;      //rand() % AtomS3.Display.height();
      int r = 40;      //(AtomS3.Display.width() >> 4) + 2;
      uint16_t c = 0xFFFFFF;  //rand();
      AtomS3.Display.fillCircle(x, y, r, c);
      //draw_function_h(&AtomS3.Display);
      
    }
  }
}