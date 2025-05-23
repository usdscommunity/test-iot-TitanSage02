#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

// Configuration WiFi en mode AP
static const char* const ap_ssid PROGMEM = "IoT-Central-Node";
static const char* const ap_password PROGMEM = "iot12345";

WebServer server(80);

// Structure pour stocker les données
struct SensorData {
  float temperature;
  float humidity;
  unsigned long timestamp;
  bool valid = false;
};

// Buffer circulaire
static const int MAX_DATA = 50;
SensorData sensorData[MAX_DATA];
int dataIndex = 0;
int dataCount = 0;

// Variables pour les stats
float minTemp = 0, maxTemp = 0, avgTemp = 0;
float minHum = 0, maxHum = 0, avgHum = 0;
unsigned long lastUpdate = 0;

// Buffers statiques pour éviter les allocations dynamiques
static char jsonResponseBuffer[1024];
static char errorBuffer[128];
static char logBuffer[256];
static char filenameBuffer[64];
static char contentTypeBuffer[32];

// Messages constants en PROGMEM
static const char* const SPIFFS_ERROR_MSG PROGMEM = "ERREUR: Mount SPIFFS failed";
static const char* const SPIFFS_OK_MSG PROGMEM = "SPIFFS mounted successfully";
static const char* const SERVER_START_MSG PROGMEM = "HTTP server started";
static const char* const ENDPOINTS_MSG PROGMEM = "Endpoints:";
static const char* const ENDPOINT1_MSG PROGMEM = "  GET /           - Dashboard web";
static const char* const ENDPOINT2_MSG PROGMEM = "  GET /api/data   - Obtenir données JSON";
static const char* const ENDPOINT3_MSG PROGMEM = "  POST /api/data  - Envoyer données capteur";
static const char* const AP_IP_MSG PROGMEM = "AP IP address: ";
static const char* const FILES_LIST_MSG PROGMEM = "\nFichiers SPIFFS:";
static const char* const FILE_NOT_FOUND_MSG PROGMEM = "File not found: ";
static const char* const INCOMPLETE_FILE_MSG PROGMEM = "Sent incomplete file";
static const char* const STATUS_ACTIVE_MSG PROGMEM = "ACTIF";
static const char* const STATUS_INACTIVE_MSG PROGMEM = "INACTIF";
static const char* const STATUS_MSG PROGMEM = "Status: ";
static const char* const NEW_DATA_MSG PROGMEM = "Nouvelle donnee : ";

// Réponses JSON statiques
static const char* const JSON_ERROR_INVALID PROGMEM = "{\"error\":\"Invalid JSON\"}";
static const char* const JSON_ERROR_MISSING PROGMEM = "{\"error\":\"Missing data\"}";
static const char* const JSON_ERROR_NO_DATA PROGMEM = "{\"error\":\"No data\"}";
static const char* const JSON_SUCCESS PROGMEM = "{\"status\":\"success\"}";
static const char* const HTML_FALLBACK PROGMEM = "<h1>IoT Central Node</h1><p>Dashboard non disponible</p>";
static const char* const NOT_FOUND_MSG PROGMEM = "404 Not Found\n\n";

// Gestion non-bloquante des vérifications de statut
static unsigned long lastStatusCheck = 0;
static const unsigned long STATUS_CHECK_INTERVAL = 5000;

void setup() {
  Serial.begin(115200);
  
  // Initialisation SPIFFS
  if(!SPIFFS.begin(true)) {
    Serial.println(FPSTR(SPIFFS_ERROR_MSG));
  } else {
    Serial.println(FPSTR(SPIFFS_OK_MSG));
    listSPIFFSFiles();
  }

  // Configuration du point d'accès
  WiFi.softAP(FPSTR(ap_ssid), FPSTR(ap_password));
  IPAddress myIP = WiFi.softAPIP();
  Serial.print(FPSTR(AP_IP_MSG));
  Serial.println(myIP);

  // Configuration des fichiers statiques - CORRECTION ICI
  server.on("/", HTTP_GET, []() {
    if(!handleFileRead("/index.html")) {
      server.send(200, "text/html", FPSTR(HTML_FALLBACK));
    }
  });
  
  server.on("/index.html", HTTP_GET, []() {
    handleFileRead("/index.html");
  });
  
  server.on("/script.js", HTTP_GET, []() {
    handleFileRead("/script.js");
  });

  // Endpoints API
  server.on("/api/data", HTTP_GET, handleGetData);
  server.on("/api/data", HTTP_POST, handlePostData);
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println(FPSTR(SERVER_START_MSG));
  Serial.println(FPSTR(ENDPOINTS_MSG));
  Serial.println(FPSTR(ENDPOINT1_MSG));
  Serial.println(FPSTR(ENDPOINT2_MSG));
  Serial.println(FPSTR(ENDPOINT3_MSG));
}

void loop() {
  server.handleClient();
  checkSensorStatus();
}

void handleGetData() {
  // Utilisation d'un buffer statique pour la réponse JSON
  StaticJsonDocument<768> doc;
  
  // Dernières données
  if(dataCount > 0) {
    int lastIndex = (dataIndex - 1 + MAX_DATA) % MAX_DATA;
    doc["current"]["temperature"] = sensorData[lastIndex].temperature;
    doc["current"]["humidity"] = sensorData[lastIndex].humidity;
    doc["current"]["timestamp"] = sensorData[lastIndex].timestamp;
  }
  
  // Statistiques
  doc["stats"]["temperature"]["min"] = minTemp;
  doc["stats"]["temperature"]["max"] = maxTemp;
  doc["stats"]["temperature"]["avg"] = avgTemp;
  doc["stats"]["humidity"]["min"] = minHum;
  doc["stats"]["humidity"]["max"] = maxHum;
  doc["stats"]["humidity"]["avg"] = avgHum;
  
  // Status
  doc["status"]["active"] = isSensorActive();
  doc["status"]["last_update"] = lastUpdate;
  doc["status"]["count"] = dataCount;

  // Sérialisation dans un buffer statique
  size_t jsonLength = serializeJson(doc, jsonResponseBuffer, sizeof(jsonResponseBuffer));
  if (jsonLength > 0) {
    server.send(200, "application/json", jsonResponseBuffer);
  } else {
    server.send(500, "application/json", FPSTR(JSON_ERROR_INVALID));
  }
}

void handlePostData() {
  if(server.hasArg("plain")) {
    const String& body = server.arg("plain");
    
    StaticJsonDocument<384> doc;
    DeserializationError err = deserializeJson(doc, body);
    
    if(err) {
      server.send(400, "application/json", FPSTR(JSON_ERROR_INVALID));
      return;
    }

    // Validation des données
    if(!doc.containsKey("temperature") || !doc.containsKey("humidity")) {
      server.send(400, "application/json", FPSTR(JSON_ERROR_MISSING));
      return;
    }

    // Stockage des données
    storeSensorData(
      doc["temperature"].as<float>(),
      doc["humidity"].as<float>(),
      doc["timestamp"] | millis()
    );
    
    server.send(200, "application/json", FPSTR(JSON_SUCCESS));
  } else {
    server.send(400, "application/json", FPSTR(JSON_ERROR_NO_DATA));
  }
}

void storeSensorData(float temp, float hum, unsigned long ts) {
  sensorData[dataIndex] = {temp, hum, ts, true};
  dataIndex = (dataIndex + 1) % MAX_DATA;
  if(dataCount < MAX_DATA) dataCount++;
  
  lastUpdate = millis();
  calculateStats();
  
  // Buffer statique pour le logging
  snprintf(logBuffer, sizeof(logBuffer), "%s%.1f°C, %.1f%%", 
           FPSTR(NEW_DATA_MSG), temp, hum);

  Serial.println(logBuffer);
}

void calculateStats() {
  if(dataCount == 0) return;
  
  float tempSum = 0, humSum = 0;
  minTemp = maxTemp = sensorData[0].temperature;
  minHum = maxHum = sensorData[0].humidity;
  
  int validCount = 0;
  for(int i = 0; i < dataCount; i++) {
    if(!sensorData[i].valid) continue;
    
    float temp = sensorData[i].temperature;
    float hum = sensorData[i].humidity;
    
    tempSum += temp;
    humSum += hum;
    
    if(temp < minTemp) minTemp = temp;
    if(temp > maxTemp) maxTemp = temp;
    if(hum < minHum) minHum = hum;
    if(hum > maxHum) maxHum = hum;
    
    validCount++;
  }
  
  if(validCount > 0) {
    avgTemp = tempSum / validCount;
    avgHum = humSum / validCount;
  }
}

bool isSensorActive() {
  return (millis() - lastUpdate) < 30000; // 30s timeout
}

void checkSensorStatus() {
  // Gestion non-bloquante avec intervalle constant
  if(millis() - lastStatusCheck >= STATUS_CHECK_INTERVAL) {
    Serial.print(FPSTR(STATUS_MSG));
    Serial.println(isSensorActive() ? FPSTR(STATUS_ACTIVE_MSG) : FPSTR(STATUS_INACTIVE_MSG));
    lastStatusCheck = millis();
  }
}

void handleNotFound() {
  server.send(404, "text/plain", FPSTR(NOT_FOUND_MSG));
}

bool handleFileRead(const char* path) {
  strncpy(filenameBuffer, path, sizeof(filenameBuffer) - 1); // copie dans un buffer statique
  filenameBuffer[sizeof(filenameBuffer) - 1] = '\0'; // marque la fin
  
  size_t len = strlen(filenameBuffer);
  if(len > 0 && filenameBuffer[len - 1] == '/') {
    strncat(filenameBuffer, "index.html", sizeof(filenameBuffer) - len - 1);
  }
  
  if(!SPIFFS.exists(filenameBuffer)) {
    snprintf(logBuffer, sizeof(logBuffer), "%s%s", 
             FPSTR(FILE_NOT_FOUND_MSG), filenameBuffer);
    Serial.println(logBuffer);
    return false;
  }
  
  getContentType(filenameBuffer, contentTypeBuffer, sizeof(contentTypeBuffer));
  File file = SPIFFS.open(filenameBuffer, "r");
  
  if(server.streamFile(file, contentTypeBuffer) != file.size()) {
    Serial.println(FPSTR(INCOMPLETE_FILE_MSG));
  }
  
  file.close();
  return true;
}

void getContentType(const char* filename, char* buffer, size_t bufferSize) {
  const char* ext = strrchr(filename, '.');
  if(ext) {
    if(strcmp(ext, ".html") == 0) {
      strncpy(buffer, "text/html", bufferSize - 1);
    } else if(strcmp(ext, ".js") == 0) {
      strncpy(buffer, "application/javascript", bufferSize - 1);
    } else if(strcmp(ext, ".json") == 0) {
      strncpy(buffer, "application/json", bufferSize - 1);
    } else {
      strncpy(buffer, "text/plain", bufferSize - 1);
    }
  } else {
    strncpy(buffer, "text/plain", bufferSize - 1);
  }
  buffer[bufferSize - 1] = '\0';
}

void listSPIFFSFiles() {
  Serial.println(FPSTR(FILES_LIST_MSG));
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  
  while(file) {
    // Utilisation d'un buffer statique pour l'affichage des fichiers
    snprintf(logBuffer, sizeof(logBuffer), "  %s (%d octets)", 
             file.name(), (int)file.size());
    Serial.println(logBuffer);
    file = root.openNextFile();
  }
}