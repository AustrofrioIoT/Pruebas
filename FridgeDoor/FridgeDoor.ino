#define M5STACK_MPU6886

#include <M5Stack.h>
#include "FreeSansBold35pt7b.h"

//TEMPERATURE DEFINES
#include <OneWire.h>
#include <DallasTemperature.h>

const int pinDatosDQ = 5;

OneWire oneWireObjeto(pinDatosDQ);
DallasTemperature sensorDS18B20(&oneWireObjeto);

//GYROSCOPE DEFINES
float gyroX = 0.0F;
float gyroY = 0.0F;
float gyroZ = 0.0F;

int contador;
int flag;
int andre[2];
int i=0;
int closed=0;

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

void setup()
{
  M5.begin();
  M5.Power.begin();
  dacWrite(25,0);
  sensorDS18B20.begin(); //Inicio sensor
  M5.IMU.Init();         //Inicio giroscopio
  M5.Lcd.setFreeFont(&FreeSansBold35pt7b);
  M5.Lcd.clear(WHITE);
  M5.Lcd.setRotation(3);
  Serial.begin(115200);
  contador=0;
}

void loop()
{
  door();
  andre[i]=flag;
  i=i+1;
  if (i==2){
    if ((andre[0]==1)&&(andre[1]==1)){
      closed=closed+1;
      Serial.println(closed);
      if (closed>=6){
        closed=5;
        read_temp();
        }
    }
    else if ((andre[0]==2)||(andre[1]==2))
    {
      closed=0;
    }
    i=0;
  }
  delay(500);

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
  sensorDS18B20.requestTemperatures();
  if (sensorDS18B20.getTempCByIndex(0) == -127)
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
    M5.Lcd.drawNumber(sensorDS18B20.getTempCByIndex(0), 20, 90);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(180, 90);
    M5.Lcd.print((char)0x6F);

    // Too high
    if (sensorDS18B20.getTempCByIndex(0) > 8)
    {
      //M5.Lcd.fillRect((int)(M5.Lcd.width() / 2) + 60, 5, 100, 230, WHITE);
      M5.Lcd.drawBitmap((int)(M5.Lcd.width() / 2) + 60, 5, 100, 230, (uint16_t *)imgNameH);
    }

    //Too low
    else if (sensorDS18B20.getTempCByIndex(0) <= -10)
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
