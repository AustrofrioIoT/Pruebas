
///WiFi Web Server
   
#include <WiFi.h>      //ESP32 Core WiFi Library    
#include <ESP8266WebServer.h>  //Local WebServer used to serve the configuration porta
#include <WebServer.h> //Local DNS Server used for redirecting all requests to the configuration portal 
#include <DNSServer.h> //Local WebServer used to serve the configuration portal 
#include <WiFiManager.h>   // WiFi Configuration Magic 
const int PIN_AP = 39;//Pin de Reset Wifi Configuration

//MQTT
#include "universal-mqtt.h"
#define PUBLISH_DELAY 300000 //Tiempos de publicacion cada 30min(30*1000*60)
unsigned long lastMillis = 0;

//Sensor de Temperatura
#include <OneWire.h>
#include <DallasTemperature.h>
const int oneWireBus = 17; 
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);



void setup() {
  // Start the Serial Monitor
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor
   pinMode(PIN_AP, INPUT);
  //declaração do objeto wifiManager
  WiFiManager wifiManager;
  
  //utilizando esse comando, as configurações são apagadas da memória
  //caso tiver salvo alguma rede para conectar automaticamente, ela é apagada.
  // wifiManager.resetSettings();
  
  //callback para quando entra em modo de configuração AP
  wifiManager.setAPCallback(configModeCallback); 
  //callback para quando se conecta em uma rede, ou seja, quando passa a trabalhar em modo estação
  wifiManager.setSaveConfigCallback(saveConfigCallback); 
  
  //cria uma rede de nome ESP_AP com senha 12345678
  wifiManager.autoConnect("ESP_AP", "12345678"); 
 
  // Start the DS18B20 sensor
  sensors.begin();
  pinMode(LED_BUILTIN, OUTPUT); //Led Indicador
  setupCloudIoT();

}

void loop() {
  WiFiManager wifiManager;
    mqttClient->loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!mqttClient->connected()) {
    connect();
  }

  // publish a message roughly every PUBLISH_DELAY ms.
  if (millis() - lastMillis > PUBLISH_DELAY) {
    lastMillis = millis();

     sensors.requestTemperatures(); 
     float temperatureC = sensors.getTempCByIndex(0);
   //  Serial.print(temperatureC);
    // Serial.println("ºC");

    // Prepare a JSON payload string

    String payload = String("{\"timestamp\":") + time(nullptr) +
                     String(",\"temperature\":") + temperatureC +
                     String(",\"itemId\":{\"ubicacion\":{\"piso1\":\"enfermeria\"}}")+
                     /*String(",\"humidity\":") + humidity +*/
                     String("}"); 
     Serial.println(payload);

    publishTelemetry(payload);
  }


  
 
  //Condicional Para Reset de AP
   if ( digitalRead(PIN_AP) == HIGH ) {
      Serial.println("resetar"); //tenta abrir o portal
      if(!wifiManager.startConfigPortal("ESP_AP", "12345678") ){
        Serial.println("Falha ao conectar");
        delay(2000);
        ESP.restart();
        delay(1000);
      }
      Serial.println("Equipo Conectado");
   }


}  

//Comunicacion Bidireccional   
void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  int ledonpos=payload.indexOf("ledon");
  if (ledonpos != -1) {
    // If yes, switch ON the ESP32 internal led
    Serial.println("Switch led on");
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    // If no, switch off the ESP32 internal led
    Serial.println("Switch led off");
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void configModeCallback (WiFiManager *myWiFiManager) {  
  Serial.println("Entrar a modo configuracion");
  Serial.println(WiFi.softAPIP()); //imprime o IP do AP
  Serial.println(myWiFiManager->getConfigPortalSSID()); //imprime o SSID criado da rede</p><p>}</p><p>//callback que indica que salvamos uma nova rede para se conectar (modo estação)
}


void saveConfigCallback () {
  Serial.println("Configuracion Guardada");
  Serial.println(WiFi.softAPIP()); //imprime o IP do AP
}
