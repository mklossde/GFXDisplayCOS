
//--------------------------------

void pageTitle() {
  pageClear();

  int wh=pixelX/2, hh=pixelY/2;

  // WIFI  
  long val=0; if(WiFi.RSSI()!=0) { val=(100+WiFi.RSSI())/2; }

  uint16_t w=50,h=50;
  int x=wh,y=150;
  drawArc(x, y, 3, 90, 60, w, h, 3, col_red);
  drawArc(x, y, 3, 90, 60, w-10, h-10, 3, col_red);
  drawArc(x, y, 3, 90, 60, w-20, h-20, 3, col_red);  
  drawArc(x, y ,3, 90, val, w, h, 3, col_green);
  drawArc(x, y, 3, 90, val, w-10, h-10, 3, col_green);
  drawArc(x, y, 3, 90, val, w-20, h-20, 3, col_green);

  fillTriangle(x, y, x-w, y-h, x-w, y, col_black);
  fillTriangle(x, y, x+w, y-h, x+w, y, col_black);
  fillCircle(x, y+10, 3, col_green);

  // title
  int16_t  x1, y1; 
  display->getTextBounds(prgTitle, 0, 0, &x1, &y1, &w, &h);
  x=wh-w/2, y=1;
  drawText(x,y,fontSize,prgTitle,col_red,-1);   
  // version
  drawText(1,pixelY-(8*fontSize),fontSize,prgVersion,col_red,-1);   
  // FreeHeap
  drawFull(pixelX-30,100,20,100,2,(int)ESP.getFreeHeap(),150000,col_red,col_white);

  if(eeMode!=EE_MODE_OK) {
    fillRect(pixelX-(8*fontSize)-5,pixelY-(8*fontSize)-1,pixelX,pixelY,col_red);
    sprintf(buffer, "%d", eeMode); drawText(pixelX-12,pixelY-8,fontSize,buffer,col_black,-1);  
  }

  if(is(appIP)) {  
    drawText(1,(8*fontSize)+20,fontSize,(char*)appIP.c_str(),col_red,-1);   
  }

  draw();
//  sprintf(buffer, "drawTitle red:%d green:%d blue:%d  white:%d",col_red,col_green,col_blue,col_white); logPrintln(LOG_DEBUG,buffer);
}

void pageStart() {
  pageClear();  
  int wh=pixelX/2, wy=pixelY/2, hp=pixelY/pixelX, ii=0;
  int d=1000/wh;
  for(int i=0;i<wh;i++) {
    drawRect(wh-i,wy-ii,(i*2),(ii*2),col_white);
    ii+=hp;  
    draw();
    delay(d);  
  }
}

//-------------------------------------------------------------------------
// PAGE ESP

void pageTest() {
  pageClear();
  pageLoopValue=0;
}

void pageTestLoop() {
  int wh=pixelX/2; int d=1000/wh;

  if(isTimer(pageLoopTime, d)) {        
    if(pageLoopValue<wh) {
      int wy=pixelY/2, hp=pixelY/pixelX;
      int ii=pageLoopValue*hp;
      drawRect(wh-pageLoopValue,wy-ii,(pageLoopValue*2),(ii*2),col_white);
      draw();
      pageLoopValue++;
    }
  }
}


//-------------------------------------------------------------------------
// PAGE ESP

/* show esp page */
void pageEsp() {
  pageClear();

  uint32_t chipid=espChipId(); // or use WiFi.macAddress() ?
  snprintf(buffer,20, "%08X",chipid);
  drawText(100,5,fontSize,buffer,col_white,-1);
  // Heap
  drawText(1,20,fontSize,"Heap",col_red,-1);
  drawFull(100,20,100,10,2,(int)ESP.getFreeHeap(),freeHeapMax,col_red,col_white);
  sprintf(buffer,"%d",ESP.getFreeHeap()); 
//  drawText(45,10,col_red,1,buffer);
// sketch
  drawText(1,40,fontSize,"Sketch",col_red,-1);
  drawFull(100,40,100,10,2,(int)ESP.getSketchSize(),(int)ESP.getFreeSketchSpace(),col_red,col_white);
  // bootType
  drawText(1,60,fontSize,"CmdOs",col_red,-1);
  drawText(100,60,fontSize,bootType,col_white,-1);
//  // mac
//  uint8_t baseMac[6]; esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
//  sprintf(buffer,"%02x:%02x:%02x:%02x:%02x:%02x\n",baseMac[0], baseMac[1], baseMac[2],baseMac[3], baseMac[4], baseMac[5]);
//  drawText(1,40,col_red,1,"Mac");
//  drawText(20,40,col_white,1,buffer);

  int px=pixelX/12;
  int py=pixelY/3, yy=pixelY-py;
  int a[13];
  // bar chart
  for(int i=0;i<12;i++) {
    int v=random(0,21); a[i]=v;
    drawFull(i*px+1,yy,px,py,1,v,21,col_black,col_red);
  }
  drawLine(1,pixelY-3,pixelX-1,pixelY-3,col_red);
  // line chart
  for(int i=1;i<12;i++) {    
    drawLine((i-1)*px+3,yy+(a[i-1]),i*px+3,yy+(a[i]),col_white);
  }    
  draw();
}

//-------------------------------------------------------------------------
// PAGE state

char *haStatePrefix="homeassistant2/";

class DisValue {
  private:
    byte _type;

    void init(char *config) {
      valKey=copy(config); // e.g switch.tasmota
      char *haType=extract(NULL,".",valKey);
      char *haName=extract(".",NULL,valKey);            
      valRemote=concat(haStatePrefix,haType,"/",haName,"/state",NULL); // e.g. "homeassistant2/switch/tasmota/state";
      sprintf(buffer,"DisValue type:%d key:%s haType:%s haName:%s valRemote:%s",_type,valKey,haType,haName,valRemote); logPrintln(LOG_DEBUG,buffer);      
    }

  public:

  //  char *valName="val";
    char *valKey; //="value";
    char *valRemote; //="homeassistant2/switch/tasmota/state";
    int valType=0;

    double valF=-1;
    double minF=999999;
    double maxF=0;
    double medF=-1;

    int col1=-1,col2=-1,col3=-1;
    int x=-1,y=-1,w=-1;

    void setVal(char *val) {
      valF=toDouble(val);
      if(valF<minF) { minF=valF; }
      if(valF>maxF) { maxF=valF; }
      sprintf(buffer,"DisValue val:%s valF:%0.2f minF:%0.2f maxF:%0.2f",val,valF,minF,maxF);  logPrintln(LOG_DEBUG,buffer);        
    }

    void reinit(char *config) {
      if(valKey) { free(valKey); }
      if(valRemote) { free(valRemote); }
      valF=-1,minF=999999,maxF=0,medF=-1;
      init(config);
    } 
    void drawUnkown() {
      if(is(valKey)) { drawText(x+w/2,y+w/2,fontSize,valKey,col_red,0); }
      drawRect(x,y,w,w,col_red); drawLine(x,y,x+w,y+w,col_red);drawLine(x,y+w,x+w,y,col_red); 
    }
    void draw(char *val) {
      if(x<0) { x=10; y=10; }
      if(w<0) { w=pixelX-x*2; if(pixelY<pixelX) { w=pixelY-y*2; } }
      if(!is(val)) { drawUnkown(); return ; } // draw unkown
      setVal(val);
      drawValue(valType,x,y,w,valKey,val,minF,medF,maxF,col1,col2,col3);
    }  
    char* toString() {
      sprintf(buffer,"DisValue type:%d key:%s valRemote:%s valType:%d",_type,valKey,valRemote,valType); return buffer;
    }
    DisValue(char *config) { init(config); }
};

DisValue *stateValue=NULL;

/* show page state */
void pageState() {  
//  if(!stateValue) { stateValue=new DisValue("switch.tasmota"); }
//  mqttAttr("value",stateAttr);
  if(stateValue) { mqttAttr(stateValue->valKey,stateValue->valRemote); }
  pageStateLoop();
}

void pageStateValue() {
//TODO get via pram ?  
  char *val=attrGet(stateValue->valKey); 
  stateValue->draw(val); 
}

void pageStateLoop() {
  pageClear();
  if(stateValue) {  pageStateValue();}
  draw();  
}

char* pageStateSet(char *config) {
  if(is(config)) { 
    if(!stateValue) { stateValue=new DisValue(config);  }
    else { stateValue->reinit(config); }
  }
  return stateValue->toString();
}

//-------------------------------------------------------------------------
// PAGE TIME

void pageTimeDraw() {
  fillRect(0,50,240,25,col_black);
  drawTime(50,50,3,col_red,-1);
  fillRect(0,140,240,25,col_black);
  drawDate(30,140,3,col_red,-1);  
  drawLine(20,100,220,100,col_white);
  draw();
}

void pageTime() {
  pageClear();
  pageTimeDraw();
}  

void pageTimeLoop() {
  if(isTimer(pageLoopTime, 500)) { 
    pageLoopValue++; if(pageLoopValue>1) { pageLoopValue=0; }
    if(pageLoopValue==1) { drawLine(20,100,220,100,col_red); }
    else { pageTimeDraw();  }
  }
}

//-------------------------------------------------------------------------
// PAGE GIF

void pageGif() {
  pageClear();
}

void pageGifLoop() {
  if(isTimer(pageLoopTime, 1000)) { 
    int max=fsDirSize(".gif");
    int f=random(0,max);
    char* name=fsFile(".gif",f,0);  
    if(!is(name)) { sprintf(buffer,"pageGif missing %d/%d",f,max);logPrintln(LOG_ERROR,buffer); return ; }

    sprintf(paramBuffer,"%s",name);
    sprintf(buffer,"pageGif %d/%d %s",f,max,to(paramBuffer));logPrintln(LOG_DEBUG,buffer);

    int px=random(0,pixelX-64),py=random(0,pixelY-64);
    drawFile(paramBuffer,paramBuffer,px,py,false);
//    delay(250);
//    drawFileClose();
//    int rx=random(-1,2)*5;
//    int ry=random(-1,2)*5;
//    effectStart(1,64,20,rx,ry);
  }
}

//-----------------------------------------------------------

void displayPageSetup() {
  pages.add(new PageFunc("title",pageTitle,NULL,10000));
  pages.add(new PageFunc("esp",pageEsp,NULL,10000));
//  pages.add(new PageFunc("test",pageTest,pageTestLoop));
  pages.add(new PageFunc("time",pageTime,pageTimeLoop,500));
  pages.add(new PageFunc("gif",pageGif,pageGifLoop,60000));
//  pages.add(new PageFunc(page_cmd,pageCmd,NULL));
  pages.add(new PageFunc("state",pageState,pageStateLoop,5000));

  pageStart();  
  pageSet(0); 

//  mqttDiscoverAdd("page",page(),"page $msg"); // pages()
//  mqttDiscoverAdd("off",page(),"page $msg"); // pages()
}

void displayPageLoop() {
}
