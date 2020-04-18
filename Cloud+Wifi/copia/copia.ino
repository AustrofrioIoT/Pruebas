
///WiFi Web Server
   
#include <WiFi.h>      //ESP32 Core WiFi Library    
#include <ESP8266WebServer.h>  //Local WebServer used to serve the configuration porta
#include <WebServer.h> //Local DNS Server used for redirecting all requests to the configuration portal 
#include <DNSServer.h> //Local WebServer used to serve the configuration portal 
#include <WiFiManager.h>   // WiFi Configuration Magic 
const int PIN_AP = 2;//Pin de Reset Wifi Configuration

//MQTT
#include "universal-mqtt.h"
#define PUBLISH_DELAY 1800000 //Tiempos de publicacion cada 30min(30*1000*60)
unsigned long lastMillis = 0;

//Sensor de Temperatura
#include <OneWire.h>
#include <DallasTemperature.h>
const int oneWireBus = 5; 
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

//Termometro Grafico
#define M5STACK_MPU6886
#include <M5Stack.h>
#include "FreeSansBold35pt7b.h"

//DISPLAY DEFINES
#define imgNameH Temp_hwb
#define imgNameL Temp_lwb
#define imgNameOK Temp_okwb
#define PicArray extern unsigned char
#define PicArrayL extern unsigned char
#define PicArrayH extern unsigned char
PicArray imgNameOK[];
PicArrayH imgNameH[];
PicArrayL imgNameL[];
#define BLACK 0x0000
#define RED 0xF800
#define CYAN 0x07FF
#define WHITE 0xFFFF

//GYROSCOPE DEFINES
float gyroX = 0.0F;
float gyroY = 0.0F;
float gyroZ = 0.0F;

//Variables Puerta
int contador;
int flag;
int andre[2];
int i=0;
int closed=0;

//Sonido
#include "SonidoPuerta.h"

void setup() {

   //Giroscopio
  M5.begin();
  M5.Power.begin();
  dacWrite(25,0);
  // Start the DS18B20 sensor
  sensors.begin();
  pinMode(LED_BUILTIN, OUTPUT); //Led Indicador
  
  M5.IMU.Init();//Inicio giroscopio
  M5.Lcd.setFreeFont(&FreeSansBold35pt7b);
  M5.Lcd.clear(WHITE);
  M5.Lcd.setRotation(3);
  contador=0;
   
  // Start the Serial Monitor
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor

  //Servidor Wifi------------

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

  //Funcion Acceso al Cloud
  setupCloudIoT();

}

void loop() {
/////Bucle Puerta///////
  door();
  andre[i]=flag;
  i=i+1;
  if (i==2){
    if ((andre[0]==1)&&(andre[1]==1)){
      closed=closed+1;
      if (closed>=6){
        closed=5;
        read_temp();
        }
    }
    else if ((andre[0]==2)||(andre[1]==2))
    {
/////////Sonido Puerta///////////
      for (int i=0; i<45000;++i){
      dacWrite(25,rawData[i]);
      delayMicroseconds(34);
      }
      dacWrite (25,0);
///////////////////////
      closed=0;
    }
    i=0;
  }
  delay(500);

/// WifiManager 
  
  WiFiManager wifiManager;
    mqttClient->loop();
  delay(10);  // <- fixes some issues with WiFi stability
  //////////  
  if (!mqttClient->connected()) {
    connect();
  }


  // publish a message roughly every PUBLISH_DELAY ms.
  if (millis() - lastMillis > PUBLISH_DELAY) {
    lastMillis = millis();

     sensors.requestTemperatures(); 
     float temperatureC = sensors.getTempCByIndex(0);

    // Prepare a JSON payload string

    String payload = String("{\"timestamp\":") + time(nullptr) +
                     String(",\"temperature\":") + temperatureC +
                     String(",\"itemId\":{\"ubicacion\":{\"piso1\":\"enfermeria\"}}")+
                     String("}"); 
     Serial.println(payload);

    publishTelemetry(payload);
  }


 
  //Condicional Para Reset de AP con estado del boton C
   if (M5.BtnC.read() == HIGH) {
      Serial.println("Resetear AP"); //tenta abrir o portal
      if(!wifiManager.startConfigPortal("ESP_AP", "12345678") ){
        Serial.println("No se pudo Conectar");
        delay(2000);
        ESP.restart();
        delay(1000);
      }
      Serial.println("Equipo Conectado");
   }


}  

void door()
{
    M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);
  if (gyroY > 20)//(gyroY <= -30) - cuando no esta rotado dispositivo
  {
    M5.Lcd.clear(WHITE);
    M5.Lcd.setCursor(5, (int)(M5.Lcd.height() / 2));
    M5.Lcd.setTextColor(RED);
    M5.Lcd.drawCentreString("Cerrado", (int)(M5.Lcd.width() / 2), 90, 1);
    contador = contador + 1;
    if ((gyroY <= 20) && (gyroY >= -20) && contador == 1) 
    {
      contador = 0;
    }
    flag=1; //estado cerrado
  }
  else if (gyroY <= -30) //else if (gyroY > 20) - cuando no esta rotado dispositivo
  { //Se cambio 40 por 20 porque así detecta apertura lenta de la puerta.
    M5.Lcd.clear(CYAN);
    M5.Lcd.setCursor(5, (int)(M5.Lcd.height() / 2));
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.drawCentreString("Abierto", (int)(M5.Lcd.width() / 2), 90, 1); 
    flag=2; //estado abierto
  }
}

void read_temp()
{
  sensors.requestTemperatures();
  if (sensors.getTempCByIndex(0) == -127)
  {
    //M5.Lcd.fillRect(0, 0, (M5.Lcd.width() / 2) + 60, 230, WHITE);
    M5.Lcd.clear(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setCursor(10, 90);
    M5.Lcd.print("Error");
  }
  else
  {
    M5.Lcd.fillRect(0, 0, (M5.Lcd.width() / 2) + 60, 230, WHITE);
    M5.Lcd.setTextColor(RED);
    // M5.Lcd.setCursor(130, 70);
    M5.Lcd.setTextSize(2);
    M5.Lcd.drawNumber(sensors.getTempCByIndex(0), 20, 90);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(180, 90);
    M5.Lcd.print((char)0x6F);

    // Too high
    if (sensors.getTempCByIndex(0) > 8)
    {
      //M5.Lcd.fillRect((int)(M5.Lcd.width() / 2) + 60, 5, 100, 230, WHITE);
      M5.Lcd.drawBitmap((int)(M5.Lcd.width() / 2) + 60, 5, 100, 230, (uint16_t *)imgNameH);
    }

    //Too low
    else if (sensors.getTempCByIndex(0) <= -10)
    {
      //M5.Lcd.fillRect((int)(M5.Lcd.width() / 2) + 60, 5, 100, 230, WHITE);
      M5.Lcd.drawBitmap((int)(M5.Lcd.width() / 2) + 60, 5, 100, 230, (uint16_t *)imgNameL);
    }
    //OK
    else
    {
      //M5.Lcd.fillRect((int)(M5.Lcd.width() / 2) + 60, 5, 100, 230, WHITE);
      M5.Lcd.drawBitmap((int)(M5.Lcd.width() / 2) + 60, 5, 100, 230, (uint16_t *)imgNameOK);
    }
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
