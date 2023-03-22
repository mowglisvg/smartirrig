// Include Libraries
#include "Arduino.h"
#include "DHT.h"
#include "SoilMoistureSensor.h"
#include "Pump.h"
#include <WiFi.h>
#include <FirebaseESP32.h>

// Paramètres WiFi
const char* ssid = "NomDeVotreWiFi";
const char* password = "MotDePasseDeVotreWiFi";

// Paramètres Firebase
#define FIREBASE_HOST "nom-de-votre-projet.firebaseio.com"
#define FIREBASE_AUTH "clé-d'authentification-de-votre-projet"

// Pin Definitions
#define DHT_PIN 14
#define RELAYMODULE_PIN_SIGNAL	2
#define SOIL_MOISTURE_SENSOR_PIN 12
#define WATERPUMP_PIN_COIL1	1
#define WATER_LEVEL_SENSOR_PIN 13

// Déclaration des variables pour les données des capteurs
float niveauEau;
float humiditesol;
float temperature;
float humidite;

/ Déclaration de la variable pour le contrôle de la pompe
bool pump = false;

// Déclaration de l'objet Firebase
FirebaseData firebaseData;



void setup() {
  // Initialisation de la communication série
  Serial.begin(9600);

  // Connexion au réseau WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected.");

  // Initialisation de Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial.println("Firebase initialized.");

  // Configuration des broches en entrée/sortie
  pinMode(WATER_LEVEL_SENSOR_PIN, INPUT);
  pinMode(SOIL_MOISTURE_SENSOR_PIN, INPUT);
  pinMode(DHT_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
}

void loop() {
  // Lecture des données des capteurs
  niveauEau = analogRead(WATER_LEVEL_SENSOR_PIN);
  humiditesol = analogRead(SOIL_MOISTURE_SENSOR_PIN);
  temperature = dht.readTemperature();
  humidite = dht.readHumidity();

  
  // Envoi des données des capteurs à Firebase
  Firebase.pushFloat("/niveauEau", niveauEau);
  Firebase.pushFloat("/humiditesol", humiditesol);
  Firebase.pushFloat("/temperature", temperature);
  Firebase.pushFloat("/humidite", humidite);

   // Lecture de la valeur de la variable PUMP dans Firebase
  Firebase.getBool("/PUMP", &pump);

//**************************************************************Activation de la pompe sans planification*************************************************************//
if (pump) {
  digitalWrite(WATERPUMP_PIN_COIL1, HIGH); // activer la pompe
} else {
  digitalWrite(WATERPUMP_PIN_COIL1, LOW); // désactiver la pompe
}



//**************************************************************Activation de la pompe avec  planification*************************************************************//



String joursSelectionnes;

Firebase.getString("/planification/TimeOn/joursSelectionnes", &joursSelectionnes);
bool joursSelectionnesBool[7] = {false}; // Initialisation à false pour tous les jours de la semaine

// Analyse de la chaîne de caractères joursSelectionnes
for (int i = 0; i < joursSelectionnes.length(); i++) {
  char jour = joursSelectionnes.charAt(i);
  switch (jour) {
    case 'Monday':
      joursSelectionnes[1] = true;
      break;
    case 'Tuesday':
      joursSelectionnes[2] = true;
      break;
    case 'Wednesday':
      joursSelectionnes[3] = true;
      break;
    case 'Thursday':
      joursSelectionnes[4] = true;
      break;
    case 'Friday':
      joursSelectionnes[5] = true;
      break;
    case 'Saturday':
      joursSelectionnes[6] = true;
      break;
    case 'Sunday':
      joursSelectionnes[0] = true;
      break;
  }
}
// Lecture de la date et de l'heure actuelles
int currentDayOfWeek = weekday(); // Numéro du jour de la semaine (1 = dimanche, 2 = lundi, etc.)
int currentHour = hour(); // Heure actuelle
int currentMinute = minute(); // Minute actuelle

// Récupération de l'heure de début et de fin de la planification
String heureDebut;
String heureFin;
  Firebase.getString("/planification/TimeOn/heurePlanification", &heureDebut);
    Firebase.getString("/planification/TimeOff/heurePlanification", &heureFin);

// Conversion des heures de début et de fin en entiers
int heureDebutInt = heureDebut.toInt();
int heureFinInt = heureFin.toInt();

// Activation/désactivation de la pompe en fonction de la date et de l'heure actuelles
if (pump) {
  if (joursSelectionnesBool[currentDayOfWeek - 1] && currentHour >= heureDebutInt && currentHour < heureFinInt) {
    digitalWrite(WATERPUMP_PIN_COIL1, HIGH); // activer la pompe
  } else {
    digitalWrite(WATERPUMP_PIN_COIL1, LOW); // désactiver la pompe
  }
} else {
  digitalWrite(WATERPUMP_PIN_COIL1, LOW); // désactiver la pompe
}

//**************************************************************Activation de la pompe sans planification*************************************************************//


  // Attente de 3 minutes avant la prochaine lecture des capteurs
  delay(3 * 60 * 1000);
 
}






