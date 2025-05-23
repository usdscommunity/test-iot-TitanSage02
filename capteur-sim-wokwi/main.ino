#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DHT_PIN 15
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3D
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Buffers statiques
static char tempBuffer[16];
static char humBuffer[16];
static char jsonBuffer[128];
static char contentLengthBuffer[32];

// Gestion non-bloquante des timings
static unsigned long lastSensorRead = 0;
static unsigned long startupScreenTime = 0;
static bool startupScreenActive = true;

// Constantes pour les messages
static const char* const STARTUP_LINE1 PROGMEM = "IoT Sensor Node";
static const char* const STARTUP_LINE2 PROGMEM = "DHT22 + OLED";
static const char* const STARTUP_LINE3 PROGMEM = "Simulation Wokwi";
static const char* const SENSOR_TITLE PROGMEM = "Sensor Node";
static const char* const DHT_ERROR_MSG PROGMEM = "Failed to read DHT";
static const char* const OLED_ERROR_MSG PROGMEM = "OLED error";
static const char* const OLED_INIT_MSG PROGMEM = "OLED initialized";

void setup() {
  Serial.begin(115200);
  dht.begin();
  Wire.begin(21, 22); // SDA=21, SCL=22 pour l'ESP32

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(FPSTR(OLED_ERROR_MSG));
    while(1);
  }

  // définir la couleur du texte en blanc
  display.setTextColor(SSD1306_WHITE);
  
  displayStartupScreen();
  startupScreenTime = millis();
  Serial.println(FPSTR(OLED_INIT_MSG));
}

void loop() {
  // Gestion non-bloquante de l'écran de démarrage
  if (startupScreenActive && (millis() - startupScreenTime >= 2000)) {
    startupScreenActive = false;
    display.clearDisplay();
    display.display();
  }

  // Lecture des capteurs toutes les 2 secondes
  if (!startupScreenActive && (millis() - lastSensorRead >= 2000)) {
    readAndSendData();
    lastSensorRead = millis();
  }
}

void readAndSendData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    Serial.println(FPSTR(DHT_ERROR_MSG));
    return;
  }

  updateDisplay(t, h);
  simulateHttpPost(t, h);
}

void simulateHttpPost(float temp, float hum) {
  // Préparation des buffers statiques
  dtostrf(temp, 6, 1, tempBuffer);
  dtostrf(hum, 6, 1, humBuffer);
  
  // Construction du JSON dans un buffer statique
  snprintf(jsonBuffer, sizeof(jsonBuffer), 
           "{\"temp\":%s,\"hum\":%s,\"timestamp\":%lu}", 
           tempBuffer, humBuffer, millis());
  
  // Calcul de la longueur dans un buffer statique
  snprintf(contentLengthBuffer, sizeof(contentLengthBuffer), 
           "Content-Length: %d", strlen(jsonBuffer));
  
  // Envoi des données
  Serial.println(F("\nPOST /api/data HTTP/1.1"));
  Serial.println(F("Host: 192.168.4.1"));
  Serial.println(F("Content-Type: application/json"));
  Serial.println(contentLengthBuffer);
  Serial.println();
  Serial.println(jsonBuffer);
  Serial.println(F("HTTP/1.1 200 OK\n"));
}

void updateDisplay(float temp, float hum) {
  display.clearDisplay();
  
  // Titre
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print(FPSTR(SENSOR_TITLE));
  
  // Température
  dtostrf(temp, 4, 1, tempBuffer);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,20);
  display.print(F("T : "));
  display.print(tempBuffer);
  display.print(F("C"));
  
  // Humidité - utilisation de buffer statique
  dtostrf(hum, 4, 1, humBuffer);
  display.setCursor(0,40);
  display.print(F("H : "));
  display.print(humBuffer);
  display.print(F("%"));
  
  display.display();
}

void displayStartupScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(FPSTR(STARTUP_LINE1));
  display.println(FPSTR(STARTUP_LINE2));
  display.println(FPSTR(STARTUP_LINE3));
  display.display();
}