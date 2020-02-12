/* code	color
0x0000	Black
0xFFFF	White
0xBDF7	Light Gray
0x7BEF	Dark Gray
0xF800	Red
0xFFE0	Yellow
0xFBE0	Orange
0x79E0	Brown
0x7E0	Green
0x7FF	Cyan
0x1F	Blue
0xF81F	Pink

 */

  #include "Free_Fonts.h"
  #include <M5Stack.h>
	#include <OneWire.h>
	#include <DallasTemperature.h>

  #define CF_OL32 &Orbitron_Light_32
      // GPIO where the DS18B20 is connected to
    const int oneWireBus = 2;     

    // Setup a oneWire instance to communicate with any OneWire devices
    OneWire oneWire(oneWireBus);

    // Pass our oneWire reference to Dallas Temperature sensor 
    DallasTemperature sensors(&oneWire);

    uint16_t glColor = TFT_BLUE;

	void setup() {
    M5.begin();
      extern unsigned char Temperature[];
      M5.Lcd.fillScreen(TFT_WHITE); 
      M5.Lcd.setFreeFont(CF_OL32);
      M5.Lcd.drawBitmap(-60,40,180,180,(uint16_t*)Temperature);//posicion donde se coloca la imagen y ancho y largo
      sensors.begin();    
 	  }


    
    void loop() {
      M5.Lcd.setTextColor(0x51d, TFT_WHITE);
      sensors.requestTemperatures(); 
      float temperatureC = sensors.getTempCByIndex(0);
      M5.Lcd.fillRect(160,120,80,600,TFT_WHITE);
      M5.Lcd.drawNumber(temperatureC,140,80,8);
      delay(2000);
    }
    


		


