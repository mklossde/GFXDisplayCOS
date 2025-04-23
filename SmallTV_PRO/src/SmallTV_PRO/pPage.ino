
//----------------------------------------------------
// Page

#define pageRefreshDefault 60000

unsigned long *pageRefreshTime = new unsigned long(0); // timer to refresh page

unsigned long *pageLoopTime = new unsigned long(0); // pageLoop timer 
int pageLoopValue=0;

int pageRefresh=pageRefreshDefault;

//----------------------------------------------------

class PageFunc { 
private: 
  void (*pageSetup)();
  void (*pageLoop)();  
  char *prgFile;
  int _refreshTime=pageRefreshDefault;
  void setPageName(char *name) { if(is(name)) { pageName=name; } else { pageName="unkown";}}
public:
  char *pageName;
  void doSetup() {      
    if(pageSetup!=NULL) { pageSetup(); } 
    else if(is(prgFile)) { cmdFile(prgFile); }
    pageRefresh=_refreshTime; 
  }
  void doLoop() {  if(pageLoop!=NULL) { pageLoop(); } }
  char* getPageName() { return pageName; }
  boolean isFile() { return is(prgFile); }

  PageFunc() { pageSetup = NULL; pageLoop = NULL;  setPageName(NULL); }
  PageFunc(char *file) { pageSetup = NULL; pageLoop = NULL;  setPageName(file); prgFile=file; }
  PageFunc(char *name,void (*pSetup)(),void (*pLoop)(),int refreshTime) { setPageName(name); pageSetup=pSetup; pageLoop=pLoop; _refreshTime=refreshTime;}

};  

PageFunc* pageFunc=NULL;  // actual page
MapList pages;   // list of pages

byte pageIndex=200; // actual page 200=off,1=title


//--------------------------------

/* clear from page */
void pageClear() {
  if(eeDisplay.displayBuffer) {  bufferClear(); } else { displayClear(); }
  _effectType=0; // effect off
}

/* enabel a page by index to display */
byte pageSet(int page) {
  if(page>=0 && page<=250) { 
    if(pageFunc!=NULL && pageFunc->isFile()) { cmdFile(NULL); } // stop old page 

    pageIndex=page;
//    if(refresh>0) {  pageRefresh=refresh; } // set page refresh
//    else { pageRefresh=pageRefreshDefault; } // reset to default 
    pageRefresh=0;*pageRefreshTime=0; // start page now 
    if(pageIndex==250) { pageFunc=NULL; pageRefresh=-1; } else { pageFunc=(PageFunc*)pages.get(pageIndex); }
    sprintf(buffer,"set page:%d",pageIndex);logPrintln(LOG_DEBUG,buffer);
  } 
  return pageIndex;
}

/* set page by index or name */
byte pageSet(char *page) { 
  if(!is(page)) { return pageIndex; }
  if(isInt(page)) { return pageSet(toInt(page)); }
  for(int i=0;i<pages.size();i++) {      
    PageFunc *pageFunc=(PageFunc*)pages.get(i);  
    if(pageFunc!=NULL && equals(pageFunc->getPageName(),page)) { return pageSet(i); }
  }
  sprintf(buffer,"unkown page %s",page); logPrintln(LOG_ERROR,buffer);
  return pageIndex;
}


/** add prg as nes page */
char* pageAdd(char *file) {
  if(is(file)) {
    char *f=copy(file);
    pages.add(new PageFunc(f));
    return f;
  }else { return "EMPTY"; }
}

/* enabel a page to display */
byte pageChange(int pageAdd) {
  byte page=pageIndex+pageAdd;
  if(pageIndex==250) { page=pageAdd; }  
  if(page>=250) { page=pages.size(); }
  if(page>=pages.size()) { page=0; } 
  return pageSet(page);
}

/* number of pages */
char* pageSize() { sprintf(buffer,"%d",pages.size()); return buffer; }

/* list all pages */
char* pageList() {
  sprintf(buffer,"");
  for(int i=0;i<pages.size();i++) {      
    PageFunc *pageFunc=(PageFunc*)pages.get(i);    
    if(pageFunc!=NULL) {
      char *pageName=pageFunc->getPageName();
      if(!is(pageName)) { pageName="unkow";} 
      sprintf(buffer+strlen(buffer),"page %d %s\n",i,pageName);
    }   
  }
  return buffer;
}

char* pageDel(int index,int end) {
  while(true) {  
    if(index==pageIndex) { pageSet(250); } // del actual => switch off
    pages.del(index);
    if(index<end) { index++; } else { return EMPTY; }
  }  
}

//--------------------------------------------------------
void pageSetup() {
}

void pageLoop() {
  if(!displayEnable && !_displaySetup) { return ; }
  if(pageIndex!=250 && isTimer(pageRefreshTime, pageRefresh)) {

    if(pageFunc!=NULL) { 
      sprintf(buffer,"page setup %d",pageIndex);logPrintln(LOG_DEBUG,buffer);    
      if(pageRefresh==0) { pageFunc->doSetup(); }
      else { pageFunc->doLoop();  }
    }else {
      sprintf(buffer,"no page found %d",pageIndex);logPrintln(LOG_ERROR,buffer);   
    }  
  }
}
