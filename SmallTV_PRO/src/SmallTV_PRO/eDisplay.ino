
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_CLK 18
#define TFT_MOSI 23
#define TFT_DC 2
#define TFT_RST 4
#define TFT_CS -1
#define TFT_BACKLIGHT 25

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

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

//---------------------------------------------------------------------------

// display store structur
typedef struct {
  int pX=pixelX;
  int pY=pixelY;
  int mode=SPI_MODE3;
  char pins[64]="18,23,2,4,-1,25";
  byte brightness=90;
  byte panelChain=1;
  byte rotation=2;
  boolean displayBuffer=false;
  boolean dmaBuffer=false;
} eeDisplay_t;
eeDisplay_t eeDisplay;    // matrix store object 

char* displayInfo() {
  sprintf(buffer, "Matrix enabled:%d ok:%d pixelX:%d pixelY:%d rotation:%d brightness:%d panelChain:%d pins:%s disBuffer:%d dmaBuffer:%d",
    displayEnable,_displaySetup,eeDisplay.pX,eeDisplay.pY,eeDisplay.rotation,eeDisplay.brightness,eeDisplay.panelChain,eeDisplay.pins,
    eeDisplay.displayBuffer,eeDisplay.dmaBuffer);
    return buffer;
}

void displaySave() {
  if(eeAppPos<=0) { logPrintln(LOG_ERROR,"displaySave wrong eeAppPos."); return ; }
  EEPROM.begin(EEBOOTSIZE);

  EEPROM.put(eeAppPos, eeDisplay ); 
  EEPROM.commit();  // Only needed for ESP8266 to get data written
  sprintf(buffer, "displaySave eeAppPos:%d pixelX:%d pixelY:%d rotation:%d pins:%s",
    eeAppPos,eeDisplay.pX,eeDisplay.pY,eeDisplay.rotation,eeDisplay.pins);logPrintln(LOG_INFO,buffer);
}

void displayLoad() {
  if(eeAppPos<=0) { logPrintln(LOG_ERROR,"displayLoad wrong eeAppPos.");return ; }
  else if(strcmp(eeType,bootType)!=0) { logPrintln(LOG_ERROR,"displayLoad type wrong"); return ; }   // validate

  EEPROM.begin(EEBOOTSIZE);
  EEPROM.get(eeAppPos, eeDisplay); // eeBoot read
  EEPROM.end(); 
  sprintf(buffer, "displayLoad eeAppPos:%d pixelX:%d pixelY:%d rotation:%d pins:%s",
    eeAppPos,eeDisplay.pX,eeDisplay.pY,eeDisplay.rotation,eeDisplay.pins);logPrintln(LOG_INFO,buffer);
}

boolean displayInit() {
//TODO displayLoad    displayLoad(); // load eeDisplay
    pixelX=eeDisplay.pX;
    pixelY=eeDisplay.pY;

    return true;
}

//---------------------------------------------------------------------------

/* draw actual screen => flip to back buffer */
void displayDraw() {
  if(!_displaySetup) { return ; }
//  if(eeDisplay.dmaBuffer) { tft.flipDMABuffer(); }// Show the back buffer, set currently output buffer to the back (i.e. no longer being sent to LED panels)
  else if(eeDisplay.displayBuffer) {
    GFXcanvas16 *canvas=(GFXcanvas16*)display;  
    uint16_t *buffer=canvas->getBuffer();
    if(buffer==NULL) { logPrintln(LOG_ERROR,"missing buffer"); return ; }
    tft.drawRGBBitmap(0,0,buffer,pixelX,pixelY);
  }
}

/* clear display */
void displayClear() { if(!_displaySetup) { return ; } tft.fillScreen(ST77XX_BLACK); }
void bufferClear() { 
  if(_displaySetup && eeDisplay.displayBuffer && display!=NULL) { 
    display->fillScreen(ST77XX_BLACK); 
  } 
}

/* set Brightness 0-255 **/
void displayBrightness(int b) {  } 
char* displayRotation(int r) {  if(r>=0) { tft.setRotation(r); } sprintf(buffer,"%d",tft.getRotation()); return buffer; } 

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
  displayInit();
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, LOW);

  tft.init(eeDisplay.pX,eeDisplay.pY, eeDisplay.mode);
  tft.setRotation(eeDisplay.rotation);      // Rotation, 0-4
  sprintf(buffer,"display px:%d py:%d mode:%d rotation:%d",eeDisplay.pX,eeDisplay.pY,eeDisplay.mode,eeDisplay.rotation);logPrintln(LOG_INFO,buffer);

  if(eeDisplay.displayBuffer) { 
    display=new GFXcanvas16(eeDisplay.pX, eeDisplay.pY);  
    if(display==NULL) {   // no ram
      logPrintln(LOG_ERROR,"displayBuffer mem MISSING. disable displayBuffer");
      display=&tft; eeDisplay.displayBuffer=false; 
    } else {
      logPrintln(LOG_INFO,"displayBuffer enabled");
    }
  }else { display=&tft; }

  col_red=toColor444(15,0,0);
  col_white=toColor444(15,15,15);
  col_black=toColor444(0,0,0);  
  col_green=toColor444(0,15,0);  
  col_blue=toColor444(0,0,15);    

  _displaySetup=true;
}

void displayLoop() {
  
}


