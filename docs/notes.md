# Rapport Technique - Smart Multi-Node IoT System

## Choix techniques
- Simulation Wokwi avec 1 ESP32 (nœud capteur)
- Serveur HTTP embarqué avec WebServer.h
- Dashboard via SPIFFS avec HTML/CSS/JS minimalistes
- Communication simulée via Serial.print()
- Buffer circulaire pour stockage des lectures

## Logique de communication simulée
- Le nœud capteur génère des requêtes HTTP POST simulées dans le moniteur série
- Format JSON avec température/humidité/timestamp
- Le serveur traite ces données comme s'il les recevait via HTTP

## Tests effectués
- [x] Lecture DHT22 simulée
- [x] Affichage OLED
- [x] Simulation transmission HTTP
- [x] Dashboard web embarqué
- [x] Calcul de statistiques