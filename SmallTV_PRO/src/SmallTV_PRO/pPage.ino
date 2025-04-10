
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

//--------------------------------------------------------
void pageSetup() {
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
