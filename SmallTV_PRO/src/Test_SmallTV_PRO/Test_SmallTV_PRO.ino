
// Test SmallTV Pro

// Infos from https://community.home-assistant.io/t/installing-esphome-on-geekmagic-smart-weather-clock-smalltv-pro/618029

// 984544 bytes as uPesy ESP Wroom

#include <Arduino.h>

static char* buffer=new char[500]; // buffer for char/logging

//--------------------------------------------------------------------------------------------
// WIFI

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


//--------------------------------------------------------------------------------------------
// Remote Debug


//#include <TimeLib.h>
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

void logprint(const char *text) { Serial.print(text); TelnetStream.print(text); }
void logprintln(const char *text) { Serial.println(text); TelnetStream.println(text); }

//--------------------------------------------------------------------------------------------
// OTA

//#include <ESPmDNS.h>
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

//--------------------------------------------------------------------------------------------
// Display

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_CLK 18
#define TFT_MOSI 23
#define TFT_DC 2
#define TFT_RST 4
#define TFT_CS -1
#define TFT_BACKLIGHT 25

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

int col=ST77XX_WHITE;
int pixelX=240, pixelY=240;
int i=0,ii=0;

void testDisplay() {
  logprintln("test display start");
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, LOW);
  
  tft.init(pixelX,pixelY, SPI_MODE3);
  tft.setRotation(2);      // Rotation, 0-4

  //----------------------------------

  tft.fillScreen(ST77XX_BLACK);
  int wh=pixelX/2, wy=pixelY/2, hp=pixelY/pixelX, ii=0;
  for(i=0;i<wh;i++) {
    tft.drawRect(wh-i,wy-ii,(i*2),(ii*2),col);
    ii+=hp;  
    delay(5);  
  }

  logprintln("test display end");
}

void testDisplayLoop() {
  int wh=pixelX/2, wy=pixelY/2, hp=pixelY/pixelX;
  
  tft.drawRect(wh-i,wy-ii,(i*2),(ii*2),col);
  i++; if(i>pixelX) { i=0;  sprintf(buffer,"test display pixelX end"); logprintln(buffer); }
  ii+=hp;  if(ii>pixelY) { ii=0; }

  col-=1000;
}

//--------------------------------------------------------------------------------------------
// Button
int sw=32;

void testButton() {
  sprintf(buffer,"test Button start %d",sw); logprintln(buffer);
  pinMode(sw, INPUT_PULLDOWN); 

  for(int i=0;i<60;i++) {
  //    byte swNow=digitalRead(sw);
    byte swNow=touchRead(sw);
    sprintf(buffer,"Button %d",swNow); logprintln(buffer);
    delay(1000);
  }  
  logprintln("test Button end");
}

void testButtonLoop() {
    byte swNow=touchRead(sw);
    sprintf(buffer,"Button %d",swNow); logprintln(buffer);
}

//--------------------------------------------------------------------------------------------

void setup() {
  delay(1); Serial.begin(115200); 
  delay(1); Serial.println("----------------------------------------------------------------------------------------------------------------------");

  testWIFI();
  setupRemoteDebug();
  testOTA();

//  testButton();
  testDisplay();  
}

void loop() {  
  if(!otaRunning) {
//  testButtonLoop();
    testDisplayLoop();  
    loopRemoteDebug();
  }
  testOTALoop();
}
