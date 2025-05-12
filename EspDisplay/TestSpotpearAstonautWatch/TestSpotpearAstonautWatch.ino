
// Test Spotpear ESP32 C3 1.69inch Round Astronaut Clock Watch
// https://spotpear.com/wiki/ESP32-C3-Ornament-Trinket-LVGL-Astronaut-Clock-Watch-MINI-TV-1.69inch-Round-LCD-TouchScreen-ST7789-240x280.html
// Board: Adafruit QT Py Esp32C4

// 1,69 Zoll rundes LCD-Touchscreen-Display - ST7789 240 x 280
// Touch CST816D 
// Buzzer GPIO1



#include <Arduino.h>

static char* buffer=new char[500]; // buffer for char/logging

//--------------------------------------------------------------------------------------------
// WIFI

/*
#include <WiFi.h>
#include <privatdata.h>

const char *wifi_ssid_default =PRIVAT_WIFI_SSID; // define in privatdata.h 
const char *wifi_pas_default = PRIVAT_WIFI_PAS;   // define in privatdata.h 

void testWIFI() {
  Serial.print("Connecting to "); Serial.println(wifi_ssid_default);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid_default, wifi_pas_default);

  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }

  Serial.println(""); Serial.println("WiFi connected."); Serial.println("IP address: "); Serial.println(WiFi.localIP());
}
*/

//--------------------------------------------------------------------------------------------
// Remote Debug

/*
#include <TelnetStream.h>

void setupRemoteDebug() {  
  TelnetStream.begin();
}

void loopRemoteDebug(){
  switch (TelnetStream.read()) {
    case 'r': TelnetStream.stop(); delay(100); ESP.restart(); break;
    case 'C': TelnetStream.println("bye bye"); TelnetStream.stop(); break;
    case 't': logprintln("test"); break;
  }  
}

*/
void logprint(const char *text) { 
  Serial.print(text); 
//  TelnetStream.print(text);
}
void logprintln(const char *text) { 
  Serial.println(text); 
//  TelnetStream.println(text); 
}


//--------------------------------------------------------------------------------------------
// OTA

/*
#include <NetworkUdp.h>
#include <ArduinoOTA.h>

boolean otaRunning=false;

void testOTA() {
   ArduinoOTA
    .onStart([]() { String type;
      if (ArduinoOTA.getCommand() == U_FLASH) { type = "sketch"; } else { type = "filesystem"; }
      Serial.println("Start updating " + type);
      otaRunning=true;
    })
    .onEnd([]() { Serial.println("\nEnd"); })
    .onProgress([](unsigned int progress, unsigned int total) { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) { Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) { Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) { Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) { Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) { Serial.println("End Failed"); }
    });

  ArduinoOTA.begin();
}

void testOTALoop() {
   ArduinoOTA.handle();
}
*/

//--------------------------------------------------------------------------------------------
// Display

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>


//  
#define TFT_CS 3
#define TFT_DC 2
#define TFT_MOSI 6
#define TFT_CLK 5
#define TFT_RST 8

//#define TFT_BACKLIGHT 4

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);

int col=ST77XX_WHITE;
int pixelX=240, pixelY=280;
int i=0,ii=0;

void testDisplay() {
  logprintln("test display start");
//  pinMode(TFT_BACKLIGHT, OUTPUT);
//  digitalWrite(TFT_BACKLIGHT, HIGH);  //ON
  
  tft.init(pixelX,pixelY);
//  tft.setRotation(2);      // Rotation, 0-4

  //----------------------------------

  tft.fillScreen(ST77XX_BLACK);
  int wh=pixelX/2, wy=pixelY/2;
  float hp=(float)pixelY/(float)pixelX;

  for(i=0;i<wh;i++) {
    int ii=(int)(i*hp);
    tft.drawRect(wh-i,wy-ii,(i*2),(ii*2),col);
    delay(5);  
  }

  tft.drawLine(0,0,100,100,ST77XX_RED);
  tft.drawLine(0,0,50,100,ST77XX_RED);
  delay(1000);
  logprintln("test display end");
}

void testDisplayLoop() {
  int wh=pixelX/2, wy=pixelY/2;
  float hp=pixelY/pixelX;
  
  int ii=i*hp;
  tft.drawRect(wh-i,wy-ii,(i*2),(ii*2),col);
  i++; if(i>pixelX) { i=0;  }

  col-=1000;


}

//--------------------------------------------------------------------------------------------
// LCD Backlight

/*
void testBacklight() {
  logprintln("test backlight start");
  pinMode(TFT_BACKLIGHT, OUTPUT);
  for(int i=0;i<5;i++) {
    digitalWrite(TFT_BACKLIGHT, LOW);
    delay(500);
    digitalWrite(TFT_BACKLIGHT, HIGH);
    delay(500);
  }
  logprintln("test backlight end");
}
*/

//--------------------------------------------------------------------------------------------
// Touch

#include "CST816D.h"

int TP_SDA=11;
int TP_SCL=7;

int TP_IRQ=9;
int TP_RST=10;

CST816D *touch;

void testTouchSetup() {
  touch=new CST816D(TP_SDA,TP_SCL,TP_RST,TP_IRQ);
  touch->begin();
}

void testTouch() {
  uint8_t gesture;
  uint16_t touchX, touchY;

  bool touched = touch->getTouch(&touchX, &touchY, &gesture);
  if(touched) {  
    Serial.print(" x:");Serial.print(touchX);Serial.print(" y:");Serial.print(touchY);
    Serial.print(" g:");Serial.println(gesture);
    testBuzzer(); // => sound touch
  }
}


//--------------------------------------------------------------------------------------------
// Buzzer

int buzzer_pin=1;

void testBuzzer() {
  pinMode(buzzer_pin, OUTPUT);

  for(int i=0;i<10;i++) {
    digitalWrite(buzzer_pin, HIGH);  //ON
    delay(10);
    digitalWrite(buzzer_pin, LOW);  //ON
    delay(10);
  }
}



//--------------------------------------------------------------------------------------------
// Button
int sw2=0;
byte sw2Last=0;

void testButton() {
  sprintf(buffer,"test Button start 2:%d",sw2); logprintln(buffer);
  pinMode(sw2, INPUT_PULLDOWN); 

}


void testButtonLoop() {
    byte b=digitalRead(sw2);
    if(sw2Last!=b) { sw2Last=b;  sprintf(buffer,"Button2 %d",b); logprintln(buffer);  }
}

//--------------------------------------------------------------------------------------------

void setup() {
  delay(1); Serial.begin(115200); 
  delay(1); Serial.println("----------------------------------------------------------------------------------------------------------------------");

//  testWIFI();
//  setupRemoteDebug();
//  testOTA();

  testBuzzer();
  testButton();
//  testBacklight();
  testDisplay();  
  testTouchSetup();
}

void loop() {  
//  if(!otaRunning) {
   testButtonLoop();
   testDisplayLoop();  
   testTouch();
//    loopRemoteDebug();
//  }
//  testOTALoop();
}
