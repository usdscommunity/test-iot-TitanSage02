# 📡 IoT Sensor Node Simulation

Simulation Wokwi d'un nœud capteur IoT utilisant un ESP32, un capteur DHT22 et un écran OLED.

![Version](https://img.shields.io/badge/version-1.0-blue)
![Type](https://img.shields.io/badge/type-simulation-orange)

## 📋 Description

Ce module constitue le nœud capteur du système IoT complet. Il est responsable de :
- La collecte des données de température et d'humidité via un capteur DHT22
- L'affichage local des mesures sur un écran OLED
- La transmission des données au serveur central via des requêtes HTTP simulées

## 🔌 Composants matériels

| Composant | Description | Connexions |
|-----------|-------------|------------|
| ESP32 | Microcontrôleur | - |
| DHT22 | Capteur de température et humidité | GPIO 15, VCC, GND |
| Écran OLED SSD1306 | Afficheur 128x64 | SDA (GPIO 21), SCL (GPIO 22), VCC, GND |

## 📚 Bibliothèques requises

- `Arduino.h` - Fonctionnalités de base d'Arduino
- `DHT.h` - Interface avec le capteur DHT22
- `Wire.h` - Communication I2C
- `Adafruit_GFX.h` - Bibliothèque graphique
- `Adafruit_SSD1306.h` - Pilote pour l'écran OLED

## ⚙️ Configuration

```cpp
// Configuration du capteur DHT22
#define DHT_PIN 15
#define DHT_TYPE DHT22

// Configuration de l'écran OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3D
```

## 🔄 Fonctionnement

1. **Initialisation** (`setup()`)
   - Configuration du port série (115200 bauds)
   - Initialisation du capteur DHT
   - Initialisation de l'afficheur OLED
   - Affichage de l'écran de démarrage pendant 2 secondes

2. **Boucle principale** (`loop()`)
   - Lecture des valeurs du capteur toutes les 2 secondes (2000ms)
   - Mise à jour de l'affichage OLED
   - Simulation d'envoi HTTP au serveur central

3. **Fonctions principales**
   - `readAndSendData()` : Lit les données et déclenche l'affichage + envoi
   - `simulateHttpPost()` : Simule une requête HTTP POST avec les données au format JSON
   - `updateDisplay()` : Met à jour l'affichage OLED avec les valeurs actuelles
   - `displayStartupScreen()` : Affiche l'écran de démarrage initial

## 📊 Format de données

Les données sont envoyées au format JSON avec la structure suivante :
```json
{
  "temp": 25.5,
  "hum": 48.3,
  "timestamp": 123456789
}
```

## 🔍 Comportement de l'OLED

L'écran OLED affiche :
- Écran de démarrage pendant 2 secondes avec :
  - "IoT Sensor Node"
  - "DHT22 + OLED"
  - "Simulation Wokwi"
- Écran principal avec :
  - En-tête "Sensor Node"
  - Température actuelle en °C (grand format)
  - Humidité actuelle en % (grand format)

## 📡 Simulation HTTP

Le code simule l'envoi de requêtes HTTP POST vers un serveur central à l'adresse `192.168.4.1`. Ces requêtes sont visibles dans le moniteur série mais ne sont pas réellement transmises par réseau.

Exemple de sortie série :
```
POST /api/data HTTP/1.1
Host: 192.168.4.1
Content-Type: application/json
Content-Length: 42

{"temp":25.5,"hum":48.3,"timestamp":123456789}
HTTP/1.1 200 OK
```

## 🚀 Utilisation

1. Ouvrez le projet dans Wokwi
2. Lancez la simulation
3. Observez l'écran OLED qui affiche d'abord l'écran de démarrage, puis les valeurs actuelles
4. Consultez le moniteur série pour voir les requêtes HTTP simulées

## 🔧 Dépannage

- **Écran OLED non fonctionnel** : Vérifiez l'adresse I2C (0x3D par défaut)
- **Données DHT incorrectes** : Vérifiez la connexion du capteur au GPIO 15
- **Aucune donnée envoyée** : Le capteur envoie des données toutes les 2 secondes uniquement

## 📝 Notes techniques

- Utilisation de buffers statiques pour optimiser la mémoire
- Messages d'erreur stockés en PROGMEM pour économiser la RAM
- Gestion non-bloquante des timings pour l'écran de démarrage et les lectures de capteurs
- Intervalle d'échantillonnage : 2 secondes (modifiable dans la variable `lastSensorRead`)

---

*TitanSage02 - Espérance AYIWAHOUN*