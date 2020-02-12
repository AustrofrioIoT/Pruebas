// define must ahead #include <M5Stack.h>
#define M5STACK_MPU6886 
// #define M5STACK_MPU9250 
// #define M5STACK_MPU6050
// #define M5STACK_200Q

#include <M5Stack.h>

float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;

float gyroX = 0.0F;
float gyroY = 0.0F;
float gyroZ = 0.0F;

float pitch = 0.0F;
float roll  = 0.0F;
float yaw   = 0.0F;

float temp = 0.0F;
int contador;

// the setup routine runs once when M5Stack starts up
void setup(){

  // Initialize the M5Stack object
  M5.begin();
  /*
    Power chip connected to gpio21, gpio22, I2C device
    Set battery charging voltage and current
    If used battery, please call this function in your project
  */
  M5.Power.begin();
    
  M5.IMU.Init();

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(GREEN , BLACK);
  M5.Lcd.setTextSize(2);
  contador=0;
}




void loop() {
  
  //uint8_t buf[6];
    // put your main code here, to run repeatedly:
  M5.IMU.getGyroData(&gyroX,&gyroY,&gyroZ);
  //gyroX=((uint16_t)buf[0]<<8)|buf[1];  
 // gyroY=((uint16_t)buf[2]<<8)|buf[3];  
 // gyroZ=((uint16_t)buf[4]<<8)|buf[5];
 // M5.Lcd.printf("%6.2f  %6.2f  %6.2f      ", gyroX, gyroY, gyroZ);
    
   if (gyroY>43.76){
       M5.Lcd.drawString("Abierto", 160, 60,2);
       M5.Lcd.setCursor(0, 50);
       contador=contador+1;
       if (gyroY<-9 && contador==1)
       {
       M5.Lcd.setTextPadding(M5.Lcd.width() - 20);
       M5.Lcd.drawString("Abierto", 160, 60, 2);  
       contador=0;
       }
   }else if (gyroY<-9)
      {
        M5.Lcd.setTextPadding(M5.Lcd.width() - 20);
        M5.Lcd.drawString("Close", 160, 60, 2);
      }       
   delay(100);
}

