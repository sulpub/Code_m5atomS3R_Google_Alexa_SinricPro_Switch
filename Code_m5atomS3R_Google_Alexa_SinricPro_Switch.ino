/*
 * Simple example for how to use SinricPro Switch device:
 * wait command for screen animation


Liste de librairie : 
  - Adafruit NeoPixel at version 1.11.0 : Adafruit_NeoPixel by Adafruit

PINOUT ATOM M5STACK
                      WIFI
                ______________
  alim+    3V3 |              |
  btn       G5 |              |G39
  GND_soft  G6 |    BOTTOM    |G38
            G7 |              |5V
            G8 |______________|GND
                 GND 5V G2 G1

  BUTTON : G41
 */

//#include <M5StickCPlus.h>
#include <M5AtomS3.h>
#include <SPIFFS.h>
#include <FS.h>
#include <JPEGDecoder.h>

#include <WiFi.h>

#include "SinricPro.h"
#include "SinricProSwitch.h"
#include "esp_task_wdt.h"


#define WIFI_SSID "wifiSSID"
#define WIFI_PASS "wifiPASSWORD"
#define APP_KEY "SinricPro application key"
#define APP_SECRET "SinrincPro application secret"

#define SWITCH_ID_1 "SinrincPro switch ID"
#define RELAYPIN_1 5
#define GND_SENSOR 6
#define BUTTON_SCREEN 41
#define BAUD_RATE 115200  // Change baudrate to your need
#define WAIT_BLINK 1000
#define WAIT_LOGO 3000
#define WAIT_BUTTON 500

unsigned long previousMillis = 0;
unsigned long previousMillisLogo = 0;
unsigned long previousMillisButton = 0;

const long interval = 5000;  // Vérifie toutes les 5 secondes
int change = 0;
unsigned long currentMillis = 0;
int buttonLevel = 0;
int i = 0;
bool bool_spiffs_error = false;
int buttonState = 0;  // variable for reading the pushbutton status
int activeWifiLoop = 1;
int buttonAction = 0;



void buttonalarm(void);



void listSPIFFS(const char* dirname = "/") {
  Serial.println("📂 Liste des fichiers SPIFFS :");

  File root = SPIFFS.open(dirname);
  if (!root || !root.isDirectory()) {
    Serial.println("❌ Impossible d’ouvrir SPIFFS ou ce n’est pas un dossier !");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      Serial.print("📄 ");
      Serial.print(file.name());
      Serial.print("\t");
      Serial.print(file.size());
      Serial.println(" octets");
    }
    file = root.openNextFile();
    Serial.flush();
  }
}



// Fonction pour afficher une image JPG avec couleurs corrigées
void drawJpeg(const char* filename, int xpos, int ypos) {
  Serial.printf("🖼️ Chargement de : %s\n", filename);

  // Vérifie que le fichier existe
  if (!SPIFFS.exists(filename)) {
    Serial.println("❌ Fichier JPG introuvable !");
    return;
  }

  // Décode le fichier JPEG
  if (JpegDec.decodeFsFile(filename) != 1) {
    Serial.println("❌ Erreur de décodage JPEG !");
    return;
  }

  // Récupère les dimensions d’un bloc (MCU)
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width + xpos;
  uint32_t max_y = JpegDec.height + ypos;

  // Buffer pointeur vers l’image
  uint16_t* pImg;

  while (JpegDec.read()) {
    pImg = JpegDec.pImage;
    uint32_t mcu_x = JpegDec.MCUx * mcu_w + xpos;
    uint32_t mcu_y = JpegDec.MCUy * mcu_h + ypos;

    if ((mcu_x + mcu_w) <= max_x && (mcu_y + mcu_h) <= max_y) {
      // 🌀 Correction des couleurs : swap bytes pour pushImage()
      for (int i = 0; i < mcu_w * mcu_h; i++) {
        uint16_t color = pImg[i];
        pImg[i] = (color >> 8) | (color << 8);
      }

      M5.Lcd.pushImage(mcu_x, mcu_y, mcu_w, mcu_h, pImg);
    } else {
      // Affichage manuel si bloc tronqué
      for (int y = 0; y < mcu_h; y++) {
        for (int x = 0; x < mcu_w; x++) {
          int px = mcu_x + x;
          int py = mcu_y + y;
          if (px < max_x && py < max_y) {
            uint16_t color = *pImg++;
            color = (color >> 8) | (color << 8);  // correction couleur
            M5.Lcd.drawPixel(px, py, color);
          } else {
            pImg++;
          }
        }
      }
    }
  }

  Serial.println("✅ Image affichée");
}




bool onPowerState1(const String& deviceId, bool& state) {
  Serial.printf("Device 1 turned %s", state ? "on" : "off");
  //digitalWrite(RELAYPIN_1, state ? HIGH : LOW);
  if (state) change = 1;
  return true;  // request handled properly
}



// setup function for WiFi connection
void setupWiFi(void) {

  esp_task_wdt_reset();  // 👈 important ! réinitialise le compteur watchdog

  Serial.println("🔌 Wi-Fi déconnecté, reconnexion en cours...");
  Serial.flush();

  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  delay(100);

  M5.Lcd.fillScreen(BLACK);
  drawJpeg("/nowifi1.jpg", 0, 0);  // adapte x, y selon besoin

  for (i = 0; i < 200; i++) {
    buttonalarm();
    if (buttonAction == 1) {
      buttonAction = 0;
      delay(1000);
      M5.Lcd.fillScreen(BLACK);
      drawJpeg("/nowifi1.jpg", 0, 0);  // adapte x, y selon besoin
    }
    delay(10);
  }

  esp_task_wdt_reset();  // 👈 important ! réinitialise le compteur watchdog

  while (WiFi.status() != WL_CONNECTED) {
    i++;
    if (i % 50 == 0) {
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASS);
      delay(100);
      Serial.println();
      esp_task_wdt_reset();  // 👈 important ! réinitialise le compteur watchdog
    }
    Serial.printf(".");

    buttonalarm();
    if (buttonAction == 1) {
      buttonAction = 0;
      delay(1000);
      M5.Lcd.fillScreen(BLACK);
      drawJpeg("/nowifi1.jpg", 0, 0);  // adapte x, y selon besoin
    }
    delay(100);
  }

  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
  M5.Lcd.fillScreen(BLACK);
  drawJpeg("/wifiok.jpg", 0, 0);  // adapte x, y selon besoin
  delay(2000);
}



// setup function for SinricPro
void setupSinricPro() {
  // add devices and callbacks to SinricPro
  pinMode(RELAYPIN_1, INPUT);
  digitalWrite(RELAYPIN_1, LOW);

  SinricProSwitch& mySwitch1 = SinricPro[SWITCH_ID_1];
  mySwitch1.onPowerState(onPowerState1);

  // setup SinricPro
  SinricPro.onConnected([]() {
    Serial.printf("Connected to SinricPro\r\n");
    M5.Lcd.fillScreen(BLACK);
    drawJpeg("/logo1.jpg", 0, 0);  // adapte x, y selon besoin
    delay(2000);
  });
  SinricPro.onDisconnected([]() {
    Serial.printf("Disconnected from SinricPro\r\n");
    M5.Lcd.fillScreen(BLACK);
    drawJpeg("/internetok.jpg", 0, 0);  // adapte x, y selon besoin
    delay(2000);
  });

  SinricPro.begin(APP_KEY, APP_SECRET);
}



// main setup function
void setup() {

  M5.begin();

  Serial.begin(BAUD_RATE);
  Serial.printf("\n🚀 DEMARRRAGE ALERTE\r\n\r\n");

  // Initialisation du WDT pour la tâche courante (loopTask)
  esp_task_wdt_init(10, true);  // 10s timeout, reset si non nourri
  esp_task_wdt_add(NULL);       // NULL = tâche courante (loopTask)

  Serial.println("✅ Watchdog activé (5s)");

  //power sensor
  pinMode(GND_SENSOR, OUTPUT);
  digitalWrite(GND_SENSOR, LOW);

  //button screen
  pinMode(BUTTON_SCREEN, INPUT_PULLUP);
  digitalWrite(BUTTON_SCREEN, HIGH);

  if (!SPIFFS.begin(true)) {
    Serial.println("❌ SPIFFS mount failed!");
    return;
  }
  Serial.println("✅ SPIFFS mount successful");

  // Affiche la liste des fichiers
  listSPIFFS();

  // Écran
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);

  // Affiche l'image JPG
  drawJpeg("/logo1.jpg", 0, 0);
  delay(3000);

  Serial.println("🔌 Wi-Fi déconnecté, premiére connexion en cours...");
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  delay(100);

  setupWiFi();

  setupSinricPro();
}



void loop() {

  esp_task_wdt_reset();  // 👈 important ! réinitialise le compteur watchdog

  SinricPro.handle();

  buttonalarm();

  // Vérifie toutes les X secondes si le Wi-Fi est toujours connecté
  if (WiFi.status() != WL_CONNECTED) {
    setupWiFi();
  }

  if (millis() >= previousMillis) {
    previousMillis = millis() + WAIT_BLINK;

    buttonLevel = digitalRead(RELAYPIN_1);
    Serial.print(buttonLevel);
    Serial.print(",");
    Serial.println(activeWifiLoop);

    if (change == 2) {
      pinMode(RELAYPIN_1, INPUT);
      digitalWrite(RELAYPIN_1, LOW);
      change = 3;
      //M5.Lcd.fillScreen(BLACK);
      //drawJpeg("/alarm1.jpg", 0, 0);  // adapte x, y selon besoin
    }

    if (change == 1) {
      pinMode(RELAYPIN_1, OUTPUT);
      digitalWrite(RELAYPIN_1, LOW);
      change = 2;
      M5.Lcd.fillScreen(BLACK);
      drawJpeg("/alarm1.jpg", 0, 0);  // adapte x, y selon besoin
      previousMillisLogo = millis() + WAIT_LOGO;
    }
  }

  if (millis() >= previousMillisLogo && (change == 3)) {
    change = 0;
    M5.Lcd.fillScreen(BLACK);
  }
}


void buttonalarm(void) {
  buttonState = digitalRead(BUTTON_SCREEN);
  if (buttonState == 0) {
    buttonAction = 1;
    if (millis() >= previousMillisButton) {
      previousMillisButton = millis() + WAIT_BUTTON;
      change = 1;
      Serial.println("\n🔳 Bouton appuyé");
      M5.Lcd.fillScreen(BLACK);
      drawJpeg("/alarm1.jpg", 0, 0);  // adapte x, y selon besoin
      pinMode(RELAYPIN_1, OUTPUT);
      digitalWrite(RELAYPIN_1, LOW);
      delay(1000);
      pinMode(RELAYPIN_1, INPUT);
      digitalWrite(RELAYPIN_1, LOW);
    }
  }
}
