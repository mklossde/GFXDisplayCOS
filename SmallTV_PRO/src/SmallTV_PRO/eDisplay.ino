
//----------------------------------------------------
// GFXDisplaCOS
const char *GFXDisplaCOSVersion = "V0.4.0";

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

Adafruit_ST7789 *tft = NULL; // = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

Adafruit_GFX *display;

boolean _displaySetup=false; // display setup ok

byte _effectType=0;
byte fontSize=2;

int col_red;
int col_white;
int col_black;
int col_green;
int col_blue;

int pixelX=240;
int pixelY=240;

//String uploadFile=String("upload");
char* uploadFile="/upload_file";
String URL_FILE="/upload_url";

//---------------------------------------------------------------------------

byte eeDisplay_VERSION=0;

// display store structur
typedef struct {
  byte VERSION=eeDisplay_VERSION;
  int pX=pixelX;
  int pY=pixelY;  
  char libType[10]="ST7789";
  char pins[64]="18,23,2,4,-1,25";
  int mode=SPI_MODE3;
  byte brightness=90;
//  byte panelChain=1;
  byte rotation=2;
  byte displayBuffer=0; // 0=no Buffer, 1=local buffer, 2=dma buffer
  boolean backlightOn=false; // backligh is on by (true=high,low=false)
} eeDisplay_t;
eeDisplay_t eeDisplay;    // matrix store object 

//-----------------------------------------------------------

void displaySave() {
  if(eeAppPos<=0) { logPrintln(LOG_ERROR,"displaySave wrong eeAppPos."); return ; }
  EEPROM.begin(EEBOOTSIZE);

  EEPROM.put(eeAppPos, eeDisplay ); 
  EEPROM.commit();  // Only needed for ESP8266 to get data written
  logPrintln(LOG_INFO,"display save");
}

boolean displayLoad() {
  if(eeAppPos<=0) { logPrintln(LOG_ERROR,"displayLoad wrong eeAppPos.");return false; }
  else if(strcmp(eeType,bootType)!=0) { logPrintln(LOG_ERROR,"displayLoad type wrong"); return false; }   // validate

  EEPROM.begin(EEBOOTSIZE);
  EEPROM.get(eeAppPos, eeDisplay); // eeBoot read
  EEPROM.end(); 

  if(eeDisplay.VERSION!=eeDisplay_VERSION) {
    sprintf(buffer,"eeDisplay wrong %d need %d",eeDisplay.VERSION,eeDisplay_VERSION);logPrintln(LOG_ERROR,buffer);
    eeDisplay=eeDisplay_t();
    return false;
  }
  return true;
}

/* init display */
void displayInit() {
    _displaySetup=false;

    pixelX=eeDisplay.pX;
    pixelY=eeDisplay.pY;
    
//TODO use libType

    // decode pins and init display (CLK,MOSI,DC,RST,CS,BACKLIGHT)
    char* temp = strdup(eeDisplay.pins);char* token = strtok(temp, ",");
    int8_t TFT_CLK=(int8_t)atoi(token); token = strtok(NULL, ",");
    int8_t TFT_MOSI=(int8_t)atoi(token); token = strtok(NULL, ",");
    int8_t TFT_DC=(int8_t)atoi(token); token = strtok(NULL, ",");
    int8_t TFT_RST=(int8_t)atoi(token); token = strtok(NULL, ",");
    int8_t TFT_CS=(int8_t)atoi(token); token = strtok(NULL, ",");
    int8_t TFT_BACKLIGHT=(int8_t)atoi(token); token = strtok(NULL, ",");    
    free(temp);  // Free the duplicated string
    tft = new Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

    // enable backlight
    if(TFT_BACKLIGHT>0) { pinMode(TFT_BACKLIGHT, OUTPUT); digitalWrite(TFT_BACKLIGHT, eeDisplay.backlightOn); }

//TODO use brightness

    tft->init(eeDisplay.pX,eeDisplay.pY, eeDisplay.mode);
    tft->setRotation(eeDisplay.rotation);      // Rotation, 0-4

    // display buffer
    if(eeDisplay.displayBuffer) { 
      display=new GFXcanvas16(eeDisplay.pX, eeDisplay.pY);  
      if(display==NULL) {   // no ram
        logPrintln(LOG_ERROR,"displayBuffer mem MISSING. disable displayBuffer");
        display=tft; eeDisplay.displayBuffer=false; 
      } else {
        logPrintln(LOG_INFO,"displayBuffer enabled");
      }
    }else { display=tft; }

    // setup base colors
    col_red=toColor444(15,0,0);
    col_white=toColor444(15,15,15);
    col_black=toColor444(0,0,0);  
    col_green=toColor444(0,15,0);  
    col_blue=toColor444(0,0,15); 
    
    _displaySetup=true;
}

//---------------------------------------------------------------------------

char* displayInfo() {
//  sprintf(buffer, "display OK:%d config: %d %d %s %s %d %d %d %d %d",
  sprintf(buffer, "display OK:%d px:%d py:%d libType:%s pins:%s mode:%d eeDisplay:%d rotation:%d displayBuffer:%d backlightOn:%d",
    _displaySetup,
    eeDisplay.pX,eeDisplay.pY,to(eeDisplay.libType),to(eeDisplay.pins),eeDisplay.mode,eeDisplay.brightness,eeDisplay.rotation,
    eeDisplay.displayBuffer,eeDisplay.backlightOn);
    return buffer;
}

/* set param to display config 
    displaySet pX pY libType pins mode brightness rotation displayBuffer backlightOn
      pins:   TFT_CLK,TFT_MOSI,TFT_DC,TFT_RST,TFT_CS,TFT_BACKLIGHT
 *  e.g. displaySet 480 480 ST7701 48,47,-1,-1,39,38 0 100 0 0 0
*/
char* displaySet(char **param) {  
  int pX=toInt(cmdParam(param));
  if(pX>0 && isAccess(ACCESS_ADMIN)) { 
    eeDisplay.pX=pX;
    eeDisplay.pY=toInt(cmdParam(param));
    strcpy(eeDisplay.libType,cmdParam(param));
    strcpy(eeDisplay.pins,cmdParam(param));
    eeDisplay.mode=toInt(cmdParam(param));
    eeDisplay.brightness=toInt(cmdParam(param));
    eeDisplay.rotation=toInt(cmdParam(param));
    eeDisplay.displayBuffer=toInt(cmdParam(param));
    eeDisplay.backlightOn=toBoolean(cmdParam(param));
    displayInit();
  }
  return displayInfo();
}

//---------------------------------------------------------------------------

/* draw actual screen => flip to back buffer */
void displayDraw() {
  if(!_displaySetup) { return ; }
//  if(eeDisplay.dmaBuffer) { tft.flipDMABuffer(); }// Show the back buffer, set currently output buffer to the back (i.e. no longer being sent to LED panels)
  else if(eeDisplay.displayBuffer==1) {
    GFXcanvas16 *canvas=(GFXcanvas16*)display;  
    uint16_t *buffer=canvas->getBuffer();
    if(buffer==NULL) { logPrintln(LOG_ERROR,"missing buffer"); return ; }
    tft->drawRGBBitmap(0,0,buffer,pixelX,pixelY);
  }
}

/* clear display */
void displayClear() { if(!_displaySetup) { return ; } tft->fillScreen(ST77XX_BLACK); }
void bufferClear() { 
  if(_displaySetup && eeDisplay.displayBuffer && display!=NULL) { 
    display->fillScreen(ST77XX_BLACK); 
  } 
}

/* set Brightness 0-255 */
void displayBrightness(int b) {  } 
char* displayRotation(int r) {  if(r>=0) { tft->setRotation(r); } sprintf(buffer,"%d",tft->getRotation()); return buffer; } 

uint16_t toColor444(uint8_t r, uint8_t g, uint8_t b) {
//    return ((r & 0x0F) << 8) | ((g & 0x0F) << 4) | (b & 0x0F);
    uint16_t r5 = (r * 255 / 15) >> 3; // Scale to 5 bits
    uint16_t g6 = (g * 255 / 15) >> 2; // Scale to 6 bits
    uint16_t b5 = (b * 255 / 15) >> 3; // Scale to 5 bits
    return (r5 << 11) | (g6 << 5) | b5;    
}
uint16_t toColor565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) |  // Top 5 bits of red
           ((g & 0xFC) << 3) |  // Top 6 bits of green
           (b >> 3);            // Top 5 bits of blue
}


//---------------------------------------------------------------------------

void displaySetup() {
  if(!displayLoad()) { _displaySetup=false; return ; }
  logPrintln(LOG_INFO,displayInfo());  
  displayInit();     
}

void displayLoop() {
  
}


