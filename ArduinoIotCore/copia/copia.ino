
#include "universal-mqtt.h"
#include <OneWire.h>
#include <DallasTemperature.h>

const int oneWireBus = 12; 
#define PUBLISH_DELAY 180000 //Tiempos de publicacion cada 30min(30*1000*60)
unsigned long lastMillis = 0;
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);



void setup() {
  // Start the Serial Monitor
  Serial.begin(115200);
  // Start the DS18B20 sensor
  sensors.begin();
  pinMode(LED_BUILTIN, OUTPUT); //Led Indicador
  setupCloudIoT(); 
}

void loop() {
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
     Serial.print(temperatureC);
     Serial.println("ºC");

    // Prepare a JSON payload string

    String payload = String("{\"timestamp\":") + time(nullptr) +
                     String(",\"temperature\":") + temperatureC +
                     /*String(",\"humidity\":") + humidity +*/
                     String("}"); 
   
    publishTelemetry(payload);
  }
}
