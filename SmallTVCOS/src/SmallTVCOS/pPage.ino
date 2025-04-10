
class PageFunc { 
private: 
  void (*pageSetup)();
  void (*pageLoop)();
  const char *pageName;

public:
  void doSetup() {  if(pageSetup!=NULL) { pageSetup(); } }
  void doLoop() {  if(pageLoop!=NULL) { pageLoop(); } }
  char* toString() { return (char*)pageName; }

  PageFunc() { pageSetup = NULL; pageLoop = NULL;  pageName=NULL; }
  PageFunc(const char *name,void (*pSetup)(),void (*pLoop)()) {
    pageName=name; pageSetup=pSetup; pageLoop=pLoop;
  }

}; 

PageFunc* pageFunc=NULL;  // actual page
MapList pages;   // list of pages

byte pageIndex=0; // actual page 0=off,1=title
unsigned long *pageRefreshTime = new unsigned long(0); // timer to refresh page

unsigned long *pageLoopTime = new unsigned long(0); // pageLoop timer 
int pageLoopValue=0;

int pageRefresh=60000; // full refresh page (pageSetup) / -1 == no refresh

//--------------------------------

/* clear from page */
void pageClear() {
  if(eeDisplay.displayBuffer) {  bufferClear(); } else { displayClear(); }
  _effectType=0; // effect off
}

/* enabel a page to display */
byte pageSet(int page) {
  if(page>=0) { 
    pageIndex=page;
    *pageRefreshTime=0;
    if(pageIndex==0) { pageFunc=NULL; } else { pageFunc=(PageFunc*)pages.get(pageIndex-1); }
    sprintf(buffer,"set page:%d pageRefreshTime:%d ",pageIndex,*pageRefreshTime);logPrintln(LOG_DEBUG,buffer);
  } 
  return pageIndex;
}

/* enabel a page to display */
byte pageChange(int pageAdd) {
  byte page=pageIndex+pageAdd;
  if(page<1) { page=pages.size(); }
  if(page>pages.size()) { page=1; } 
  return pageSet(page);
}

int pageCmdNr=0;


char* pageList() {
  sprintf(buffer,"");
  for(int i=0;i<pages.size();i++) {  
    pageFunc=(PageFunc*)pages.get(i);
    if(pageFunc!=NULL) {
      sprintf(buffer+strlen(buffer),"page %i %s\n",i,pageFunc->toString());
    }
  }
  return buffer;
}

char* pageDel(int pageIndex) {
  pages.del(pageIndex);
  return EMPTY;
}

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
  drawText(x,y,fontSize,prgTitle,col_red);   
  // version
  drawText(1,pixelY-(8*fontSize),fontSize,prgVersion,col_red);   
  // FreeHeap
  drawFull(pixelX-30,100,20,100,2,(int)ESP.getFreeHeap(),150000,col_red,col_white);

  if(eeMode!=EE_MODE_OK) {
    fillRect(pixelX-(8*fontSize)-5,pixelY-(8*fontSize)-1,pixelX,pixelY,col_red);
    sprintf(buffer, "%d", eeMode); drawText(pixelX-12,pixelY-8,fontSize,buffer,col_black);  
  }

  if(is(appIP)) {  
    drawText(1,(8*fontSize)+20,fontSize,(char*)appIP.c_str(),col_red);   
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
  drawText(100,5,fontSize,buffer,col_white);
  // Heap
  drawText(1,20,fontSize,"Heap",col_red);
  drawFull(100,20,100,10,2,(int)ESP.getFreeHeap(),150000,col_red,col_white);
  sprintf(buffer,"%d",ESP.getFreeHeap()); 
//  drawText(45,10,col_red,1,buffer);
// sketch
  drawText(1,40,fontSize,"Sketch",col_red);
  drawFull(100,40,100,10,2,(int)ESP.getSketchSize(),(int)ESP.getFreeSketchSpace(),col_red,col_white);
  // bootType
  drawText(1,60,fontSize,"CmdOs",col_red);
  drawText(100,60,fontSize,bootType,col_white);
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
// PAGE TIME

void pageTimeDraw() {
  fillRect(0,50,240,20,col_black);
  drawTime(50,50,3,col_red);
  drawDate(20,140,3,col_red);
  fillRect(0,120,240,20,col_black);
  drawLine(20,100,200,100,col_white);
  draw();
}

void pageTime() {
  pageClear();
  pageTimeDraw();
}  

void pageTimeLoop() {
  if(isTimer(pageLoopTime, 500)) { 
    pageLoopValue++; if(pageLoopValue>1) { pageLoopValue=0; }
    if(pageLoopValue==1) { drawLine(20,100,200,100,col_red); }
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

void pageSetup() {
  pages.add(new PageFunc("title",pageTitle,NULL));
  pages.add(new PageFunc("esp",pageEsp,NULL));
//  pages.add(new PageFunc("test"",pageTest,pageTestLoop));
  pages.add(new PageFunc("time",pageTime,pageTimeLoop));
  pages.add(new PageFunc("gif",pageGif,pageGifLoop));
//  pages.add(new PageFunc(page_cmd,pageCmd,NULL));

  pageStart();  
  pageSet(1); 
}

void pageLoop() {
  if(!displayEnable && !_displaySetup) { return ; }
  if(pageIndex>0 && isTimer(pageRefreshTime, pageRefresh)) {

    if(pageFunc!=NULL) { 
      sprintf(buffer,"page setup %d",pageIndex);logPrintln(LOG_DEBUG,buffer);    
      pageFunc->doSetup(); 
    }else {
      sprintf(buffer,"no page found %d",pageIndex);logPrintln(LOG_ERROR,buffer);   
    }  
  } else {
    if(pageFunc!=NULL) { pageFunc->doLoop(); }
  }
}
