
// SmallTVCOS - Esp32 SmallTV based on GFXDisplayCOS and CmdOS 
// dev by OpenOn.org source from http://github.com/mklossde/GFXDisplayCOS

// GFXDisplayCOS - http://github.com/mklossde/GFXDisplayCOS
// CmdOS - http://github.com/mklossde/CmdOs


#include <AnimatedGIF.h>

#include <privatdata.h>

const char *prgTitle = "SmallTVPROCOS";
const char *prgVersion = "V0.4.0";

const char* user_admin = "admin"; // default user
char user_pas[]="admin";   // default espPas

const char *wifi_ssid_default = PRIVAT_WIFI_SSID; // define in privatdata.h 
const char *wifi_pas_default = PRIVAT_WIFI_PAS;   // define in privatdata.h 
const char *mqtt_default = PRIVAT_MQTTSERVER;     // define in privatdata.h 

//byte MODE_DEFAULT=21; // normal=21=MODE_WIFI_CL_TRY /
byte MODE_DEFAULT=20; //  MODE_PRIVAT=20=load privat values, use wifi_ssid_default and wifi_pas_default and mqtt_default
//byte MODE_DEFAULT=0; // EE_MODE_FIRST=0=RESET on start

boolean serialEnable=true; // enable/disbale serial in/out
boolean bootSafe=true;    // enable/disbale boot safe
boolean wifiEnable=true;  // enable/disbale wifi

#define mdnsEnable false   // enable/disable mDNS detection 

#define webEnable true    // enable/disbale http server
#define ntpEnable false     // enable time server
#define enableFs true         // enable fs / SPIFFS

#define netEnable true       // enable/disbale network ping/dns/HttpCLient 
#define telnetEnable false       // enable/disbale telnet
#define webSerialEnable false // enable/disbale web serial
#define mqttEnable true      // enable/disbale mqtt
#define mqttDiscovery false   // enable mqtt Homeassistant Discovery  
boolean mqttCmdEnable=true;  // enable mqtt sedn/receive cmd
boolean mqttLogEnable=false;  // enable mqtt send log

#define otaEnable false        // enabled/disbale ota update 
#define updateEnable false     // enabled/disbale update firmware via web 

#define ledEnable false       // enable/disbale serial
int ledGpio=15;            // io of led
boolean ledOnTrue=true;           // gpio false=led-on

#define SW_MODE_NORMAL 0 // Button
#define SW_MODE_PULLUP 1 // Button with pullUp enabled
#define SW_MODE_PULLDOWN 2 // Button with pullDown enabled
#define SW_MODE_TOUCH 3   // TOUCH BUTTON

#define swEnable true        // enable/disbale switch
int swGpio=32;                // io pin of sw 
byte swOn=3;                // 0=pressend on false, 1= pressed on true, 2=use actual as off, 3-9=use actual minus dif, >9 pressed below swON
byte swMode=SW_MODE_TOUCH;  // mode pullUp/pullDown/touch

int swTimeBase=100;     // unprell (max state change time) and tick time base (e.g. 100ms)
byte swTickShort=2;     // swTickShort*swTimeBase => 5*100 => 500ms;
byte swTickLong=5;      // swTickLong*swTimeBase => 10*100 => 1s;

// app
boolean displayEnable=true; // enable display

int _webPort = 80;
 
#if webEnable
  #include <ESPAsyncWebServer.h>
  AsyncWebServer webServer(_webPort);
#endif

//--------------------------------------------------------------


char* appCmd(char *cmd, char **param) {
    return matrixCmd(cmd,param); 
}

/** call on mqtt startup */
void mqttOnConnect() {
  #if mqttDiscovery
    mqttDiscover("text","page",true,true); // set page ##page##
    mqttDiscover("text","display",true,true); // set page
  #endif
}
boolean mqttOnMsg(char *topic,char *msg) { 
//Serial.print("mqttOnMsg topic:"); Serial.println(topic) ;  
  if(endsWith(topic,"page")) { pageSet(msg); return true; }
  else if(endsWith(topic,"display")) { pageStateSet(msg); return true; }
  return false; 
}

//--------------------------------------------------------------


/* callback to add app web pages */
void webApp() {
  #if webEnable
    webServer.on("/app", HTTP_GET, [](AsyncWebServerRequest *request) { matrixWeb(request); });
//    webServer.on("/appSetup", HTTP_GET, [](AsyncWebServerRequest *request) { matrixWebSetup(request); });
  #endif
}

//--------------------------------------------------------------

void setup() {
  cmdOSSetup(); 
  if(isModeNoError()) { 
    displaySetup();
    pageSetup();
    displayPageSetup();
    swCmd(1,"pageNext");
    swCmd(2,"pagePriv");
  }
}

void loop() {
  cmdOSLoop(); 
  if(isModeNoError()) { 
    displayLoop();
    drawLoop();
    pageLoop();
    displayPageLoop();
    effectLoop();    
  }
}
