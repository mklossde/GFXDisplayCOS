
// CmdOS 
// dev by OpenOn.org source from http://github.com/mklossde/CmdOS

#ifdef ESP32
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(TARGET_RP2040)
  #include <WiFi.h>
#endif

#include <EEPROM.h>       // EEprom read/write

#include <time.h>         // time 
#include <sys/time.h>     // time

/* cmdOS by michael@OpenON.org */
const char *cmdOS="V.0.4.0";
char *APP_NAME_PREFIX="CmdOs";
 
String appIP="";

/* on init auto find wifi set_up (password set_up) and connect */
#define wifi_setup "set_up"

long freeHeapMax;

//-----------------------------------------------------------------------------


#define EE_MODE_FIRST 0 // First init => EEPROM Wrong
#define EE_MODE_SETUP 1 // EEInit / wifi Setup mode
#define EE_MODE_AP 2 // EEInit / wifi AP mode

#define EE_MODE_WIFI_OFF 10             // OFF 
#define EE_MODE_PRIVAT 20              // Private 
#define EE_MODE_WIFI_TRY 21          // Wifi first client access try 
#define EE_MODE_WIFI_CL_RECONNECT 23    // Wifi is reconnetion (to a Router)

#define EE_MODE_OK 30  // MODE OK

#define EE_MODE_START 40  // MODE START
#define EE_MODE_WRONG 45  // MODE WRONG

#define EE_MODE_ERROR 50  // MODE ERROR , Enable only WIFI
#define EE_MODE_SYSERROR 51  // MODE ERROR , Enable only CMD


//-----------------------------------------------------------------------------
// access

#define ACCESS_ADMIN 1 // admin function 
#define ACCESS_CHANGE 2 // user function 
#define ACCESS_READ 3 // info function 
#define ACCESS_USE 5 // user function 
#define ACCESS_ALL 10 // general function

/* log Level */
#define LOG_SYSTEM 0
#define LOG_ERROR 2
#define LOG_INFO 5
#define LOG_DEBUG 10

/* actual log level */
byte logLevel=LOG_INFO;

/* do have access */
bool isAccess(int requireLevel);

//-----------------------------------------------------------------------------

#define EEBOOTSIZE 500

char eeType[5];
byte eeMode=0;
int eeAppPos=0;

//-----------------------------------------------------------------------------

char* cmdLine(char* prg); // execute one line 
char* cmdPrg(char* prg); // execute a cmd-prg 
char* cmdFile(char* p0); // execute a cmd-file 

//-----------------------------------------------------------------------------
// char utils

#define MAX_DONWLOAD_SIZE 50000

#define bufferMax 256
#define paramBufferMax 128
#define maxInData 128 // max line length

#define minValueLen 8

static char* buffer=new char[bufferMax]; // buffer for char/logging
static char* paramBuffer=new char[paramBufferMax]; // buffer for params
static char inData [maxInData]; // read buffer
static char inIndex = 0; // read index

static char* EMPTY="";
static String EMPTYSTRING="";
static char* UNKOWN="UNKOWN";

//------------------------------------


/* replace all old_car with new_cahr in str 
    e.g. replace(str,' ','+');
*/
void replace(char *str, char old_char, char new_char) {
    if (str == NULL) { return; }
    while (*str != '\0') { // Iterate through the string until the null terminator
        if (*str == old_char) { *str = new_char; } // Replace the character
        str++; // Move to the next character
    }
}

/* return if str ends with find 
    e.g. if(endsWith(str,".gif"))
*/
boolean endsWith(char *str,char *find) {
  if(str==NULL || find==NULL) { return false; }
  int len=strlen(str);
  int findLend=strlen(find);
 return len >= findLend && strcmp(str + len - findLend, find) == 0;
}

/* return if str start with find 
    e.g. if(startWith(str,"/"))
*/
boolean startWith(char *str,char *find) {
  if(!is(str) || !is(find)) { return false; }
  //return strcmp(str, find) == 0;
  int l1=strlen(str); int l2=strlen(find);
  if(l1<l2) { return false; }
  for(int i=0;i<l2;i++) {  if(*str++!=*find++) { return false; } }
  return true;
}



/* validate is cstr equals to find  
    e.g. if(equals(cmd,"stat")) */
boolean equals(char *str,char *find) {
  if(!is(str) || !is(find)) { return false; }
  int l1=strlen(str); int l2=strlen(find); 
  if(l1!=l2) { return false; }
  for(int i=0;i<l2;i++) {        
    if(*str!=*find) { return false; } 
    str++; find++;
  }  
  return true;
}

/* size/len of text  
    int len=size(text);
*/
int size(char *text) { if(text==NULL) { return -1; } else { return strlen(text); }}

/* insert at pos into buffer */
void insert(char* buffer,int pos,char* insertText) {
    size_t insertLen = strlen(insertText);
    size_t len = strlen(buffer);
    size_t newLen = insertLen + len;      
    // Shift existing text to the right
    memmove(buffer + pos + insertLen, buffer + pos , len - pos + 1);  // +1 for null terminator
    // Copy the prefix at the beginning
    memcpy(buffer+pos, insertText, insertLen);
} 

 
/*  validate if chars not NULL  
    e.g. if(is(text))
*/
boolean is(char *p) { return p!=NULL && p!=EMPTY; }
/*  validate if chars have size betwee >=min <max
    e.g. if(is(text,1,32))
*/
boolean is(char *p,int min,int max) { return p!=NULL && strlen(p)>=min && strlen(p)<max; }

boolean is(String str) { return (str!=NULL || str!=EMPTYSTRING); }
boolean is(String str,int min,int max) { if(str==NULL || str==EMPTYSTRING ) { return false; } int len=str.length(); return len>=min && len<max; }

/* convert to correct char */
char* to(byte d) { sprintf(buffer,"%d",d); return buffer; }
char* to(int d) { sprintf(buffer,"%d",d); return buffer; }
char* to(long d) { sprintf(buffer,"%d",d); return buffer; }
char* to(boolean d) { sprintf(buffer,"%d",d); return buffer; }
char* to(double d) { sprintf(buffer,"%.2f",d); return buffer; }
char* to(char *p) {if(p!=NULL && strlen(p)>0 && strlen(p)<bufferMax) { return p; } else { return EMPTY; } }
const char* to(const char *p) {if(p!=NULL && strlen(p)>0 && strlen(p)<bufferMax) { return p; } else { return EMPTY; } }

char* to(char *a,char *b) { sprintf(buffer,"%s%s",to(a),to(b)); return buffer; }
char* to(const char *a, const char *b,const char *c) {  sprintf(buffer,"%s%s%s",to(a),to(b),to(c)); return buffer; }
char* to(const char *a, const char *b,const char *c,const char *d) {  sprintf(buffer,"%s%s%s%s",to(a),to(b),to(c),to(d)); return buffer; }
char* to(const char *a, const char *b,const char *c,const char *d,const char *e) {  sprintf(buffer,"%s%s%s%s%s",to(a),to(b),to(c),to(d),to(e)); return buffer; }

void *toLower(char* str) {
    if(str==NULL) { return NULL; }
    int i=0;
    while(str[i]!='\0') { str[i] = (char)tolower((unsigned char)str[i]); }
}

/* convert cahr* to string */
String toString(const char *text) {  if(!is(text)) { return EMPTYSTRING; } return String(text); }
String toString(char *text) {  if(!is(text)) { return EMPTYSTRING; } return String(text); }

boolean toBoolean(int i) { return i>0; }
/* convert char* to boolean */
boolean toBoolean(char *p) { return p!=NULL && strlen(p)>0 && (strcmp(p, "on")==0 || strcmp(p, "true")==0 || atoi(p)>0); }
/* convert char* to int */
int toInt(char *p) { 
  if(!is(p)) { return -1; }
  else if(isInt(p)) { return atoi(p); } 
  if(toBoolean(p)) { return 1; }else { return 0; } 
} 
/* convert char* to double */
double toDouble(char *p) { 
  if(!is(p)) { return -1; }
  else if(isDouble(p)) { return atof(p); }
  if(toBoolean(p)) { return 1; } else { return 0; } 
}
/* convert char* to long */
long int toLong(char *p) { 
  if(!is(p)) { return -1; }
  else if(isInt(p)) { return atol(p); } 
  if(toBoolean(p)) { return 1; }else { return 0; } 
}
/* convert char* to unsigned long */
unsigned long toULong(char *p) { 
  if(!is(p)) { return -1; }
  else if(isBoolean(p)) {  if(toBoolean(p)) { return 1; }else { return 0; } }
  else { return strtoul(p, NULL, 0); } 
}

boolean isDouble(char *p) {
  char *x=p;
  if (x==NULL || x==EMPTY || *x == '\0') { return false; } // Empty string is not a number
  else if (*x == '+' || *x == '-')  { x++; } // Handle optional sign
  while (*x) { if (!isdigit(*x) && *x!='.') { return false; } else { x++;} }// Non-digit character found
  return true;  // All characters are digits
}

boolean isInt(char *p) {
  char *x=p;
  if (x==NULL || x==EMPTY || *x == '\0') { return false; } // Empty string is not a number
  else if (*x == '+' || *x == '-')  { x++; } // Handle optional sign
  while (*x) { if (!isdigit(*x)) { return false; } else { x++;} }// Non-digit character found
  return true;  // All characters are digits
}

boolean isBoolean(char *p) { 
  char *x=p;
  if (x==NULL || x==EMPTY || *x == '\0') { return false; } // Empty string is not a number
  if(*x=='t' || *x=='T' || *x=='f' || *x=='F') { return true; }
  return false;
}

//-----------------------------------------------------------------------------

/* append text1 and text2 => text1text2 (by use paramBuffer) 
    e.g. char* param=paramAppend("/",file)
*/
char* paramAppend(char *text1,char *text2) {  
  if(size(text1)+size(text2) >= paramBufferMax) { return NULL; }
  paramBuffer[0]= '\0';  
  if(is(text1)) { strcpy(paramBuffer, text1);  }
  if(is(text2)) { strcat(paramBuffer, text2); }
  return paramBuffer;
}

//-----------------------------------------------------------------------------
// ESP Tools

/* reboot esp 
    e.g. espRestart("restart after time")
*/
void espRestart(char* message) {  
  if(serialEnable) { Serial.print("### Rebooting "); Serial.println(message); delay(1000); }
  ESP.restart();
}

/* espChip ID */
uint32_t espChipId() {
  #ifdef ESP32
    uint32_t chipId=0;
    for (int i = 0; i < 17; i = i + 8) { chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;}
    return chipId;
  #elif defined(ESP8266)
    return ESP.getChipId();
  #endif
}

/* esp info */
char* espInfo() {
    sprintf(buffer,"ESP chip:%d free:%d/%d core:%s freq:%d flashChipId:%d flashSize:%d flashSpeed:%d SketchSize:%d FreeSketchSpace:%d",    
      espChipId(),ESP.getFreeHeap(),freeHeapMax, ESP.getSdkVersion(),ESP.getCpuFreqMHz()
      ,ESP.getFlashChipMode(),ESP.getFlashChipSize(),ESP.getFlashChipSpeed(),ESP.getSketchSize(),ESP.getFreeSketchSpace());           
    return buffer;
}

//-----------------------------------------------------------------------------
// Log

void webLogLn(String msg); // define in web
void mqttLog(char *message); // define in mqtt
void telnetLog(char *message); // define in telnet

/* log with level 
    e.g. logPrintln(LOG_INFO,"info"); 
        sprintf(buffer,"name:'%s'",name);logPrintln(LOG_INFO,buffer);  
*/
void logPrintln(int level,const char *text) { 
  if(level>logLevel || !is(text)) { return ; }
  if(serialEnable) { Serial.println(text); } 
  if(telnetEnable) { telnetLog((char*)text); }
  if(webSerialEnable) { webLogLn(toString(text)); }  
  if(mqttLogEnable) { mqttLog((char*)text); }
}

/* log with lvel and string 
    e.g. logPrintln(LOG_DEBUG,"info text");
*/
void logPrintln(int level,String text) {  
  if(level>logLevel || !is(text)) { return ; } 
  const char* log=text.c_str();
  if(serialEnable) { Serial.println(log); } 
  if(telnetEnable) { telnetLog((char*)log); }
  if(webSerialEnable) { webLogLn(text); }
  if(mqttLogEnable) { mqttLog((char*)log); }  
}

/* set actual logLevel - log this level and above
    e.g. setLogLevel(LOG_DEBUG) 
*/
char* setLogLevel(int level) {
  if(level>=0) { logLevel=level; }
  sprintf(buffer,"%d",logLevel); return buffer;
}



//-------------------------------------------------------------------------------------------------------------------
//  MapList



/* list of object and map of key=value */
class MapList {
private:
  int _index=0; int _max=0;
  void** _array=NULL; // contains values 
  char** _key=NULL; // contains keys
  int* _vsize=NULL; // contains alloc-size of each value
  boolean _isMap=false;

  void grow(int grow) {
    _max+=grow; 
    if(_array==NULL) {          
//      _array = (void**) malloc(_max * sizeof(void*));
      _array = (void**)malloc(_max * sizeof(void*));
      if(_isMap) { _key = (char**) malloc(_max * sizeof(char*)); }
      _vsize = (int*) malloc(_max * sizeof(int));
  }else {
      _array = (void**)realloc(_array, _max * sizeof(void*)); 
      if(_isMap) { _key = (char**)realloc(_key, _max * sizeof(char*)); }    
      _vsize = (int*)realloc(_vsize, _max * sizeof(int));
    }
  }
  void growTo(int max,void *obj) {
    for(int i=_max;i<max;i++) { grow(1);_array[_max-1]=obj; }
  }

public:
  // map
  /* set by copy key and value and replace value on change */
  void replace(char *key,char *obj,int len) {
    if(!is(key)) { return ; } 
    int index=find(key);
    if(index==-1) {
      if(_index>=_max) { grow(1); }       
      int size=len; if(size<minValueLen) { size=minValueLen; }
      char* to=new char[size+1]; if(to==NULL) { espRestart("replace() memory error"); }
      if(len>0) { memcpy( to, obj, len); } 
      to[len]='\0'; 
      _key[_index]=copy(key); _array[_index]=to; _vsize[_index]=size;      
      sprintf(buffer,"set %d '%s'='%s' len:%d size:%d",_index,key,to,len,size); logPrintln(LOG_DEBUG,buffer);
      _index++;
    }else {
      void* old=(void*)_array[index];
      int oldSize=_vsize[index];
      if(oldSize<=len) {
        _array[index] = (void*)realloc(_array[index], len+1); if( _array[index]==NULL) { espRestart("map-replace memory error"); }
        _vsize[index]=len;        
      }         
      char* o=(char*)_array[index]; 
      if(len>0) { memcpy(o, obj, len); } o[len]='\0';
      sprintf(buffer,"replace '%s'='%s' len:%d oldSize:%d",key,o,len,oldSize); logPrintln(LOG_DEBUG,buffer);
    }
  }
  
  /* set key=value into list e.g. list.set("key",value); */
  void* set(char *key,void *obj) {  
    int index=find(key);   
    if(index>=0) { void* old=_array[index]; _array[index]=obj; return old; } // overwrite 
    else {
      if(_index>=_max) { grow(1); } 
      char *k=copy(key); _key[_index]=k; _array[_index]=obj; _index++;        
      return NULL;
    }
  }  
  /* get key at index e.g. char *key=list.key(0); */
  char* key(int index) { if(index>=0 && index<_index) { return _key[index]; } else { return NULL; } }  
  /* get value with key e.g. char *value=(char*)list.get(key); */
  void* get(char *key) {  
    if(!is(key)) { return NULL; }
    for(int i=0;i<_index;i++) {  if(equals(_key[i],key)) { return _array[i]; } } 
    return NULL;
  } 
  /* del key=value e.g. char* old=list.del(key); */
  boolean del(char *key) { 
    if(!is(key)) { return false; }
    int index=find(key); if(index==-1) { return false; }
    del(index);        
    return true;
  }  
  /* find index of key e.g. int index=list.find(key); */
  int find(char *key) { 
    if(!is(key)) { return -1; }
    for(int i=0;i<_index;i++) {  if(equals(_key[i],key)) { return i; } } return -1; }

  // list ------------------------------------------------
  /* add object to list e.g. list.add(obj); */
  void add(void *obj) { if(_index>=_max) { grow(1); } _array[_index++]=obj; } 
  void addIndex(int index,void *obj) { 
    if(index>=_max) { growTo(index+1,NULL); }     
    _array[index]=obj; if(index>=_index) { _index=index+1; } } 
  /* get obejct at index e.g. char* value=(char*)list.get(0); */
  void* get(int index) { if(index>=0 && index<_index) { return _array[index]; } else { return NULL; } }  
  /* del object at index e.g. char* old=(char*)list.del(0); */
  boolean del(int index) {   
    if(index<0 || index>=_index) {return false; }      
    void *obj=_array[index]; if(obj!=NULL) { delete obj; } 
    if(_isMap) { void *oldKey=_key[index]; if(oldKey!=NULL) { delete oldKey; } }        
    for(int i=index;i<_index-1;i++) { 
      _array[i]=_array[i+1]; 
      if(_isMap) { _key[i]=_key[i+1]; }
      _vsize[i]=_vsize[i+1];
    } 
    _index--; 
    return true;    
  }
  /* clear all (without prefix) / clear with prefix (e.g. clear my ) */
  void clear(char *prefix) { for(int i=_index;i>=0;i--) { if(!is(prefix) || startWith(key(i),prefix)) { del(i); }} }

  /* size of list e.g. int size=list.size(); */
  int size() { return _index; }
  MapList(int max) {  grow(max); }
  MapList() {   }
  MapList(boolean isMap) {  _isMap=isMap;  } // enable as map
  ~MapList() { delete _array; if(_isMap) { delete _key; } }

};


//-----------------------------------------------------------------------------
// Memory

// new [] => delete[]
// NEW => delete
// malloc() or calloc() => free

char* newChar(int len) {
  return new char[len];
}

void* newMalloc(int size) {
  return (void*)malloc(size);
}

/* buffered new char of len , by use oldchar (buffer first:\0 end of char, second:\0 end of buffer) */
char* newChar(char *oldChar,int len) {
  if(oldChar!=NULL) { 
    int blen=newCharLen(oldChar);
    if(blen>len) { 
      for(int i=0;i<blen-1;i++) { oldChar[i]=' ';}  oldChar[len-1]='\0';  
      return oldChar; }
    else { delete oldChar; }
  }
  int size=len; if(len<minValueLen) { size=minValueLen; }
  char *nc= new char[size+1]; 
  for(int i=0;i<size-1;i++) { nc[i]=' ';} nc[len-1]='\0'; nc[size]='\0'; // double \0
  return nc;
}

/* buffered copy char (buffer first:\0 end of char, second:\0 end of buffer) */
char* newChar(char *oldChar,char *str) {
  if(str==NULL) { if(oldChar!=NULL) { oldChar[0]='\0'; } return oldChar; }
  int len=strlen(str);
  oldChar=newChar(oldChar,len+1);
  memcpy(oldChar, str, len); //to[len]='\0';   
  return oldChar;
}  

/* buffered char len */
int newCharLen(char *oldChar) {
  if(oldChar==NULL) { return 0; }
  byte count=0; int blen=0; while(count<2) { if(oldChar[blen++]=='\0') { count++; } }
  return blen; 
}

//-----------------------------------------------------------------------------
// dynamic memory

/* concat char to new char*, use NULL as END, (e.g char *res=concat("one","two",NULL); ), dont forget to free(res); */
char* concat(char* first, ...) {
    size_t total_len = 0;

    va_list args;
    va_start(args, first);
    size_t l=0;
    for (char* s = first; s != NULL && (l=strlen(s))>0; s = va_arg(args, char*)) {  total_len += l;  }
    va_end(args);

    char *result = (char*)malloc(sizeof(char) *(total_len + 1)); // +1 for null terminator
    if (!result) return NULL;
    result[0] = '\0'; // initialize empty string

    va_start(args, first);
    for (char* s = first; s != NULL; s = va_arg(args, char*)) { strcat(result, s); }
    va_end(args);
    return result;
}

/* copy org* to new (NEW CHAR[]
    e.g. char* n=copy(old); 
*/
char* copy(char* org) { 
  if(org==NULL) { return NULL; }
  int len=strlen(org);
  char* newStr=newChar(len+1); 
  memcpy( newStr, org, len); newStr[len]='\0'; 
  return newStr;
}

/* create a copy of org with new char[max] (NEW CHAR[])*/
char* copy(char *to,char* org,int max) { 
  if(to==NULL) { to=newChar(max+1); }
  if(to==NULL) { espRestart("copy() memory error"); }
  if(org!=NULL) { 
    int len=strlen(org); if(len>max) { len=max; }
    memcpy( to, org, len); to[len]='\0'; 
  }else { to[0]='\0'; }
  return to;
}

/* copy (MALLOC) */
char* copy(char *to,String str,int max) { 
  if(to==NULL) { to = (char*)malloc((max + 1)*sizeof(char));  }     
  if(to==NULL) { espRestart("copy() memory error"); }
  if(str!=NULL) {         
//TODO take care on string len    
    strcpy(to, str.c_str()); 
  }
  return to;
}

char* copy(String str) {  
  if(str==NULL || str==EMPTYSTRING) { return NULL; } 
  char* s = (char*)malloc(str.length() + 1); 
  if(s==NULL) { espRestart("to() memory error"); }
  strcpy(s, str.c_str());
  return s;
}
char* copy(String str,char* def) {  
  if(str==NULL || str==EMPTYSTRING) { return def; } 
  int len  =str.length()+1; if(len==0) { return def; } char ca[len]; str.toCharArray(ca,len); return(ca);
}

//------------------------------------------------------------------

/* extract from src (NEW char[]) (e.g. is=extract(".",":","This.is:new") )*/
char* extract(char *start, char *end, char *src) {
    const char *start_ptr=src;
    if(is(start)) {  // find start if given
      start_ptr = strstr(src, start); 
      if (!start_ptr) { return NULL; } 
      else { start_ptr += strlen(start); }  // Move past 'start'
    }      
    size_t len = 0;
    if(is(end)) { 
      const char *end_ptr = strstr(start_ptr, end); if (!end_ptr) { return NULL; }
      len = end_ptr - start_ptr; 
    }else  {
      len=strlen(start_ptr);
    }
    char *result=newChar(len+1);
    strncpy(result, start_ptr, len);  result[len] = '\0';  
    return result;
}


//-----------------------------------------------------------------------------
// FILESYSTEM SPIFFS / LittleFS

#if enableFs    
  #ifdef ESP32  
    #include <SPIFFS.h>  
    #define FILESYSTEM SPIFFS
  #elif defined(ESP8266)
//    #include <SPIFFS.h>
//    #define FILESYSTEM U_FS
    #include <LittleFS.h>  
    #define FILESYSTEM LittleFS
  #endif

  String rootDir="/";

  /* delete file in SPIFFS [ADMINI] */ 
  boolean fsDelete(String file) {     
    if(!is(file)) { return false; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; } 
    boolean ok=FILESYSTEM.remove(file);  
    sprintf(buffer,"fsDel '%s' ok:%d",file.c_str(),ok);logPrintln(LOG_INFO,buffer);  
    return true;
  }
  boolean fsRename(String oldFile,String newFile) { 
    if(!is(oldFile) || !is(newFile)) { return false; }
    else if(!oldFile.startsWith(rootDir)) { oldFile=rootDir+oldFile; } 
    else if(!newFile.startsWith(rootDir)) { newFile=rootDir+newFile; } 
    boolean ok=FILESYSTEM.rename(oldFile,newFile);  
    sprintf(buffer,"fsRename '%s' => '%s' OK:%d",oldFile.c_str(),newFile.c_str(),ok);logPrintln(LOG_INFO,buffer);  
    return ok;
  }
  
  /* create SPIFFS file and write p1 into file */
  boolean fsWrite(String file,char *p1) { 
    if(!is(file)) { return false; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; }
    File ff = FILESYSTEM.open(file, "w");
    if(!ff){ return false; }
    int len=strlen(p1);
    if(p1!=NULL && len>0) { ff.print(p1); }
    ff.close();
    sprintf(buffer,"fsWrite '%s %d",file.c_str(),len);logPrintln(LOG_INFO,buffer); 
    return true;
  }

  /* create SPIFFS file and write p1 into file */
  boolean fsWriteBin(String file,uint8_t *p1,int len) { 
    if(!is(file)) { return false; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; }    
    File ff = FILESYSTEM.open(file, "w");
    if(!ff){ return false; }
    ff.write(p1,len);
    ff.close();
    sprintf(buffer,"fsWriteBin '%s %d",file,len);logPrintln(LOG_INFO,buffer); 
    return true;
  }


  /* read file as char array 
        char *data;  
        data = fsRead(name); 
        delete[] data;
  */
  char* fsRead(String file) {  
    if(!is(file)) { return NULL; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; }
    File ff = FILESYSTEM.open(file, "r");  
    if(ff==NULL) { sprintf(buffer,"fsRead unkown '%s'",file.c_str());logPrintln(LOG_INFO,buffer);   return NULL; } 
    size_t fileSize= ff.size();

    char *charArray = newChar(fileSize + 1);
    ff.readBytes(charArray, fileSize);
    charArray[fileSize] = '\0';
    ff.close();

    sprintf(buffer,"fsRead '%s' %d",file.c_str(),fileSize);logPrintln(LOG_INFO,buffer);  
    return charArray;
  }


  /* read file as bin 
        size_t dataSize = 0; // gif data size
        uint8_t *data = fsReadBin(name, dataSize); 
        delete[] data;
*/
  uint8_t* fsReadBin(String file, size_t& fileSize) {
    if(!is(file)) { return NULL; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; }
    File ff = FILESYSTEM.open(file, "r");  
    if(ff==NULL) { sprintf(buffer,"fsReadBin unkown '%s'",file.c_str());logPrintln(LOG_INFO,buffer);   return NULL; } 
    fileSize= ff.size();

    uint8_t *byteArray = new uint8_t[fileSize];
    ff.read(byteArray, fileSize);

    ff.close();
    sprintf(buffer,"fsReadBin '%s' %d",file.c_str(),fileSize);logPrintln(LOG_INFO,buffer);  
    return byteArray;
  }


  int fsSize(String file) { 
    if(!is(file)) { return -1; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; }
    File ff = FILESYSTEM.open(file,"r");
    if(ff==NULL) { return -1; } 
    int len=ff.size();
    ff.close();
    return len;
  }

  /* show a file */
  void fsCat(String file) { 
    if(!is(file)) { return ; }
    else if(!file.startsWith(rootDir)) { file=rootDir+file; }
    File ff = FILESYSTEM.open(file, "r");
    if(ff==NULL) { logPrintln(LOG_INFO,"missing");  } 
    char buffer[50];
    while (ff.available()) {
      int l = ff.readBytes(buffer, sizeof(buffer));
      buffer[l] = '\0';  
      logPrintln(LOG_INFO,buffer);
    }
    ff.close();
  }

  /* list files in SPIFFS of dir (null=/) */
  char* fsDir(String find) {
    if(!isAccess(ACCESS_READ))  { return "NO ACCESS fsDir"; }
    sprintf(buffer,"Files:\n");
    File root = FILESYSTEM.open(rootDir,"r");
    File foundfile = root.openNextFile();
    while (foundfile) {
      String file=foundfile.name();
      if(!is(find) || file.indexOf(find)!=-1) { 
        sprintf(buffer+strlen(buffer),"%s (%d)\n",file,foundfile.size());        
      }
      foundfile = root.openNextFile();
    }
    root.close();
    foundfile.close();
    return buffer; 
  }

  /* list number of files in SPIFFS of dir (null=/) */
  int fsDirSize(String find) {
    int count=0;
    File root = FILESYSTEM.open(rootDir,"r");
    File foundfile = root.openNextFile();
    while (foundfile) {
      String file=foundfile.name();
      if(!is(find) || file.indexOf(find)!=-1) { count++; }
      foundfile = root.openNextFile();
    }
    root.close();
    foundfile.close();
    return count; 
  }

  /* get file-name match filter, in dir at index (e.g. .gif,0 => first gif-file) 
      type<=0 => name of file
      type=1 => size of file
  */
  char* fsFile(String find,int count,int type) {
    File root = FILESYSTEM.open(rootDir,"r");
    File foundfile = root.openNextFile();
    while (foundfile) {
      String file=foundfile.name();
      if(!is(find) || file.indexOf(find)!=-1) { 
        if(count--<=0) { 
          if(type<=0) { sprintf(buffer,"%s",(char*)file.c_str()); return buffer;  }
          else if(type==1) { sprintf(buffer,"%d",foundfile.size()); return buffer;  }
          else { return "unkown type"; }
        }
      }
      foundfile = root.openNextFile();
    }
    root.close();
    foundfile.close();
    return EMPTY; 
  }

  /* format SPIFFS */
  void fsFormat() {
    sprintf(buffer,"FS formating..."); logPrintln(LOG_DEBUG,buffer); 
    if (SPIFFS.format()) { sprintf(buffer,"FS format DONE"); logPrintln(LOG_SYSTEM,buffer);  }
    else { sprintf(buffer,"FS format FAILD"); logPrintln(LOG_ERROR,buffer); }    
  }

  #if netEnable
    #include <WiFiClient.h>
  #ifdef ESP32
    #include <HTTPClient.h>
  #elif defined(ESP8266)
    #include <ESP8266HTTPClient.h>    
  #endif

    // e.g. https://www.w3.org/Icons/64x64/home.gif
    char* fsDownload(String url,String name,int reload) {
      if(!is(url,0,250)) { return "missing url"; }

      HTTPClient http;
      if(name==NULL) { name=url.substring(url.lastIndexOf('/')); }
      if(!name.startsWith("/")) { name="/"+name; }

      // check redownload 
      if(fsSize(name)>0) {
        if(reload==-1) { sprintf(buffer,"download foundOld '%s'",name); return buffer; }
      }

      #ifdef ESP32
        http.begin(url); 
      #elif defined(ESP8266)
        WiFiClient client;
        http.begin(client,url); 
      #endif      
      
      int httpCode = http.GET();
      int size = http.getSize();
      if(size>MAX_DONWLOAD_SIZE) { http.end(); return "download maxSize error"; }

      FILESYSTEM.remove(name);  // remove old file
      if (httpCode == 200) {
        sprintf(buffer,"fs downloading '%s' size %d to '%s'", url.c_str(), size,name.c_str());logPrintln(LOG_INFO,buffer);
        File ff = FILESYSTEM.open(name, "w"); 
        http.writeToStream(&ff);
        ff.close();

        sprintf(buffer,"fs download '%s' size %d to '%s'", url.c_str(), size,name.c_str());logPrintln(LOG_INFO,buffer);
        http.end();
        return "download ok";
      } else {
        sprintf(buffer,"fs download '%s' error %d",name.c_str(),httpCode);logPrintln(LOG_INFO,buffer);
        http.end();
        return "download error";
      }

    } 

    /* do rest call and return result */
    char* rest(String url) {
      if(!isAccess(ACCESS_READ))  { return "NO ACCESS"; }
      if(!is(url,0,250)) { return "missing url"; }

      HTTPClient http;
      #ifdef ESP32
        http.begin(url); 
      #elif defined(ESP8266)
        WiFiClient client;
        http.begin(client,url); 
      #endif    
      int httpCode = http.GET();

      if (httpCode == 200) {
        int size = http.getSize();
        if(size>bufferMax-1) { http.end(); return "response size error"; }

        String payload = http.getString();
        http.end();
        return (char*)payload.c_str();

      } else {
        sprintf(buffer,"rest '%s' error %d",url.c_str(),httpCode);
        http.end();
        return buffer;
      }  
    }

  #else 
    char* fsDownload(String url,String name,int reload) { return EMPTY; }
    char* rest(String url) { return EMPTY; }  
  #endif


//----------------------------------------------

String fsToSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

/* filesystem setup */
void fsSetup() {
  if(!enableFs) { return ; }
  #ifdef ESP32
    if (!FILESYSTEM.begin(true)) {    // if you have not used SPIFFS before on a ESP32, it will show this error. after a reboot SPIFFS will be configured and will happily work.
      espRestart("FILESYSTEM ERROR: Cannot mount");
    }
  #endif
  if(!FILESYSTEM.begin()){
    logPrintln(LOG_SYSTEM,"FILESYSTEM Mount Failed");
  } else {
    #ifdef ESP32
      sprintf(buffer,"SPIFFS Free:%s Used:%s Total:%s",
        fsToSize((FILESYSTEM.totalBytes() - FILESYSTEM.usedBytes())),fsToSize(FILESYSTEM.usedBytes()),fsToSize(FILESYSTEM.totalBytes()));logPrintln(LOG_INFO,buffer);
    #else
//TODO show nonne spiff fs ?     
      sprintf(buffer,"FS");logPrintln(LOG_INFO,buffer);
    #endif
  }
}

#else
  boolean fsDelete(String file) { return false; }
  boolean fsWrite(String file,char *p1) { return false; }
  boolean fsRename(String oldFile,String newFile) { return false; }
  char* fsRead(String file) { return NULL; }
  int8_t* fsReadBin(String file, size_t& fileSize) { return NULL; }
  int fsSize(String file) { return -1; }
  void fsCat(String file) {}
  char* fsDir(String find) { return "fs not implemented";}  
  int fsDirSize(String find) { return 0; }
  char* fsDownload(String url,String name,int reaload) { return "fs not implemented"; }
  char* rest(String url) { return "fs not implemented"; }  
  char* fsToSize(const size_t bytes) { return "fs not implemented"; }  
  void fsSetup() {}
  void fsFormat() {}
  char* fsFile(String find,int count,int type) { return EMPTY; }
  
#endif


//-----------------------------------------------------------------------------
// Time

unsigned long _timeMs;           // actual time in mills (set by loop)
time_t timeNow;                     // e.g. ntp time
tm tm;                              // the structure tm holds time information in a more convient way
char *ntpServer="unkown";           // actual ntp server
boolean ntpRunning=false;           // is ntpServer running

/* periodical/interval timing 
 nextTime=nexctTime in ms: 
    nextTime=0 => startNow, 
    nextTime=1 => start period,
    nextTime=2 => OFF
    nextTime=0 => off, nextTime=executeTime
 period= next period in ms
    period=-1 (one time, but off after)
 e.g.
    unsigned long *wifiTime = new unsigned long(0);
    if (isTimer(wifiTime, 1000)) {....}
*/
boolean isTimer(unsigned long *lastTime, unsigned long period) {
  if(*lastTime==2 ) { return false; } // lastTime=2 => OFF
  else if(*lastTime==0) {  // do now
    *lastTime=_timeMs; // 0= start now
    if(*lastTime>=0 && *lastTime<2) { *lastTime=3; }
    return true;
  }else if(*lastTime==1) {  
    *lastTime=_timeMs; // 1= start next period
    if(*lastTime>=0 && *lastTime<2) { *lastTime=3; }
    return false;  
  }else if (period!=-1 && _timeMs >= *lastTime+period) {  // next period found
    *lastTime = _timeMs; 
      if(*lastTime>=0 && *lastTime<2) { *lastTime=3; }
    return true;
  } else {  return false; }// not yet
}


/* show state of intervall timer */
char* timerShow(char *info,unsigned long *lastTime, unsigned long period) {
  sprintf(buffer,"%s now:%d timer:%d period:%d next:%d",info,_timeMs,*lastTime,period,(*lastTime+period-_timeMs));
  return buffer;
}

//---------------------------

/* get date as char (e.g. char* date=getDate(); "01.01.2025" ) */
char* getDate() { sprintf(buffer,"%02d.%02d.%04d",tm.tm_mday,tm.tm_mon + 1,tm.tm_year + 1900);  return buffer;  }
/* getTime  as char* (e.g. char* time=getTime(); 13:50) */
char* getTime() { sprintf(buffer,"%02d:%02d:%02d",tm.tm_hour,tm.tm_min,tm.tm_sec);  return buffer; }
/* get actual time in ms (without NTP get ms since startup) */
unsigned long timeMs() { if(timeNow>0) { return timeNow*1000; } else { return _timeMs; } }
/* get actual time in seconds (without NTP get ms since startup) */
unsigned long timeSec() { if(timeNow>0) { return timeNow; } else { return _timeMs/1000;} }

/* get time info */
char* timeInfo() {
//  long t=(tm.tm_hour*3600+tm.tm_min*60+tm.tm_sec);
  if(is(ntpServer)) { 
    sprintf(buffer,"TIME ntpServer:%s time:%d.%d.%d %d %d:%d:%d timeNow:%d timeMs:%lu",
      ntpServer,tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_wday, tm.tm_hour, tm.tm_min, tm.tm_sec,timeNow,_timeMs); 
  }else {
    sprintf(buffer,"TIME up:%lu ",_timeMs); 
  }

  return buffer;  
}

//---------------------------------------------------------------
// EventTimer 
// instance new Time(firstTimeInMs,nextTimeInMs,executeEvent) 
//  e.g. EventTimer *myTimer=new EventTimer(10000,2000,&info); myTimer->start();  // after 10s excute info() and repeat every 2s
//

MapList eventList;

class MyEventTimer {
  private:
    boolean _on=false;
    byte _sec; byte _min; byte _hour; byte _day; byte _wday; byte _month;
    char *_cmd;
    byte _lastSec=255;
  public:
    void start() { _on=true; }
    void stop() { _on=false; }
    boolean _isTime() {      
      //tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_wday, tm.tm_hour, tm.tm_min, tm.tm_sec
      if(_lastSec==tm.tm_sec) { return false; } else { _lastSec=tm.tm_sec; } // take care on double execute in one second              
      if(_sec<60 && _sec!=tm.tm_sec){ return false; }
      if(_min<60 && _min!=tm.tm_min){ return false; }
      if(_hour<60 && _hour!=tm.tm_hour){ return false; }
      if(_day>0 && _day<60 && _day!=tm.tm_mday){ return false; }
      if(_wday>0 && _wday<60 && _wday!=tm.tm_wday){ return false; }
      if(_month>0 && _month<60 && _month!=(tm.tm_mon+1)){ return false; }
      return true;
    }
    void event() { logPrintln(LOG_DEBUG,"event start"); String ret=cmdLine(_cmd); logPrintln(LOG_INFO,ret); }
    void loop() { if(_on && _isTime()) { event(); } }  
    char* info() { sprintf(buffer,"timer %d %d %d %d %d %d %d \"%s\"",_on,_sec,_min,_hour,_day,_wday,_month,_cmd); return buffer;}

    MyEventTimer(boolean on,byte sec,byte min,byte hour,byte wday,byte day,byte month,char *cmd) {
      _on=on;
      _sec=sec; _min=min;_hour=hour; _wday=wday; _day=day; _month=month;
      _cmd=cmd;
    }
     ~MyEventTimer() { if(_cmd!=NULL)  { free(_cmd); } }
};

/* 
  timer 1 -1 -1 -1 -1 -1 -1 "stat"
*/
void timerAdd(boolean on,byte sec,byte min,byte hour,byte wday,byte day,byte month,char *cmd) {
  MyEventTimer* et=new MyEventTimer(on,sec,min,hour,wday,day,month,copy(cmd));
  eventList.add(et);
}

void timerDel(int index) { 
  eventList.del(index);  
}

int timerSize() { return eventList.size(); } 

/* show all timers */
void timerLog() {
  for(int i=0;i<eventList.size();i++) {
    MyEventTimer* timer=(MyEventTimer*)eventList.get(i); 
    logPrintln(LOG_INFO,timer->info());
  }
}

/* loop all timers */
void timerLoop() { 
  for(int i=0;i<eventList.size();i++) {
//    MyEventTimer* timer=timerGet(i);
    MyEventTimer* timer=(MyEventTimer*)eventList.get(i); 
    timer->loop();
  }
}

//-----------------------------------------------------------------

/* ntp intervall timer */
unsigned long *ntpTimer = new unsigned long(0);

/* time setup */
void timeSteup() {
  _timeMs = millis(); // set actual millis
}

/* time loop */
void timeLoop() {
  _timeMs = millis(); // set actual millis
  if(isTimer(ntpTimer, 1000)) { // every second
    if(ntpRunning && ntpEnable)  { 
      time(&timeNow);                  // read the current time    
      localtime_r(&timeNow, &tm);      // update the structure tm with the current time
    }
    timerLoop();
  }
}


//------------------------------------------------------------
// Attr

MapList attrMap(true); 

/* remove '$' from key */
char* _toAttr(char *key) {
  if(!is(key)) { return EMPTY; } else if(*key=='$') { key++; }
  return key;
}

boolean attrHave(char *key) { return attrMap.find(_toAttr(key))!=-1; }
char* attrGet(char *key) {  return (char*)attrMap.get(_toAttr(key)); }
void attrSet(char *key,String value) { 
  char *v=(char*)value.c_str(); 
  attrMap.replace(_toAttr(key),v,strlen(v)); 
} 
void attrSet(char *key,char *value) { attrMap.replace(_toAttr(key),value,strlen(value));  }
void attrDel(char *key) { attrMap.del(_toAttr(key)); }
char* attrInfo() {
  sprintf(buffer,"attrs:\n");
  for(int i=0;i<attrMap.size();i++) {  
    char *key=attrMap.key(i);
    char *value=(char*)attrMap.get(i);
    if(is(key) && is(value)) {
      sprintf(buffer+strlen(buffer),"attr %s \"%s\"\n",key,value);
    }
  }
  return buffer;
}
void attrClear(char *prefix) { attrMap.clear(prefix); }

//------------------------------------------------------------
// Sys

/* get sys attribute */
char* sysAttr(char *name) {
  int d=0; char* s;
  if(!is(name)) { return EMPTY; }
  else if(equals(name,"timestamp")) { d=_timeMs;  }
  else if(equals(name,"time")) { s=getTime();  }
  else if(equals(name,"date")) { s=getDate(); }
  else if(equals(name,"ip")) { s=(char*)appIP.c_str(); }
  else if(equals(name, "freeHeap")) { d=ESP.getFreeHeap(); }// show free heap
  else if(equals(name, "freeHeapMax")) { d=freeHeapMax; }// show free heap  
  else { return EMPTY; }

  if(is(s)) { sprintf(paramBuffer,"%s",s); } else { sprintf(paramBuffer,"%d",d); }
  return paramBuffer;   
}

//------------------------------------------------------------
// Params


MapList appParams(true);

class AppParam {
  private:
    char *_name;
    char *_remote;
  public:
    boolean _change=false;
    byte _type=0;

    char* name() { return _name; }
    char* remote() { return _remote; }
    boolean is(char *remote) { return equals(remote,_remote); }
    char* get() { return (char*)attrGet(_name); }
    void set(char *value) { attrSet(_name,value);_change=true; }

    AppParam(char *name,char *remote,byte type) { 
        _name=copy(name); _remote=copy(remote);
        _type=type;
    }
    ~AppParam() { free(_name);  free(_remote); }
};

//------

char* paramsInfo() {
  sprintf(buffer,"params:\n");
  for(int i=0;i<appParams.size();i++) {      
    char *key=appParams.key(i);
    AppParam *param=(AppParam*)appParams.get(i);
    sprintf(buffer+strlen(buffer),"param %s remote:%s type:%d change:%d \n",param->name(),param->remote(),param->_type,param->_change);
  }
  return buffer;
}

boolean paramSet(char *remote,char *value) {
    for(int i=0;i<appParams.size();i++) {  
    AppParam *p=(AppParam*)appParams.get(i);
    if(p->is(remote)) { 
      p->set(value);
      return true;
    }
  }
  return false;
}

int paramsFind(char *remote) {
  for(int i=0;i<appParams.size();i++) {  
    AppParam *p=(AppParam*)appParams.get(i);
    if(p->is(remote)) { return i; }
  }
  return NULL;
}

void* paramsGet(char *name) {
  return appParams.get(name);
}

//------

char* paramRemote(char *name) {
  AppParam *param=(AppParam*)appParams.get(name);
  if(param!=NULL) { return param->remote(); } else { return NULL; }
}

boolean paramsDel(char *name) {
  AppParam *param=(AppParam*)appParams.get(name);
  if(param==NULL) { return false; }
  appParams.del(name);
  return true;
}


boolean paramsAdd(char *name,char *remote,byte type) {
  AppParam *p=new AppParam(name,remote,type);
  appParams.set(name,p);  // add new param
  return true;
}

/*
boolean paramsAdd(AppParam *p) {
  if(paramGet(p->name)!=NULL) { return false; }
  appParams.add(p->name(),p);  // add new param
  return true;
}
*/

//------------------------------------------------------------

void paramSkipSpace(char **pp) {
    if(pp==NULL || *pp==NULL || **pp=='\0') { return ; }    
    while(**pp==' ' || **pp=='\t') { (*pp)++; } // skip spaces and tabs    
}

void paramTrim(char **pp) {
  if(pp==NULL) { return ; }
  paramSkipSpace(pp);
  int i=strlen(*pp); while(i>=0 && **pp==' ') { i--; }
  *pp[i]='\0';  
}

/* read next param */
char* paramNext(char **pp,char *split) {
    paramSkipSpace(pp);
    if(pp==NULL || *pp==NULL || **pp=='\0') { return NULL; }

    char* p1;    
    if(**pp=='"') { // read string "param"
      (*pp)++; // skip first "
      p1 = strtok_r(NULL, "\"",pp);  
      if(p1==NULL) { return EMPTY; }   
      return p1;   

    }else { 
      p1 = strtok_r(NULL, split,pp);            
    }

    return p1;
}


//-------------------------------------------------------------------------------------------------------------------
// LED

// BOOT SWITCH: BOOT=>BLINK 2x=>PRESS 2s=>WIFI_AP / 5s=>clear

#if ledEnable
int ledPatternFlashSlow[] = { 5, 500, 0 };      // 1 Flash slow => Wifi SP mode
int ledPattern2Flash[] = { 5, 10, 5, 500, 0 };  // 2 Flash slow => Wifi CL connectiong
int ledPatternBlink1[] = { 10, 1, 0 };          //

unsigned long *ledTime = new unsigned long(0);  // led timer
boolean ledBlinkOn = false;                     // is blink on
boolean ledOn = false;                          // is led on
int (*ledPattern)[];                            // led blink pattern
int ledTicks = 0;
byte ledCount = 5;
byte ledIndex = 0;

// direct blink blinkSpeed in ms - just for test/debug/error
void ledBlink(byte times, int blinkSpeed) {
  if (times * blinkSpeed > 10000) { return; }  // ignore delay >10s
  if (ledEnable) {
    for (int i = 0; i < times; i++) {
      digitalWrite(ledGpio, ledOnTrue);
      delay(blinkSpeed);
      digitalWrite(ledGpio, !ledOnTrue);
      delay(blinkSpeed);
      ledOn = !ledOnTrue;
    }
  }
}
void ledSet(boolean on) {
  digitalWrite(ledGpio, on);
  ledOn = on;
}

void ledOff() {
  if (ledEnable) {
    digitalWrite(ledGpio, !ledOnTrue);
    ledOn = !ledOnTrue;
  }
  ledBlinkOn = false;
}

//-----

// show actual led blink
void ledShow() {
  if (ledEnable) {
    if (ledOn) {
      digitalWrite(ledGpio, ledOnTrue);
    } else {
      digitalWrite(ledGpio, !ledOnTrue);
    }
  }
}

// blink with pattern, max times (0=unlimited)
void ledBlinkPattern(byte max, int (*blinkPattern)[]) {
  ledPattern = blinkPattern;
  ledCount = max;
  ledTicks = 0;
  ledIndex = 0;
  ledBlinkOn = true;
  ledOn = true;
  ledShow();
  //  sprintf(buffer,"LED blink %d index:%d time:%d count:%d",ledGpio,ledIndex,(*ledPattern)[ledIndex],ledCount); logPrintln(buffer);
}

//--------------------------

void ledSetup() {
  if (!ledEnable) { return; }
  pinMode(ledGpio, OUTPUT);
  sprintf(buffer, "LED setup gpio:%d on:%d", ledGpio, ledOnTrue);
  logPrintln(LOG_INFO, buffer);
}

void ledLoop() {
  if (!ledEnable) { return; }
  if (!ledBlinkOn || !isTimer(ledTime, 10)) { return; }  // every 10ms

  if (!ledBlinkOn || (*ledPattern)[0] == 0) { return; }
  ledTicks++;
  if (ledTicks > (*ledPattern)[ledIndex]) {
    ledOn = !ledOn;
    ledTicks = 0;
    ledIndex++;
    if ((*ledPattern)[ledIndex] == 0) {
      ledIndex = 0;
      if (ledCount > 0) {
        ledCount--;
        if (ledCount == 0) {
          ledBlinkOn = false;
          ledOn = false;
        }
      }
    }
    ledShow();
  }
}


char *ledInit(int pin, boolean on) {
  if (pin != -1) {
    ledGpio = pin;
    ledOnTrue = on;
    ledSetup();
  }
  sprintf(buffer, "led pin:%d on:%d", ledGpio, ledOnTrue);
  return buffer;
}

char *ledSwitch(char *c, char *c2) {
  if (isBoolean(c)) {
    boolean b = toBoolean(c);
    ledSet(b);
    sprintf(buffer, "%d", b);
    return buffer;
  }
  // ledBlink time speed
  else if (isInt(c)) {
    int b = toInt(c);
    int b2 = toInt(c2);
    ledBlink(b, b2);
    sprintf(buffer, "%d %d", b, b2);
    return buffer;
  } else {
    return EMPTY;
  }
}

#else

void ledBlink(byte times, int blinkSpeed) {}
void ledBlinkPattern(byte max, int (*blinkPattern)[]) {}
void ledOff() {}

void ledSetup() {}
void ledLoop() {}

char* ledInit(int pin, boolean on) {
  return EMPTY;
}
char* ledSwitch(char* c, char* c2) {
  return EMPTY;
}

#endif

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
// sw

#if swEnable
//int _sw_time_base=100; // time base of sw in ms

#define swTickMax 254  // too long press 255*100 => 25.5s

//using ButtonEvent = void (*)(byte shortCount,unsigned long longTime); //type aliasing //C++ version of: typedef void (*InputEvent)(const char*)
unsigned long *switchTime = new unsigned long(0);  // sw timer

class Switch {

private:
  byte _swGpio;
  byte _swOn = 1;
  byte _swMode = 0;

  byte swLast = false;           // last switch on/off
  byte swShortCount = 0;         // number of short-press count
  unsigned long swLastTime = 0;  // last change
  byte swTickCount = 0;
  //ButtonEvent _onPress=NULL;
  MapList cmdList;  // 0=onDown,1..9=on n click,10=on Long

public:

  char *setCmd(int nr, char *cmd) {
    if (nr > 11) {
      return EMPTY;
    } else if (nr < 0) {
      sprintf(buffer, "");
      for (int i = 0; i < cmdList.size(); i++) {
        char *value = (char *)cmdList.get(i);
        if (is(value)) {
          sprintf(buffer + strlen(buffer), "swCmd %d \"%s\"\n", i, value);
        }
      }
      return buffer;
    } else if (!is(cmd)) {
      char *cmd = (char *)cmdList.get(nr);
      sprintf(buffer, "swCmd nr:%d cmd:%s", nr, to(cmd));
      return buffer;
    } else if (size(cmd) < 2) {
      cmdList.del(nr);
      sprintf(buffer, "swCmd del nr:%d", nr);
      return buffer;
    } else {
      cmdList.addIndex(nr, copy(cmd));
      sprintf(buffer, "swCmd set nr:%d cmd:%s", nr, to(cmd));
      return buffer;
    }
  }

  // sw press short times and  long time in ms (e.g. s_s_l => 2,600ms,2)
  void swPress(byte shortCount, unsigned long longTime) {
    sprintf(buffer, "SW press short:%d long:%dms", shortCount, longTime);
    logPrintln(LOG_DEBUG, buffer);
    if (longTime == 0) {
      char *cmd = (char *)cmdList.get(shortCount);
      if (is(cmd)) {
        char *c = copy(cmd);
        cmdLine(c);
        delete[] c;
      }
    } else {
      char *cmd = (char *)cmdList.get(10);
      if (is(cmd)) {
        char *c = copy(cmd);
        cmdLine(c);
        delete[] c;
      }
    }
  }

  // sw first (immediately)
  void swFirstDown(byte swNow) {
    sprintf(buffer, "SW DOWN %d", swNow);
    logPrintln(LOG_DEBUG, buffer);
    char *cmd = (char *)cmdList.get(0);
    if (is(cmd)) {
      char *c = copy(cmd);
      cmdLine(c);
      delete[] c;
    }  // cmdLine / cmdPrg
  }

public:
  // read button
  byte swRead() {
    if (_swMode == SW_MODE_TOUCH) {
      #ifdef ESP32
        return touchRead(_swGpio);
      #else 
        return digitalRead(_swGpio);
      #endif 
    } else {
      return digitalRead(_swGpio);
    }
  }
  // is button press
  boolean isOn() {
    return isOn(swRead());
  }
  boolean isOn(byte swNow) {
    if (_swOn < 2) {
      return swNow == _swOn;
    } else {
      return swNow < _swOn;
    }
  }

  void loop() {
    byte swNow = swRead();
    boolean isNowOn = isOn(swNow);

    if (isNowOn != swLast) {  // change
      if (isNowOn) {          // change=>on
        if (swShortCount == 0 && swTickCount == 0) { swFirstDown(swNow); }
        swTickCount = 1;                  // start on tick
        swShortCount++;                   // inc press
      } else {                            // change => off
        if (swTickCount >= swTickLong) {  // off => last on-swTickCount >swTickLong
          swPress(swShortCount, swTickCount);
          swShortCount = 0;
          swTickCount = 0;  // relase => long press
        } else {
          swTickCount = 1;  // start off tick
        }
      }

      swLast = isNowOn;
    } else if (isNowOn && swShortCount > 0) {   // on is hold
      swTickCount++;                            // count on tick
    } else if (!isNowOn && swShortCount > 0) {  // is off hold
      swTickCount++;                            // count off tick
      if (swTickCount > swTickShort) {
        swPress(swShortCount, 0);
        swShortCount = 0;
        swTickCount = 0;
      }  // no new press => short press
    }

    if (swTickCount >= swTickMax) {        // max on/off  => reset
      swPress(swShortCount, swTickCount);  // max => long press
      swShortCount = 0;
      swTickCount = 0;
    }
  }

  char *swInfo() {
    sprintf(buffer, "SW %d gpio:%d on:%d swMode:%d - swTimeBase:%d swTickShort:%d swTickLong:%d", swRead(), _swGpio, _swOn, _swMode,
            swTimeBase, swTickShort, swTickLong);
    return buffer; 
  }

  Switch(int gpio, byte swOn, byte swMode) {
    _swGpio = swGpio;
    _swOn = swOn;
    _swMode = swMode;
    swLast = !swOn;
    if (_swMode == SW_MODE_TOUCH) {
    }                                                                        //
    else if (_swMode == SW_MODE_PULLUP) { pinMode(_swGpio, INPUT_PULLUP);   // input with interal pullup ( _swGpio=GND (false) => pressed)
    #ifdef ESP32
      }else if (_swMode == SW_MODE_PULLDOWN) { pinMode(_swGpio, INPUT_PULLDOWN); // input with interal pulldown
    #endif
    }  else { pinMode(_swGpio, INPUT); }
    if (_swOn == 2) {
      byte swNow = swRead();
      if (swNow == 0) {
        _swOn = 1;
      } else if (swNow == 2) {
        _swOn = 0;
      } else {
        _swOn = swNow;
      }
    }  // swOn==2 => use actual as !on
    else if (_swOn > 2 && _swOn < 10) { _swOn = swRead() - _swOn; }
    logPrintln(LOG_INFO, swInfo());
  }
};

Switch *sw = NULL;
unsigned long *switchStartup = new unsigned long(1);  // sw timer

void swSetup() {
  if (!swEnable) { return; }
  sw = new Switch(swGpio, swOn, swMode);
}

// swInit 32 3 3 100 4 5
char *swInit(int pin, int on, byte mode, int timeBase, byte tickShort, byte tickLong) {
  if (pin != -1) {
    swGpio = pin;
    swOn = on;
    swMode = swMode;
    swTimeBase = timeBase;
    swTickShort = tickShort;
    swTickLong = tickLong;
    swSetup();
    return EMPTY;
  } else {
    return sw->swInfo();
  }
}

char *swCmd(int i, char *cmd) {
  if (sw == NULL) { return EMPTY; }
  return sw->setCmd(i, cmd);
}

void swLoop() {
  if (!swEnable) {
    return;
  } else if (sw != NULL && isTimer(switchTime, swTimeBase)) {
    sw->loop();
  }  // every 100ms
}

#else
void swSetup() {}
void swLoop() {}
char* swCmd(int i, char* cmd) {
  return EMPTY;
}
char* swInit(int pin, boolean on, byte mode, int timeBase, byte tickShort, byte tickLong) {
  return EMPTY;
}

#endif

//--------------------------------------------------------------------------------------
//  Wifi
 
//#include <WiFi.h>
#include <DNSServer.h>

#ifdef ESP32
  #include <esp_sntp.h> // time
#else
  #include <sys/time.h>  // struct timeval
#endif

#include <time.h>     // time

// Bootloader version
char bootType[5] = "Os01"; // max 10 chars

// bootloader data struc
typedef struct {
  unsigned long timestamp=0;                 // timestamp of store  
  unsigned long saveCount=0;                  // number of saves
  char wifi_ssid[32]="";                   // WIFI SSID of AccessPoint
  char wifi_pas[32]="";                     // WIFI password of AccessPoint
  char wifi_ntp[32]="";                   // ntp server
  // board
  char espName[32];                        // Name of device
  char espPas[32]="";                      // Password of device
  char espBoard[32]="";                    // BoardType
  // mqtt
  char mqtt[128]="";                        // mqtt Server
  boolean mqttLogEnable=false;              // enable send all logs to mqtt
  // access
  byte accessLevel=ACCESS_ADMIN;             // accessLevel 
} eeBoot_t;

eeBoot_t eeBoot;    // bootloader data 


#define MAX_NO_WIFI 120 // Max time 2min no wifi
#define MAX_NO_SETUP 20 // Max time 60s no wifi
unsigned long *wifiTime = new unsigned long(0);

#define WIFI_CON_OFF 0
#define WIFI_CON_CONNECTING 2
#define WIFI_CON_APCLIENT 3
#define WIFI_CON_CONNECTED 4

byte wifiStat=WIFI_CON_OFF; //  wifi status
int bootWifiCount=0; // counter for wifi not reached
int _lastClient=0; // numer of AP clients

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
// EEPROM

boolean isModeOk() { return eeMode>EE_MODE_AP && eeMode<EE_MODE_ERROR; }
boolean isModeNoSystemError() { return eeMode<EE_MODE_SYSERROR; }
boolean isModeNoError() { return eeMode<EE_MODE_ERROR; }

/* save to ee */
void eeSave() {
  if(serialEnable) { Serial.println("### SAVE");}
  eeBoot.timestamp=timeSec();  
  eeBoot.saveCount++; // rememeber number of saves
  int pos=0;
  EEPROM.begin(EEBOOTSIZE);
  EEPROM.put(pos, bootType ); pos+=5; // write type for validation
  EEPROM.put(pos, eeMode); pos+=1; 
  EEPROM.put(pos, eeBoot); pos+=sizeof(eeBoot); // save bootloader 
  EEPROM.commit();  // Only needed for ESP8266 to get data written
  eeAppPos=pos;
  sprintf(buffer,"eeSave eeAppPos %d",eeAppPos);logPrintln(LOG_DEBUG,buffer);
}

void eeRead() {   
  EEPROM.begin(EEBOOTSIZE);
  int pos=0;
  EEPROM.get( pos, eeType );  pos+=5; // eeType = "EspBoot100";
  if(strcmp(eeType,bootType)!=0) { eeMode=EE_MODE_FIRST; return ; }   // validate

  EEPROM.get(pos, eeMode ); pos+=1;
  EEPROM.get(pos, eeBoot); pos+=sizeof(eeBoot);// eeBoot read
  EEPROM.end(); 
  eeAppPos=pos;
  sprintf(buffer,"eeRead eeAppPos %d",eeAppPos);logPrintln(LOG_DEBUG,buffer);
}

void eeSetMode(byte mode) {
  EEPROM.begin(EEBOOTSIZE);
  EEPROM.put( 5, mode );
  EEPROM.end(); 
}

void setMode(byte mode) {
  eeSetMode(mode);
  eeMode=mode;
}

/* set default values */
void eeDefault() {
  uint32_t chipid=espChipId(); // or use WiFi.macAddress() ?
  if(!is(eeBoot.espName) || MODE_DEFAULT==EE_MODE_PRIVAT) { snprintf(eeBoot.espName,20, "CmdOs%08X",chipid);  }
  if((!is(eeBoot.espPas) || MODE_DEFAULT==EE_MODE_PRIVAT)) { sprintf(eeBoot.espPas,user_pas); }     // my private esp password   
  if(!is(eeBoot.wifi_ssid) || MODE_DEFAULT==EE_MODE_PRIVAT) {sprintf(eeBoot.wifi_ssid,wifi_ssid_default); } // my privat WIFI SSID of AccessPoint
  if(!is(eeBoot.wifi_pas) || MODE_DEFAULT==EE_MODE_PRIVAT) {sprintf(eeBoot.wifi_pas,wifi_pas_default); } // my privat WIFI SSID of AccessPoint
  if(!is(eeBoot.mqtt) || MODE_DEFAULT==EE_MODE_PRIVAT) {sprintf(eeBoot.mqtt,mqtt_default); }           // my privat MQTT server 
}

/* on first start prg */
void eeInit() {
  if(serialEnable) { Serial.println("### INIT");}
  eeMode=EE_MODE_SETUP; 
  eeDefault();
  eeSave();
}

/* ee setup */
void eeSetup() {
//TODO show restart reason
  if(MODE_DEFAULT==EE_MODE_FIRST) { bootClear(); } // is INIT => reset ALL
  else if(MODE_DEFAULT==EE_MODE_PRIVAT) {  if(serialEnable) { Serial.println("### MODE PRIVAT"); } eeDefault(); eeMode=EE_MODE_WIFI_TRY; return; }

  eeRead();

  if(strcmp(eeType,bootType)!=0) {  // type wrong
    if(serialEnable) { Serial.println("### MODE WRONG"); }
    eeInit(); // first Time 
    return; 
  }else if(eeMode==EE_MODE_SETUP) {
    if(serialEnable) { Serial.println("### MODE SETUP "); }
    setAccess(true);
    return ;
  } else if(eeMode==EE_MODE_AP) {
    if(serialEnable) { Serial.println("### MODE AP "); }
    setAccess(true);
    return ;
  }else if(eeMode==EE_MODE_OK) { 
    if(serialEnable) { Serial.println("### MODE OK -> START"); }
    setMode(EE_MODE_START); // mark  
    return ;
  }

  if(!bootSafe) { 
    Serial.print("### MODE(NOBS) ");Serial.println(eeMode); // ignore all other on disable boot safe 
  }else if(eeMode==EE_MODE_ERROR) {
    if(serialEnable) { Serial.println("### MODE ERROR "); }
    setAccess(true);
    setMode(EE_MODE_SYSERROR);  // mark 
  } else if(eeMode==EE_MODE_SYSERROR) {
    setAccess(true);
    if(serialEnable) { Serial.println("### MODE SYSERROR "); }
  }else if(eeMode>EE_MODE_WRONG) {
    if(serialEnable) { Serial.println("### MODE RE-INIT"); }
    eeInit(); // re-init

  } else if(eeMode>=EE_MODE_START) {
    if(serialEnable) { Serial.println("### MODE RE-START"); }
    eeSetMode(eeMode+1); // mark wrong+1

  }else {
    if(serialEnable) { Serial.print("### MODE ");Serial.println(eeMode); }
  }
  
}

unsigned long *eeTime = new unsigned long(1);
int okWait=60000; // wait 2min before WIFI => OK or SETUP => AP => WIFI 

void eeLoop() {
}

//-----------------------------------------------------------------------------

// info about boot 
char* bootInfo() {
   sprintf(buffer,"eeBoot eeMode:%d espName:%s espPas:%d espBoard:%s wifi_ssid:%s mqtt:%s ntp:%s timestamp:%d count:%d", 
    eeMode, to(eeBoot.espName),is(eeBoot.espPas),to(eeBoot.espBoard),to(eeBoot.wifi_ssid),to(eeBoot.mqtt),to(eeBoot.wifi_ntp),
    eeBoot.timestamp,eeBoot.saveCount); 
   return buffer;
}

/* set espName,espInfo */ 
char* bootSet(char* espName,char* espPas,char* espBoard) {
  if(is(espName,1,31) && isAccess(ACCESS_ADMIN)) { strcpy(eeBoot.espName,espName); }
  if(is(espPas,1,31) && isAccess(ACCESS_ADMIN)) { strcpy(eeBoot.espPas,espPas); }
  if(is(espBoard,1,31) && isAccess(ACCESS_ADMIN)) { strcpy(eeBoot.espBoard,espBoard); }
  return bootInfo();
}

/* save bootloader from RAM into EEPROM [ADMIN] */
void bootSave() {
  if(!isAccess(ACCESS_ADMIN)) { logPrintln(LOG_ERROR,"no access bootSave"); return ; }

  // auto set WIFI_CL_TRY when in AP
  if((eeMode<EE_MODE_WIFI_OFF) && is(eeBoot.wifi_ssid) && is(eeBoot.wifi_pas)) {
    setMode(EE_MODE_WIFI_TRY); 
  }
 
  eeSave();
  //EEPROM.end(); DO NOT END HERE => Otherwise EEPROM is not written
  ledBlink(3,100); // save => direct blink 30x100ms
  logPrintln(LOG_SYSTEM,bootInfo());
  espRestart("EEPROM boot save"); // restart after save
}

// Loads configuration from EEPROM into RAM
void bootRead() {
  eeRead();
  if(strcmp(eeType,bootType)!=0) { 
    sprintf(buffer,"EEPROM wrong"); logPrintln(LOG_SYSTEM,buffer); // => eeprom error => direct blink 2x100ms
    ledBlink(2,100); return ; 
  }    

  sprintf(buffer,"EEPROM boot read mode:%d timestamp:%d espName:%s wifi_ssid:%s",eeMode,eeBoot.timestamp,eeBoot.espName,eeBoot.wifi_ssid); logPrintln(LOG_SYSTEM,buffer); 
  logPrintln(LOG_SYSTEM,bootInfo());  
  if(!is(eeBoot.espPas)) { setAccess(ACCESS_ADMIN); } // without espApd admin=true
  ledBlink(1,100); // OK => direct blink 1x100ms
}

// Reset EEPROM bytes to '0' for the length of the data structure
void bootClear() {
  logPrintln(LOG_SYSTEM,"EEPROM boot clear"); ledBlink(10,100); // clear now => direct blink 10x100ms
  EEPROM.begin(EEBOOTSIZE);
  for (int i = 0 ; i < EEBOOTSIZE ; i++) {EEPROM.write(i, 0);}
  delay(200);
  EEPROM.commit();
  EEPROM.end();  
}

byte _bootRestVal=0;

/* boot reset */
char* bootReset(char *p) {
  int i=toInt(p);
  if(i>1 && i==_bootRestVal) { bootClear(); return "reset done";} // do reset 
  else { _bootRestVal=random(2,99); sprintf(buffer,"%d",_bootRestVal); return buffer; } // without set new reset value
}

//-------------------------------------------------------------------------------------------------------------------
// Access

/* is actual login */
boolean _isLogin=false;

/* have access level */
bool isAccess(int requireLevel) {   
  if(_isLogin) { return true; } // is login 
  else if(!is(eeBoot.espPas)) { return true; } // no password given
  else if(requireLevel>=eeBoot.accessLevel) { return true; } // access free
  else { 
    sprintf(buffer,"ACCESS DENIED %d<%d %d",requireLevel,eeBoot.accessLevel,_isLogin); 
    cmdError(buffer);     
    return false;     
    }
}

void setAccess(boolean login) { _isLogin=login; }
void setAccessLevel(byte accessLevel) { eeBoot.accessLevel=accessLevel; }

/* login (isAdmin=true) */
boolean login(char *p) {
  if(!is(eeBoot.espPas) || equals(p, eeBoot.espPas))  {  _isLogin=true; return true; }
  else { _isLogin=false; return false; }
}


//-------------------------------------------------------------------------------------------------------------------
// mDNS

#if mdnsEnable
  #ifdef ESP32
    #include <ESPmDNS.h>
  #elif defined(ESP8266)
    #include <ESP8266mDNS.h>
  #endif

void mdnsSetup() {
  if(!is(eeBoot.espName)) { return ; }
  else if(MDNS.begin(eeBoot.espName)) { 
    if(webEnable) { MDNS.addService("http", "tcp", 80); }
    sprintf(buffer,"MDNS setup %s http_tcp",eeBoot.espName); logPrintln(LOG_INFO,buffer); 
  }else { sprintf(buffer,"MDNS error"); logPrintln(LOG_ERROR,buffer); }
}

  void mdnsLoop() {  
    #ifdef ESP32
    #elif defined(ESP8266)
      MDNS.update();
    #endif
  }

#else
  void mdnsSetup() {}
  void mdnsLoop() { }
#endif

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
// Wifi

void webSetup();
    
// start scan network
char* wifiScan() { 
  byte networksFound=WiFi.scanNetworks(); 
  sprintf(buffer,"WIFI scan %d\n",networksFound);
  for (int i = 0; i < networksFound; i++) {
    if(strlen(buffer)<bufferMax-40) {    
      sprintf(buffer+strlen(buffer)," %d: %s (Ch:%d %ddBm) %d\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) );
    }
  }
  return buffer; 
}


//-------------------------------------------------------------------------------------------------------------------
// time

#if ntpEnable
  //const char* const PROGMEM NTP_SERVER[] = {"fritz.box", "de.pool.ntp.org", "at.pool.ntp.org", "ch.pool.ntp.org", "ptbtime1.ptb.de", "europe.pool.ntp.org"};
  //const char *NTP_TZ    = "CET-1CEST,M3.5.0,M10.5.0/3";
  #define timezone "CET-1CEST,M3.5.0/02,M10.5.0/03" // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv

  // callback when ntp time is given
  void ntpSet(struct timeval *tv) {
    time(&timeNow);                    // read the current time
    localtime_r(&timeNow, &tm);           // update the structure tm with the current time
    sprintf(buffer,"NTP set %d",timeNow); logPrintln(LOG_INFO,buffer);
    ntpRunning=true;
  }

  void ntpSet2(bool from_sntp) {
    time(&timeNow);                    // read the current time
    localtime_r(&timeNow, &tm);           // update the structure tm with the current time
    sprintf(buffer,"NTP set %d",timeNow); logPrintln(LOG_INFO,buffer);
    ntpRunning=true;    
  }

  /* ntp/timeserver config */
  void ntpSetup() {
  //  esp_sntp_servermode_dhcp(1);  // (optional)

    if(is(eeBoot.wifi_ntp,1,32)) { 
      ntpServer=eeBoot.wifi_ntp;     
    }else { 
      String gw=WiFi.gatewayIP().toString();
      ntpServer = copy((char*)gw.c_str());
    }
    
    if(ntpServer==NULL) {  logPrintln(LOG_ERROR,"ntp server missing");  return ; }

    int gtm_timezone_offset=0; // gmt+ or gmt-
    int dst=0; // 0=winter-time / 1=summer-time
    sprintf(buffer,"NTP start '%s' gtm_timezone_offset:%d dst:%d",ntpServer,gtm_timezone_offset,dst); logPrintln(LOG_INFO,buffer); 
    configTime(gtm_timezone_offset * 3600, dst*3600, ntpServer); //ntpServer
      
    #ifdef ESP32
      sntp_set_time_sync_notification_cb(ntpSet); // callback on ntp time set
    #else
      settimeofday_cb(ntpSet2); // callback on ntp time set
    #endif
  }
#else 
  void ntpSetup() { } 
#endif

/* set time and timeServer [ADMIN] */
char* timeSet(char* time,char* timeServer) {
  if(time!=NULL && strlen(time)>0) {
    if(!isAccess(ACCESS_ADMIN)) { return "no access set time"; }
    time_t newTime=(time_t)atol(time);
    timeval tv;tv.tv_sec = newTime;    
    settimeofday(&tv,NULL); // set your time (e.g set time 1632839830)
  }
  if(is(timeServer,1,31)) { 
    if(!isAccess(ACCESS_ADMIN)) { return "no access set timeServer"; }
    strcpy(eeBoot.wifi_ntp,timeServer);    
  }
  return timeInfo();
}

//-------------------------------------------------------------------------------------------------------------------

#if netEnable

  #include <ESPping.h>
  
  IPAddress pingIP;

  /* dns resolve ip/host to ip (e.g. char* name=netDns("192.168.1.1"); */
  char* netDns(char *ipStr) {
      #ifdef ESP32
        WiFi.hostByName(ipStr, pingIP);  
      #elif defined(ESP8266)
        WiFi.hostByName(ipStr, pingIP);  
      #endif
      sprintf(buffer,"%s",pingIP.toString().c_str()); return buffer;
  }

  /* ping given ip/host and return info (e.g. char* info=cmdPing("192.168.1.1"); ) */
  char* cmdPing(char *ipStr) { 
    #ifdef ESP32
      WiFi.hostByName(ipStr, pingIP);  
    #elif defined(ESP8266)
      WiFi.hostByName(ipStr, pingIP);   
    #endif

    int time=-1;
    if(Ping.ping(pingIP,1)) { time=Ping.minTime(); } 
    sprintf(buffer,"PING %s=%s time:%d",ipStr,pingIP.toString().c_str(),time); return buffer;
  }

#else 
  char* netDns(char *ipStr) { return NULL; }
  char* cmdPing(char *ipStr) { return NULL; }
#endif

//-------------------------------------------------------------------------------------------------------------------

//TODO #include "user_interface.h"

void sleepOver(void) {
  sprintf(buffer,"SLEEP over mode:%d",eeMode); logPrintln(LOG_INFO,buffer);
    
  wifiInit();
}

void sleep(byte mode,long sleepTimeMS) { //10e3=10s
    if(mode==0) { 
        sleepOver();  // switch all on
        
    } else if(mode==1) { 
        wifiOff(); }  // Modem OFF
        
    else if(mode==2) {  // LightSleep
      WiFi.mode(WIFI_OFF);  // WIFI off
      delay(sleepTimeMS + 1);
      
    }else if(mode==3) {  // DeepSleep
      WiFi.mode(WIFI_OFF);  // WIFI off
      uint64_t sleepTimeMicroSeconds = 10e6;
      ESP.deepSleep(sleepTimeMS*1000); 
      // reset is called after deepsleep 
    }  
}


void sleep(char* sleepMode,char *sleepTimeMS) {
  byte m=atoi(sleepMode); int s=atoi(sleepTimeMS);
  sprintf(buffer,"SLEEP %d %d",m,s);logPrintln(LOG_INFO,buffer);
  sleep(m,(long)s);
}
 
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------

// boot clear with BOOT=>BLINK 2x=>PRESS 2s=>WIFI_AP / 5s=>clear 
#if swEnable
  void bootSW() {    
    if(sw==NULL) { return ; } // now swtich for SW-Boot  
    delay(1000);
    if(!sw->isOn()) { return ; }
    logPrintln(LOG_INFO,"bootSetupSW -----------------------------------");
    byte count=0;
    while(sw->isOn() && count<4) { // while on, max 4*5s=20s
      delay(5000);
      ledBlink(count,100); // blink count
      count++;    
    }
    if(count==1) {  logPrintln(LOG_SYSTEM,"BOOTSW ap mode"); eeMode=EE_MODE_AP; } // hold sw >=5s => Mode Wifi AccessPoint
    else if(count==2) {  } 
//TODO find better way    else if(count==3) {  logPrintln(LOG_SYSTEM,"BOOTSW clear");bootClear(); espRestart("SW clear"); } // hold sw >=5s => reset
  }
#else 
  void bootSW() {}
#endif

void bootSetup() {
  if(MODE_DEFAULT!=EE_MODE_PRIVAT) { 
    // reset sw
    bootRead();
    bootSW();
  }
  
  sprintf(buffer,"BOOT init mode:%d espName:%s espBoard:%s wifi_ssid:%s timestamp:%d", eeMode,eeBoot.espName,eeBoot.espBoard,eeBoot.wifi_ssid,eeBoot.timestamp); logPrintln(LOG_INFO,buffer);
}

//--------------------------------------------------------------------------------------
// Wifi as AccessPoint


char apSSID[20]="";

IPAddress ap_IP(192,168,0,1);
IPAddress ap_gateway(192,168,0,1);
IPAddress ap_subnet(255,255,255,0);

DNSServer dnsServer;
boolean dnsRedirectEnable=true;

//-------------------------------------------

/* return info of wifi (log-buffer) */
char* wifiInfo() {
  sprintf(buffer,"WIFI mode:%d ip:%s wifi_ssid:%s - mac:%08X status:%d signal:%d rmac:%08X wifiStat:%d bootWifiCount:%d",
    eeMode,WiFi.localIP().toString().c_str(),WiFi.SSID().c_str()
    ,WiFi.macAddress(),WiFi.status(),WiFi.RSSI(),WiFi.BSSID()
    ,wifiStat,bootWifiCount); 
  
  if(is(apSSID)) {
    IPAddress myIP = WiFi.softAPIP(); 
    sprintf(buffer+strlen(buffer)," - AP_SSID:%s AP_IP:%s", apSSID,myIP.toString().c_str()); 
  }

  return buffer;
}

/* set wifi */ 
char* wifiSet(char *wifi_ssid,char *wifi_pas) {  
  if(is(wifi_ssid) && is(wifi_pas) && isAccess(ACCESS_ADMIN)) {     
    sprintf(buffer,"WIFI set %s %s",wifi_ssid,wifi_pas); logPrintln(LOG_SYSTEM,buffer);
    strcpy(eeBoot.wifi_ssid,wifi_ssid); 
    strcpy(eeBoot.wifi_pas,wifi_pas);
  }
  return wifiInfo();
}

/* setup wifi and espPas + save + boot */
char* setupEsp(char *wifi_ssid, char *wifi_pas,char *espName, char *espPas,char *mqtt) {
  if(!isAccess(ACCESS_ADMIN)) { return "no access setup"; }
  if(!is(wifi_ssid) && !(wifi_pas)) { return "missing ssid/pas"; }

  eeBoot= eeBoot_t(); // reinit 
  if(is(wifi_ssid,1,31)) {strcpy(eeBoot.wifi_ssid,wifi_ssid); }
  if(is(wifi_pas,1,31)) { strcpy(eeBoot.wifi_pas,wifi_pas);  }
  if(is(espName,1,31)) { strcpy(eeBoot.espName,espName); }
  if(is(espPas,1,31)) { strcpy(eeBoot.espPas,espPas); }
  mqttSet(mqtt); 
  bootSave();
  return bootInfo();
}

//-------------------------------------------

// start WIFI as AccessPoint
void wifiAccessPoint(boolean setpUpAP) {   
//TODO  ledBlinkPattern(0,&ledPatternFlashSlow); // blink AP mode
  if(setpUpAP) {
    sprintf(apSSID,"%s",wifi_setup);
  }else {
    uint32_t chipid=espChipId(); // or use WiFi.macAddress() ?
    snprintf(apSSID,20, "%s%08X",APP_NAME_PREFIX,chipid);
  }

  WiFi.softAPConfig(ap_IP, ap_gateway, ap_subnet);  
  WiFi.softAP(apSSID);
  setAccess(true); // enable admin in AP
  IPAddress myIP = WiFi.softAPIP();
  sprintf(buffer,"WIFI AccessPoint SSID:%s IP:%s", apSSID,myIP.toString().c_str()); logPrintln(LOG_SYSTEM,buffer); 
//TODO  ledBlinkPattern(0,&ledPatternFlashSlow); // blink AP mode

  dnsServer.start(53, "*", myIP); // redirect all dns request to esp
  dnsRedirectEnable=true;

  if(webEnable) { webSetup(); } // start web

  appIP=ap_IP.toString(); // set ip to ap_IP
  bootWifiCount=1;
}



/* this ap is connected from client */
void wifiAPClientConnect() {
  int numClients = WiFi.softAPgetStationNum();
  if (numClients>_lastClient) {
    _lastClient=numClients;
    wifiStat=WIFI_CON_APCLIENT;
    sprintf(buffer,"client %d connect to ap",numClients); logPrintln(LOG_INFO,buffer);    
    // esp_wifi_ap_get_sta_list()
  } else if(numClients==0) {  wifiStat=WIFI_CON_CONNECTING; } 
}

/* this client connected to remote set_up */
void wifiAPConnectoToSetup() {
    if (WiFi.status() == WL_CONNECTED) {      
      String gw=WiFi.gatewayIP().toString();
      String setupUrl="http://"+gw+"/setupDevice";
      sprintf(buffer,"WIFI set_up connected %1 call %s",gw.c_str(),setupUrl.c_str()); logPrintln(LOG_SYSTEM,buffer);
      char* ret=cmdRest((char*)setupUrl.c_str());
      logPrintln(LOG_SYSTEM,ret);
    }
}

//--------------------------------------------------------------------------------------

// wifi check connecting
void wifiConnecting() {
    if (WiFi.status() == WL_CONNECTED) {      
      String gw=WiFi.gatewayIP().toString();
      appIP=WiFi.localIP().toString();      
      sprintf(buffer,"WIFI mode:%d Connectd IP:%s Gateway:%s DNS:%s ", eeMode,appIP.c_str(), gw, WiFi.dnsIP().toString()); logPrintln(LOG_SYSTEM,buffer); 
      wifiStat=WIFI_CON_CONNECTED;

      if(eeMode==EE_MODE_WIFI_TRY) { setMode(EE_MODE_OK); } // try => cl  
      ledOff();

      // enable services
      if(webEnable) { webSetup(); }
      if(mdnsEnable) { mdnsSetup(); } 
      if(ntpEnable) { ntpSetup(); }

    }else { // Connecting
      wifiStat=WIFI_CON_CONNECTING;
      if(bootWifiCount==0) { 
//    }  else if( eeMode == EE_MODE_SETUP && bootWifiCount<MAX_NO_SETUP) {  // try faild
//        logPrintln(LOG_INFO,"no wifi setup"); 
//        eeSetMode(EE_MODE_AP); eeSave();espRestart("no setup wifi, fallback ap"); // fallback to AccessPoint on faild try  

      }else if(bootWifiCount<MAX_NO_WIFI) {              
//        if(serialEnable) {sprintf(buffer,"%d",WiFi.status()); Serial.print(buffer); }   
        bootWifiCount++;    
      
      } else if( eeMode == EE_MODE_WIFI_TRY) {  // try faild
        sprintf(buffer,"WIFI error CL-TRY ssid:%s reset to AP", eeBoot.wifi_ssid); logPrintln(LOG_SYSTEM,buffer); 
        setMode(EE_MODE_SETUP); espRestart("no wifi, fallback setup"); // fallback to AccessPoint on faild try      

      } else {
         //sprintf(buffer,"WIFI error connect ssid:%s mode:%d", eeBoot.wifi_ssid,eeBoot.mode); logPrintln(LOG_SYSTEM,buffer); 
         bootWifiCount=1; // try again
      }
    }  
}

//--------------------------------------------------------------------------------------

byte setupDevice=0;

char* setupDev(char *p0) {
  if(is(p0)) { 
    setupDevice=toInt(p0); 
    if(setupDevice>0) { wifiAccessPoint(true); } // enable wifi setp_up
    else { wifiInit(); }
  } 
  sprintf(buffer,"%d",setupDevice); return buffer;
}

//--------------------------------------------------------------------------------------


/* wifi login to SSID Router */
void wifiStart() {
//TODO  ledBlinkPattern(0,&ledPattern2Flash); // Blink unlimeted 2Flash
//TODO  configTime(NTP_TZ, NTP_SERVER[1]); // define timezone
//TODO  settimeofday_cb(ntpSet);  /// callback on ntp time set

  sprintf(buffer,"WIFI connecting SSID:%s ...", eeBoot.wifi_ssid); logPrintln(LOG_INFO,buffer); 
  delay(100);
//TODO ?  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(eeBoot.wifi_ssid, eeBoot.wifi_pas);
  bootWifiCount=1;  
  wifiStat=WIFI_CON_CONNECTING;
}

/* try connect wifi*/
void wifiTry() {
//TODO  ledBlinkPattern(0,&ledPattern2Flash); // Blink unlimeted 2Flash
  eeSetMode(EE_MODE_SETUP); // set FALLBACK ON ERROR
  sprintf(buffer,"WIFI try connecting SSID:%s ...", eeBoot.wifi_ssid); logPrintln(LOG_INFO,buffer); 
  delay(10);  
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA); //WIFI_STA
  // WIFI
  WiFi.begin(eeBoot.wifi_ssid, eeBoot.wifi_pas);
  // AP
  WiFi.softAPConfig(ap_IP, ap_gateway, ap_subnet);  
  WiFi.softAP(apSSID);
  bootWifiCount=1;  
  wifiStat=WIFI_CON_CONNECTING;
}

/* wifi login to setup Router */
void wifiStartSetup() {
  sprintf(buffer,"WIFI set_up connecting %s ...", wifi_setup); logPrintln(LOG_INFO,buffer); 
  delay(10);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_setup);
  bootWifiCount=1;  
  wifiStat=WIFI_CON_CONNECTING;
}

/* set wifi offline */
void wifiOff() {
  logPrintln(LOG_INFO,"WIFI off");
  WiFi.mode(WIFI_OFF);
}

// validate wifi connection
void wifiValidate() {

  if(eeMode==EE_MODE_AP) { 
    wifiAPClientConnect(); 
    if(isTimer(eeTime, okWait)) {
      if(wifiStat!=WIFI_CON_APCLIENT && is(eeBoot.wifi_ssid) && is(eeBoot.wifi_pas)) { 
        logPrintln(LOG_ERROR,"NO AP => switch to WIFI");
        eeMode=EE_MODE_START; wifiSetup();
      }
    }
    return ;
  }else if(eeMode==EE_MODE_SETUP) { 
    wifiAPConnectoToSetup(); 
//    if(serialEnable) { Serial.print("s");Serial.print(bootWifiCount); Serial.print(WiFi.status()); }
    if(isTimer(eeTime, okWait)) {
//      bootWifiCount++;
//      if(bootWifiCount>MAX_NO_SETUP) { // set_up failed => switch to ap
      logPrintln(LOG_INFO,"\nno wifi set_up found => switch to AP"); 
//        eeSetMode(EE_MODE_AP); eeSave();espRestart("no setup wifi, fallback ap"); // fallback to AccessPoint on faild try 
      eeMode=EE_MODE_AP;  
      wifiAccessPoint(false);             
    }    
  }

  if (wifiStat==WIFI_CON_CONNECTING) { // Connect or Reconnect
    wifiConnecting();   
  }   

  if(eeMode==EE_MODE_START && isTimer(eeTime, okWait)) {
    if(wifiStat==WIFI_CON_CONNECTED) { 
      logPrintln(LOG_DEBUG,"WIFI ok => EE_MODE_OK");
      setMode(EE_MODE_OK); 
    }else { 
//      logPrintln(LOG_ERROR,"NO WIFI => switch to SETUP");
//      eeMode=EE_MODE_SETUP; wifiSetup();
      logPrintln(LOG_ERROR,"NO WIFI => switch to AP");
      eeMode=EE_MODE_AP; wifiSetup();
    }

  }

}


// start wifi 
void wifiInit() {
    boolean ssidOk=is(eeBoot.wifi_ssid) && is(eeBoot.wifi_pas);
    appIP=EMPTY;
    
    if(ssidOk && eeMode==EE_MODE_WIFI_TRY) { // => Try 
      wifiTry(); // start Client
    
    }else if(ssidOk && (eeMode>=EE_MODE_PRIVAT && eeMode<=EE_MODE_WRONG )) { // => NormalMode
      wifiStart(); // start Client 
        
    }else if(eeMode==EE_MODE_SETUP) { // => SetupMode
      wifiStartSetup(); // start AccessPoint     

    }else if(eeMode==EE_MODE_AP || eeMode==EE_MODE_ERROR) { // => SetupMode
      wifiAccessPoint(false); // start AccessPoint 

    }else {
        wifiOff(); // => OFF  
    }
}

char* bootMode(int mode) {
  if(mode>=0) { setMode(mode); wifiInit(); }
  sprintf(buffer,"%d",eeMode); return buffer;
}

//----------------------------

void wifiSetup() {
  if(wifiEnable) { wifiInit(); }
}

void wifiLoop() {
  if(wifiEnable && isTimer(wifiTime, 1000)) { wifiValidate(); } // every second
  if(mdnsEnable) { mdnsLoop(); } 
  if(dnsRedirectEnable) { dnsServer.processNextRequest();  } // Process DNS requests 
}

void wifiStart(boolean on) { 
  wifiEnable=on; if(on) { setMode(EE_MODE_WIFI_TRY); wifiSetup(); } 
  else { WiFi.mode(WIFI_OFF); setMode(EE_MODE_WIFI_OFF); }
}

//-------------------------------------------------------------

#if otaEnable

#include <ArduinoOTA.h>

void otaSetup() {
   
 ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) { type = "sketch"; } else {   type = "filesystem"; }
//      FILESYSTEM.end(); // NOTE: if updating FS this would be the place to unmount FS using FS.end()    
    if(serialEnable) { Serial.println("Start updating " + type); }
  });
  ArduinoOTA.onEnd([]() {
    if(serialEnable) {  Serial.println("\nEnd"); }
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if(serialEnable) {  Serial.printf("Progress: %u%%\r", (progress / (total / 100))); }
  });
  ArduinoOTA.onError([](ota_error_t error) {
    if(serialEnable) { 
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) { Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) { Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) { Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) { Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) { Serial.println("End Failed"); }
    }
  });

  if(is(eeBoot.espName)) { ArduinoOTA.setHostname(eeBoot.espName); }
  if(is(eeBoot.espPas)) { ArduinoOTA.setPassword(eeBoot.espPas); logPrintln(LOG_INFO,"OTA setup ESPPAS"); }
  else { ArduinoOTA.setPassword("admin"); logPrintln(LOG_INFO,"OTA setup admin");  }

  ArduinoOTA.begin();
  
}

void otaLoop() {
  ArduinoOTA.handle();
}
#else 
  void otaSetup() {}
  void otaLoop() {}
#endif


//--------------------------------------------------------------------
// MQTT

#if mqttEnable

#include <PubSubClient.h>

unsigned long *mqttTime = new unsigned long(0); // mqtt timer

unsigned int mqttConFail=0;  // number of connection faild
static int mqttStatus = 0; // 0=Off, 1=Connecting, 2=On

char* mqttPrefix="device/esp";

boolean mqttSSL=false;
char* mqttClientName;
char* mqttServer;
int mqttPort;
char* mqttUser;
char* mqttPas;


char *mqttTopicOnline;   // topic device online/offline
char *mqttTopicStat;    // topic to send status of entitys
char *mqttTopicReceive; // topic path of all commands (e.g. device/esp/EspBoot00DC9235/control)

char* mqttTopicCmd=NULL; // topic for cmd messages (e.g. device/esp/EspBoot00DC9235/control/cmd)
char* mqttTopicResponse=NULL; // topic for resposne of cmd messages (e.g. device/esp/EspBoot00DC9235/stat_cmd) 
char *mqttTopicLog=NULL;

WiFiClient *mqttWifiClient=NULL;
#ifdef ESP32
  #include <NetworkClientSecure.h>
  NetworkClientSecure *mqttClientSSL=NULL; // WiFiClientSecure / NetworkClientSecure
#elif defined(ESP8266)
  #include <WiFiClientSecure.h>
  WiFiClientSecure *mqttClientSSL=NULL; // WiFiClientSecure / NetworkClientSecure
#endif   

PubSubClient *mqttClient=NULL;


//-------------------------------------------------------------------------------------
// mqtt-url:  mqtt://USER:PAS@SERVER:PORT or mqtts://USER:PAS@SERVER:PORT /

/* get mqtt infos */
char* mqttInfo() {
  if(!is(eeBoot.mqtt) || !is(mqttUser) || !is(mqttServer) ) { return "mqtt not defined"; }
  char *type="mqtt"; if(mqttSSL) { type="mqtts"; }  
  if(is(mqttUser)) {
    sprintf(buffer,"MQTT status:%d type:%s user:%s pas:%d server:%s port:%d (ee:%s)",
      mqttStatus,type,to(mqttUser),is(mqttPas),to(mqttServer),mqttPort,to(eeBoot.mqtt)); return buffer;
  }else { sprintf(buffer,"MQTT status:%d  %s://%s:%d (ee:%s)",mqttStatus,type,to(mqttServer),mqttPort,to(eeBoot.mqtt)); return buffer;}
}

/* split mqtt-url */
void mqttSetUrl(char* mqttUrl) {
  mqttUser=NULL; mqttPas=NULL; mqttServer=NULL; mqttPort=1833;  
  if(!is(mqttUrl,3,127)) { logPrintln(LOG_ERROR,"MQTT missing/wrong"); return ; }

  char *mqtt=copy(mqttUrl);  
   if(strncmp(mqtt, "mqtt://",7)==0) { mqttSSL=false;  } 
   else if(strncmp(mqtt, "mqtts://",7)==0) { mqttSSL=false; }
   else { return ; } 

   char *ptr; strtok_r(mqtt, "://",&ptr); ptr+=2; // start
   if(ptr==NULL) { logPrintln(LOG_ERROR,"MQTT mqtt/mqtts user"); return ;} 

   if(strchr(ptr, '@')!=NULL) {
    mqttUser = strtok_r(NULL, ":",&ptr); if(mqttUser==NULL) { logPrintln(LOG_ERROR,"MQTT missing user"); return ;} 
    mqttPas = strtok_r(NULL, "@",&ptr); if(mqttPas==NULL) { logPrintln(LOG_ERROR,"MQTT missing pas");  return ;} 
   }
   
   mqttServer = strtok_r(NULL, ":",&ptr); if(mqttServer==NULL) { logPrintln(LOG_ERROR,"MQTT missing server"); return ;} 
   char *port=strtok_r(NULL, "",&ptr); if(port==NULL) { logPrintln(LOG_ERROR,"MQTT missing port"); return ;} 
   mqttPort=atoi(port);

   sprintf(buffer,"MQTT set ssl:%d server:%s port:%d user:%s pas:%s", mqttSSL, to(mqttServer),mqttPort,to(mqttUser),to(mqttPas));  logPrintln(LOG_INFO,buffer);
//TODO memory leek here   delete[] mqtt; 
}

/* set mqtt url */
char* mqttSet(char* mqtt) {
  if(is(mqtt,1,128) && isAccess(ACCESS_ADMIN)) {     
    strcpy(eeBoot.mqtt,mqtt);  

    mqttSetUrl(eeBoot.mqtt);
  }
  return mqttInfo();
}



//-------------------------------------------------------------------------------------
// publish messages

/* publich log to mqtt */
void mqttLog(char *message) {
  if(eeBoot.mqttLogEnable) {
      if (mqttStatus != 2) { return ; }
      mqttClient->publish(mqttTopicLog, message);
  }
}

/* subcribe topic to attr */
boolean mqttAttr(char* name,char *topic) {
  if(mqttStatus != 2) { return false; }  
  paramsAdd(name,topic,0);
  attrMap.replace(name,(char*)"",0); 
  boolean ok=mqttSubscribe(topic); 
  sprintf(buffer,"MQTTattr subscribe '%s' topic:%s ok:%d", name,topic,ok); logPrintln(LOG_DEBUG,buffer);
  return true;
}

boolean mqttDel(char* name,char *topic) {
    if(topic==NULL) { return false; }
    boolean ok=mqttClient->unsubscribe(topic); 
    attrMap.del(name);
    appParams.del(name);
    sprintf(buffer,"MQTTattr unsubsrcibe '%s' topic:%s ok:%d", name,topic,ok); logPrintln(LOG_DEBUG,buffer);
    return ok;
}

boolean mqttDel(char* name) {
    char *topic=paramRemote(name);
    if(topic==NULL) { return false; } else { return mqttDel(name,topic); }
}

int paramsClear(byte type) {
  int count=0;
  for(int i=appParams.size()-1;i>=0;i--) {  
    AppParam *p=(AppParam*)appParams.get(i);
    if(type==255 || type==p->_type) { 
      mqttDel(p->name(),p->remote());
      count++;
    }
  }
  return count;
}

//-----------------------------------------------------

/* publish a maessage */
boolean mqttPublish(char* topic,char *message) {
  if (mqttStatus != 2) { return false; }
  boolean ok=mqttClient->publish(topic, message);  
  if(!ok) { sprintf(buffer,"MQTT publish ERROR %s len:%d",topic,strlen(message));logPrintln(LOG_ERROR,buffer); }
  else { 
//Serial.print(topic); Serial.print("=");Serial.println(message); Serial.print(" len:");Serial.println(strlen(message));
    sprintf(buffer,"MQTT publish %s",topic); logPrintln(LOG_DEBUG,buffer);
  }
  return ok;
}

/* subscibe a topic */
boolean mqttSubscribe(char *topic) {
    if (mqttStatus != 2) { return false; }
    boolean ok=mqttClient->subscribe(topic); // subscribe cmd topic           
    if(!ok) { sprintf(buffer,"MQTT subscribe ERROR %s",topic);logPrintln(LOG_ERROR,buffer); }
    else { sprintf(buffer,"MQTT subscribe %s ok:%d", topic,ok); logPrintln(LOG_INFO,buffer); }
    return ok;
}

//-----------------------------------------------------

void mqttPublishState(char *name,char *message) {
  if(!is(message) ||  mqttStatus != 2) { return ; }
  char *espStat=concat(mqttTopicStat,name,NULL);
  mqttPublish(espStat,message);  
  free(espStat);
}

//-------------------------------------------------------------------------------------
// Receive messages

void mqttReceive(char* topic, byte* payload, unsigned int length) {    
  char *msg=copy(NULL,(char*)payload,length);
//  if(length>=bufferMax) { logPrintln(LOG_ERROR,"mqtt msg to long"); return ; }
//  memcpy( buffer, payload, length); buffer[length]='\0'; char *msg=buffer;

  if (mqttCmdEnable && strcmp(topic,mqttTopicCmd) == 0) {   // call cmd     
    char *result=cmdLine(msg); 
//    free(msg);    
    mqttPublishState("cmd",result);
    sprintf(buffer,"MQTT cmd '%s' %d", topic, length); logPrintln(LOG_DEBUG,buffer);

  }else if(endsWith(topic,"restart")) { espRestart("MQTT restart"); 

  }else if(mqttOnMsg(topic,msg)) {
//     free(msg);
     return ;
     
  } else if(attrMap.find(topic)!=-1) {  // set topic as attribute 
    attrMap.replace(topic,(char*)payload,length);
    sprintf(buffer,"MQTT attrSet '%s'", topic); logPrintln(LOG_DEBUG,buffer);
//    free(msg);

/*
  #if mqttDiscovery  
  } else if (strcmp(topic,mqttTopicReceive) == 0) { 
      char *msg=copy(NULL,(char*)payload,length);

      sprintf(buffer,"MQTT HA '%s'=%s", topic,msg); logPrintln(LOG_DEBUG,buffer);
      //char *result=cmdLine(msg); 
      //if(result!=NULL) { mqttPublish(mqttTopicStat,result); }
      free(msg);
  #endif
*/

  } else { 
    boolean ok=paramSet(topic,msg); // set as param
    if(!ok) { sprintf(buffer,"MQTT unkown topic '%s'", topic); logPrintln(LOG_DEBUG,buffer); }    
//    free(msg);
  }

}

//-------------------------------------------------------

#if mqttDiscovery

/* auto discover this as a light in HomeAssistant */
void mqttDiscover(char *type,char *name,boolean stateOn,boolean receiveOn) {     
    
    uint32_t chipid=espChipId();
//    char *topic=concat("homeassistant/",type,"/CmdOS/",eeBoot.espName,"/config",NULL);
//    sprintf(paramBuffer,"CmdOs%08X_%s",chipid,name);
    
    sprintf(buffer,"{\"name\":\"%s\",\"uniq_id\":\"CmdOs%08X_%s\",\"avty_t\":\"%s\"",name,chipid,name,mqttTopicOnline);
    if(stateOn) {
       sprintf(buffer+strlen(buffer),",\"stat_t\":\"%s%s\"",mqttTopicStat,name);
    }
    if(receiveOn) { 
      char *espCmd=concat(mqttTopicReceive,name,NULL); 
      sprintf(buffer+strlen(buffer),",\"cmd_t\":\"%s\"",espCmd);
      free(espCmd);
    }
    sprintf(buffer+strlen(buffer),",\"dev\":{\"name\":\"%s\",\"ids\":\"CmdOs%08X\",\"configuration_url\":\"http://%s\",\"mf\":\"%s\",\"mdl\":\"%s\",\"sw\":\"%s\"}",eeBoot.espName,chipid,appIP.c_str(),APP_NAME_PREFIX,prgTitle,prgVersion);
    sprintf(buffer+strlen(buffer),"}");
    
    char *topic=concat("homeassistant/",type,"/",eeBoot.espName,"/",name,"/config",NULL);
    mqttPublish(topic,buffer);
    free(topic); 

//    mqttPublish(mqttTopicStat, "");        // state        
}

#endif

//-------------------------------------------------------

boolean mqttRunning=false;

void mqttInit() {
  if(MODE_DEFAULT==EE_MODE_PRIVAT) { mqttSetUrl((char*)mqtt_default); } // my privat MQTT server)
  else if(!is(mqttServer) && is(eeBoot.mqtt)) { mqttSetUrl(eeBoot.mqtt);  }// set mqtt
  mqttClientName=eeBoot.espName;

  if(!is(mqttServer)) { logPrintln(LOG_SYSTEM,"MQTT error - mqttServer missing");  mqttConFail=3; return ; }  
  else if(mqttPort<1) { logPrintln(LOG_SYSTEM,"MQTT error - mqttPort missing");  mqttConFail=3; return ; }  
  else if(!is(eeBoot.espName)) { logPrintln(LOG_SYSTEM,"MQTT error - clientName missing");  mqttConFail=3; return ; }  

  if(!mqttSSL) { 
    mqttWifiClient=new WiFiClient();  
    mqttClient=new PubSubClient(*mqttWifiClient);
  }else {
    #ifdef ESP32
      mqttClientSSL=new NetworkClientSecure();  
    #elif defined(ESP8266)
      mqttClientSSL=new WiFiClientSecure();  
    #endif    
    mqttClient=new PubSubClient(*mqttClientSSL);
  }  

  mqttTopicOnline=concat(mqttPrefix,"/",eeBoot.espName,"/online",NULL); // availability/online
  mqttTopicStat=concat(mqttPrefix,"/",eeBoot.espName,"/stat_",NULL); 
  mqttTopicReceive=concat(mqttPrefix,"/",eeBoot.espName,"/control/",NULL);
  if(mqttLogEnable) { mqttTopicLog=concat(mqttPrefix,"/",eeBoot.espName,"/log",NULL); }

  mqttClient->setBufferSize(512); // extends mqtt message size
  mqttClient->setCallback(mqttReceive);     
  mqttClient->setServer(mqttServer, mqttPort);

  mqttTopicCmd =concat(mqttTopicReceive,"cmd",NULL);
  mqttTopicResponse =concat(mqttTopicStat,"cmd",NULL);

  mqttRunning=true;
  *mqttTime=0; // start conection now
  if(!is(mqttUser) || !is(mqttPas)) {
    sprintf(buffer,"MQTT init %s:%d user: pas: client:%s", mqttServer, mqttPort,mqttClientName); logPrintln(LOG_INFO,buffer);
  }else { 
    sprintf(buffer,"MQTT init %s:%d user:%s pas:%s client:%s", mqttServer, mqttPort,mqttUser,mqttPas,mqttClientName); logPrintln(LOG_INFO,buffer);
  }
}


void mqttConnect() {    
    sprintf(buffer,"MQTT connecting... %s => %s", mqttClientName, mqttServer); logPrintln(LOG_DEBUG,buffer);    

    if (mqttClient->connect(mqttClientName,mqttUser,mqttPas,mqttTopicOnline, 0, true, "offline")) { // cennect with user and last will availability="offline"
      sprintf(buffer,"MQTT connected %s => %s", mqttClientName, mqttServer); logPrintln(LOG_INFO,buffer); 

      mqttStatus = 2;          
      
      char *cmdTopic=concat(mqttTopicReceive,"+",NULL); mqttSubscribe(cmdTopic); free(cmdTopic); // subscibe all cmds

      #if mqttDiscovery
        mqttDiscover("button","restart",false,true); // send mqtt homaAssistant state online Discover
        if(mqttCmdEnable) { mqttDiscover("text","cmd",true,true); } // send mqtt homaAssistant Discover
//        mqttPublishState("state", "on");                   // discovery state
      #endif

      mqttOnConnect(); // app spezific mqtt-commands after mqtt connect
      mqttClient->publish(mqttTopicOnline, "online");    // availability  online/offline
      mqttConFail=0;   // connected => reset fail

    } else {
      sprintf(buffer,"MQTT connection faild %d rc:%d - %s => %s", mqttConFail, mqttClient->state(),mqttClientName, mqttServer);  logPrintln(LOG_SYSTEM,buffer);      
      mqttConFail++; 
      mqttStatus = 0;
    }
}

void mqttDisconnect() { 
  if(mqttClient!=NULL && mqttClient->connected()) {
    mqttClient->publish(mqttTopicOnline, "offline");    // availability  online/offline
//    publishValue("status","disconnect");
    mqttClient->disconnect();     
  }
  mqttClient=NULL;
  mqttStatus = 0;
  mqttRunning=false;
}

void mqttOpen(boolean on) {
  if(on) { mqttInit(); } else { mqttDisconnect(); }
}
//-------------------------------------------------------------------------------------

void mqttSetup() {
  if(mqttEnable) { mqttInit(); }
}

void mqttLoop() {
  if(mqttRunning && is(mqttServer) && eeMode==EE_MODE_OK) {
    if(mqttClient==NULL) { mqttInit(); }
    else if (!mqttClient->connected()) { 
        if(mqttConFail<3 && isTimer(mqttTime, 1000)) { 
          mqttConnect(); // every second => reconnect mqtt
        }else if(mqttConFail>=3 && isTimer(mqttTime, 60000)) { 
          mqttConnect(); // every min => reconnect mqtt
        }
    }
    mqttClient->loop();   
  }  
}

#else
  void mqttSetUrl(char* mqttUrl) {}
  void mqttOpen(boolean on) {}
  void mqttInit() {}
  void mqttDisconnect() { }
  char* mqttSet(char* mqtt) { return "MQTT disabled"; }
  void mqttSetup() {}
  void mqttLoop() {}
  void mqttLog(char *message) {}
  boolean mqttPublish(char* topic,char *message) {} 
  boolean mqttAttr(char *name,char *topic) {}  
  boolean mqttDel(char*name) {}
  void mqttPublishState(char *name,char *message) {}

  int paramsClear(byte type) { return 0; }

#endif


//-----------------------------------------------------------------------------------
// web

#if webEnable

#include <Arduino.h>
#ifdef ESP32
  #include <AsyncTCP.h>
  #include <ESPmDNS.h>
#elif defined(ESP8266)
  #include <ESPAsyncTCP.h>
  #include <ESP8266mDNS.h>
#endif

AsyncAuthenticationMiddleware basicAuth;

boolean _webInit = false;

String webParam(AsyncWebServerRequest *request,String key) { 
  if (request->hasParam(key)) { return request->getParam(key)->value(); }
  else { return EMPTYSTRING; }
}

//-------------------------------------------------------------------------------------------------------------------

String pageHead(String html, String title) {
  html += "<html><head><title>" + title + "</title></head><body><b>" + prgTitle + " - " + title + "</b> | ";
  html += "[<a href='/app'>app</a>][<a href='/config'>config</a>][<a href='/file'>file</a>][<a href='/webserial'>console</a>][<a href='/cmd'>cmd</a>][<a href='/firmware'>firmware</a>]<hr>";
  return html;
}
String pageEnd(String html,String message) {
  if(is(message)) { html+="<script>alert('"+message+"');</script>"; }
  html += "</body></html>";
  return html;
}
String pageForm(String html, String title) {
  html += "<table><form method='GET'><tr><th colspan='2'>" + title+"</th></tr>";
  return html;
}

String pageFormEnd(String html) { 
  html+="</form></table>"; return html;
}

String pageInput(String html, const char *key, char *value) {
  String k = String(key);
  html += "<tr><th align='right'>"+k+"</th><td><input label='" + k + "' type='text' name='" + k + "' value='" + String(value) + "' size='64'/></td></tr>";
  return html;
}
String pageButton(String html, const char *key, const char *value) {
  html += "<tr><th></th><td><input type='submit' name='" + String(key) + "' value='" + String(value) + "'/></td>";
  return html;
}

String pageCmd(String html, const char *cmd, const char *name) {
  html+="[<a href='/cmd/"+String(cmd)+"'>"+String(name)+"</A>]";
  return html;
}

String pageUpload(String html, String title,String action) {
  html += "<form id='dz' method='POST' action='" + action + "' enctype='multipart/form-data' style='width:300px;height:150px;border: 1px dotted grey;text-align: center;'>";
  html += "<br><br><b>"+title+"</b><br>(drop or select file)<br><br><input id='fz' type='file' name='update'><input type='submit' value='ok'></form>";
  html += "<script>var ddz=document.getElementById('dz');";
  html += "function dragIt(e,color) { e.preventDefault();e.stopPropagation(); ddz.style.backgroundColor=color; }";
  html += "function dropit(e) { e.preventDefault();e.stopPropagation(); document.getElementById('fz').files=e.dataTransfer.files; ddz.submit(); }";
  html += "ddz.addEventListener('dragover', function (e) {dragIt(e,\"green\");}, false);";
  html += "ddz.addEventListener('drop', dropit, false);";
  html += "ddz.addEventListener('dragleave', function (e) {dragIt(e,\"white\");}, false);";
  html += "</script>";
  return html;
}

//-------------------------------------------------------------------------------------------------------------------

boolean isWebAccess(int level) {
  setAccess(true); // enable is admin in web access
  return true;
}

void webRoot(AsyncWebServerRequest *request) {
  isWebAccess(ACCESS_READ); 
  String html = "";
  html = pageHead(html, "Index");
  html = pageEnd(html,EMPTYSTRING);
  request->send(200, "text/html", html);
}

void webWifiReset(AsyncWebServerRequest *request) {
  String value=webParam(request,"reset");
  char* res=bootReset((char*)value.c_str());
  String html = "";
  html = pageHead(html, "Reset");
  html += "<b>"+String(res)+"</b>";
  html = pageForm(html, "Reset");
  html = pageInput(html, "reset", "");
  html = pageButton(html, "ok", "ok");
  html = pageFormEnd(html);

  html = pageEnd(html,EMPTYSTRING);
  request->send(200, "text/html", html);
}

//-------------------------------------------------------------------------------------------------------------------
// File Manager

void webFileManagerRename(AsyncWebServerRequest *request, String name) {
  if(!isWebAccess(ACCESS_CHANGE)) { request->send(403, "text/html"); }
  String html = "";
  html = pageHead(html, "File Manager - Rename");
  html += "Rename <form method='get'><input type='hidden' name='name' value='" + name + "'/><input type='text' name='newname' value='" + name + "'/><input type='submit' name='doRename' value='ok'></form>";
  html = pageEnd(html,EMPTYSTRING);
  request->send(200, "text/html", html);
}

void webFileManagerEd(AsyncWebServerRequest *request, String name) {
  if(!isWebAccess(ACCESS_CHANGE)) { request->send(403, "text/html"); }
  String html = "";
  html = pageHead(html, "File Manager - Ed");
  html += "<form method='GET' action='?doSave=1'><input type='text' name='name' value='" + name + "'/><br>";
  html += "<textarea label='" + name + "' name='value' cols='80' rows='40'>";
  File ff = SPIFFS.open(name, "r");
  if (ff) { html += ff.readString(); }
  ff.close();
  html += "</textarea><br><input type='submit' name='doSave' value='save'>";
  html += "<input type='submit' name='doSaveAndRun' value='save and run'></form>";
  html = pageEnd(html,EMPTYSTRING);
  request->send(200, "text/html", html);
}

/* save file [ADMIN]  */
void webFileManagerSave(AsyncWebServerRequest *request, String name, String value) {  
  if(!isWebAccess(ACCESS_CHANGE)) { request->send(403, "text/html"); }
  
  File ff = SPIFFS.open(name, "w");
  if (value != NULL) { ff.print(value); }
  ff.close();
  sprintf(buffer, "save %s", name.c_str()); logPrintln(LOG_INFO,buffer);
}


/* upload file */
void webFileManagerUpload(AsyncWebServerRequest *request, String file, size_t index, uint8_t *data, size_t len, bool final) {
  if(!isWebAccess(ACCESS_CHANGE)) { request->send(403, "text/html"); }
  sprintf(buffer, "upload %s %d index:%d", file.c_str(), len, index);logPrintln(LOG_INFO,buffer);

  File ff;
  if (!index) {
    FILESYSTEM.remove(rootDir + file); // remove old file 
    ff = SPIFFS.open(rootDir + file, "w");
  } else {
    ff = SPIFFS.open(rootDir + file, "a");
  }

  for (size_t i = 0; i < len; i++) { ff.write(data[i]); }
  ff.close();
  if (final) {
    sprintf(buffer, "uploaded %s %d", file.c_str(), (index + len)); logPrintln(LOG_INFO,buffer);
    request->redirect("/file");
  }
}


void webFileManager(AsyncWebServerRequest *request) {
  String message;
  if (request->hasParam("del")) { fsDelete(webParam(request,"name")); }
  else if (request->hasParam("rename")) { webFileManagerRename(request, webParam(request,"name")); return; }
  else if (request->hasParam("doRename")) { fsRename(webParam(request,"name"), webParam(request,"newname")); }
  else if (request->hasParam("ed")) { webFileManagerEd(request, webParam(request,"name")); return; }
  else if (request->hasParam("doSave") || request->hasParam("doSaveAndRun")) { 
    webFileManagerSave(request, webParam(request,"name"), webParam(request,"value")); 
    if(request->hasParam("doSaveAndRun")) { cmdFile((char*)webParam(request,"name").c_str()); }
    webFileManagerEd(request, webParam(request,"name")); return;
  }
  else if (request->hasParam("doUploadUrl")) { message=fsDownload(webParam(request,"url"), webParam(request,"name"),-1);  }

  String html = "";
  html = pageHead(html, "File Manager");
  html += "[<a href='?ed=1&name=/new'>new</a>]<p>";
  File root = FILESYSTEM.open(rootDir,"r");
  File foundfile = root.openNextFile();
  while (foundfile) {
    String name = String(foundfile.name());
    html += "<li><a href='/res?name=" + rootDir + name + "'>" + name + "</a> (" + foundfile.size() + ")";
    html += " [<a href='?ed=1&name=" + rootDir + name + "'>edit</a>]";
    html += " [<a href='?del=1&name=" + rootDir + name + "'>delete</a>]";
    html += " [<a href='?rename=1&name=" + rootDir + name + "'>rename</a>]";
    html += " [<a href='/res?name=" + rootDir + name + "' download='" + name + "'>download</a>]";
    foundfile = root.openNextFile();
  }
  root.close();
  foundfile.close();
  html += "</ul><hr>";
  html = pageUpload(html, "Upload File","/doUpload");
  html += "<form method='GET'>Upload URL<input type='text' size='64' name='url'/><input type='submit' name='doUploadUrl' value='ok'/></form>";
  html = pageEnd(html,message);
  request->send(200, "text/html", html);
}

//-------------------------------------------------------------------------------------------------------------------
// OTA Web Update

#if updateEnable

  #ifdef ESP32
    #include <Update.h>
  #elif defined(ESP8266)
    #include <Updater.h>
  #endif

  size_t content_len;

  void webUpdate(AsyncWebServerRequest *request) {
    String html = "";
    html = pageHead(html, "Firmware Update");
    html += "<form method='POST' action='/doUpdate' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
    html = pageEnd(html,EMPTYSTRING);
    request->send(200, "text/html", html);
  }

  void webDoUpdate(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!updateEnable) {
      request->send(200, "text/html", "ota disabled");
      return;
    }
    //  if(!request->authenticate(http_username, http_password)) { return request->requestAuthentication(); }
    if (!index) {
      logPrintln(LOG_SYSTEM,"Update");
      content_len = request->contentLength();
      #ifdef ESP8266
        Update.runAsync(true);
        if (filename.indexOf("filesystem") > -1) {            
          if (!Update.begin(content_len, U_FS)) { Update.printError(Serial); }
        }else {
          uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
          if (!Update.begin(maxSketchSpace, U_FLASH)) { Update.printError(Serial); }
        }
      #else
        int cmd = (filename.indexOf("spiffs") > -1) ? U_SPIFFS  : U_FLASH;  // if filename includes spiffs, update the spiffs partition
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) { Update.printError(Serial); }
      #endif
    }

    if (Update.write(data, len) != len) {
      Update.printError(Serial);
  #ifdef ESP8266
    } else {
      sprintf(buffer, "Progress: %d%%", (Update.progress() * 100) / Update.size()); logPrintln(LOG_SYSTEM,buffer);
  #endif
    }

    if (final) {
      AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
      response->addHeader("Refresh", "20");
      response->addHeader("Location", "/");
      request->send(response);
      if (!Update.end(true)) {
        Update.printError(Serial);
      } else {
        espRestart("Update complete");
      }
    }
  }

  void webProgress(size_t prg, size_t sz) {
    sprintf(buffer, "Progress: %d%%", (prg * 100) / content_len);
    logPrintln(LOG_SYSTEM,buffer);
  }

#else
  void webUpdate(AsyncWebServerRequest *request) { request->send(200, "text/html", "webupdate not implemented"); }
  void webDoUpdate(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {}
  void webProgress(size_t prg, size_t sz) {}
#endif

//-------------------------------------------------------------------------------------------------------------------
// web serial console

#if webSerialEnable

  #include <WebSerialLite.h>

  void recvMsg(uint8_t *data, size_t len){
    String line = "";
    for (int i = 0; i < len; i++) { line += char(data[i]); }
    char ca[len + 1];
    for (int i = 0; i < len; i++) { ca[i] = data[i]; }
    ca[len] = '\0';
    char* ret = cmdLine(ca);  // exec cmd
    if(is(ret)) { webLogLn(toString(ret)); }
  }

  void webLogLn(String msg) {
    if (webEnable && _webInit) { WebSerial.println(msg); }
  }

  void webSerialSetup() {
    WebSerial.begin(&webServer);
    WebSerial.onMessage(recvMsg);
//TODO    if (is(eeBoot.espPas)) { WebSerial.setAuthentication(user_admin, eeBoot.espPas); }  // webSerial auth
    sprintf(buffer, "WebSerial started /webserial"); logPrintln(LOG_DEBUG,buffer);
  }

#else
  void webSerialSetup() {}
  void webLogLn(String msg) {}
#endif

//-------------------------------------------------------------------------------------------------------------------
// auth

void webWifiSet(AsyncWebServerRequest *request) {

  String v=webParam(request,"espName"); 
  if(is(v,1,20)) {  v.toCharArray(eeBoot.espName, sizeof(eeBoot.espName)); }
  v=webParam(request,"espPas"); if(is(v,1,20)) {  v.toCharArray(eeBoot.espPas, sizeof(eeBoot.espPas)); }
  v=webParam(request,"espBoard"); if(is(v,1,20)) {  v.toCharArray(eeBoot.espBoard, sizeof(eeBoot.espBoard)); }
  v=webParam(request,"wifi_ssid"); if(is(v,1,32)) {  v.toCharArray(eeBoot.wifi_ssid, sizeof(eeBoot.wifi_ssid)); }
  v=webParam(request,"wifi_pas"); if(is(v,1,32)) {  v.toCharArray(eeBoot.wifi_pas, sizeof(eeBoot.wifi_pas)); }
  v=webParam(request,"wifi_ntp"); if(is(v,1,32)) {  v.toCharArray(eeBoot.wifi_ntp, sizeof(eeBoot.wifi_ntp)); }
  v=webParam(request,"mqtt"); if(is(v,1,64)) {  v.toCharArray(eeBoot.mqtt, sizeof(eeBoot.mqtt)); }

  logPrintln(LOG_INFO,bootInfo());
}

void webWifi(AsyncWebServerRequest *request) {
  String message;
  if (request->hasParam("ok")) { webWifiSet(request); message="set"; }
  else if (request->hasParam("save")) { bootSave(); message="saved";  }
  else if (request->hasParam("reset")) { webWifiReset(request); return ;  }
  else if (request->hasParam("restart")) { espRestart("web restart"); }

  String html = "";
  html = pageHead(html, "wifi");
  html+= "[<a href='/config'>network</a>][<a href='/appSetup'>app</a>]";
  html = pageForm(html, "Wifi config");
  html = pageInput(html, "espName", eeBoot.espName);
  html = pageInput(html, "espPas", ""); // eeBoot.espPas
  html = pageInput(html, "espBoard", eeBoot.espBoard);
  html = pageInput(html, "wifi_ssid", eeBoot.wifi_ssid);
  html = pageInput(html, "wifi_pas", ""); // eeBoot.wifi_pas
  html = pageInput(html, "wifi_ntp", eeBoot.wifi_ntp);
  html = pageInput(html, "mqtt", eeBoot.mqtt);
  html = pageButton(html, "ok", "ok");
  html = pageFormEnd(html);
  html+= "<hr>";
  html = pageCmd(html, "restart", "restart");
  html = pageCmd(html, "save", "save");
//  html = pageCmd(html, "reset", "reset");
  html += "[<a href='?reset=0'>reset</a>]";
  
  //  html=pageFormEnd(html,"ok");
  html = pageEnd(html,message);
  request->send(200, "text/html", html);
}

//-------------------------------------------------------------------------------------------------------------------

/* call cmd from web (e.g. "/cmd/wifi KEY PAS" or "/cmd/wifi?wlan=KEY&password=PAS" or "/cmd?cmd=wifi&wlan=KEY&password=PAS")
  ! parameter will addad to cmd on given order => ?one=1&b=3&c=2 => 1 3 2 !
*/
void webCmd(AsyncWebServerRequest *request) {
  String cmd;
  // get cmd from path
  String path = request->url();
  int index=path.indexOf('/',1);
  if(index!=-1) { cmd=path.substring(index+1); }
  // get cmd from parma
  if(cmd==NULL && request->hasParam("cmd")) { cmd=request->getParam("cmd")->value(); } // get attr (&cmd=CMD)
  if(cmd==NULL) {  
    String html=""; html=pageHead(html,"cmd");
    html+="<form>CMD:<input type='text' size='100' name='cmd' value=''/><input type='submit' name='ok' value='ok'></form>";
    html=pageEnd(html,EMPTYSTRING);
    request->send(200, "text/html", html); 
    return ; 
  }

  // add all params to cmd 
  int params = request->params();
  for(int i=0;i<params;i++){
    const AsyncWebParameter* p = request->getParam(i);
    String key=p->name();
    if(!key.equals("cmd") && !key.equals("ok")){ cmd+=" "+p->value(); } 
  }
  
  char *inData=(char*)cmd.c_str(); 
  String ret=cmdLine(inData);

//  String html=""+ret;
  request->send(200, "text/plain", ret);
}

//-------------------------------------------------------------------------------------------------------------------



/* setup remote device by return "setup wifi_ssid wifi_pas NAME espPas" */
void webSetupDevice(AsyncWebServerRequest *request) {
  if(setupDevice>0) {
    String name=webParam(request,"name");
    char *setupName=NULL; if(name!=NULL) { setupName=(char*)name.c_str(); } 
    sprintf(buffer,"setup %s %s \"%s\" %s %s",to(eeBoot.wifi_ssid),to(eeBoot.wifi_pas),to(setupName),to(eeBoot.espPas),
      to(eeBoot.mqtt));
    request->send(200, "text/plain", buffer);     
    if(setupDevice<255) { setupDevice--;}
    sprintf(buffer,"webSetupDevice %s setupDevice:%d",to(setupName),setupDevice); logPrintln(LOG_INFO,buffer); 
    if(setupDevice==0) { 
//      wifiInit(); 
      WiFi.softAPdisconnect (true);
    } // switch back to normal wifi
  }
}  



//-------------------------------------------------------------------------------------------------------------------
// auth

/* show/download file */
void webRes(AsyncWebServerRequest *request) {
  String name=webParam(request,"name");
  if(!is(name)) { request->send(403, "text/html"); }
//  else if(onlyImage && (!name.endsWith(".gif") || !name.endsWith(".jpg"))) {  request->send(403, "text/html","not image"); }
  else { request->send(SPIFFS, name);  }
}

//-------------------------------------------------------------------------------------------------------------------


void webSetup() {
  if (!webEnable || _webInit) {
    _webInit = false;
    return;
  }

  // enable auth
  basicAuth.setUsername(user_admin);
  basicAuth.setPassword(eeBoot.espPas);
  basicAuth.setRealm("MyApp");
  basicAuth.setAuthFailureMessage("Authentication failed");
  basicAuth.setAuthType(AsyncAuthType::AUTH_BASIC);
  basicAuth.generateHash();

  webServer.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) { webWifi(request);})
    .addMiddleware(&basicAuth);

  #if webSerialEnable
    // web serial console
    webSerialSetup();
  #endif

  // OTA
  if (updateEnable) {
    webServer.on("/firmware", HTTP_GET, [](AsyncWebServerRequest *request) {
            webUpdate(request);
          })
        .addMiddleware(&basicAuth);
//TODO    if(eeBoot.accessTyper!=ACCESS_ALL) {  .addMiddleware(&basicAuth); }
      
    webServer.on(
            "/doUpdate", HTTP_POST,
            [](AsyncWebServerRequest *request) {},
            [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data,
               size_t len, bool final) {
              webDoUpdate(request, filename, index, data, len, final);
            })
      .addMiddleware(&basicAuth);
    ;
    sprintf(buffer, "WebUpdate started /update");
    logPrintln(LOG_DEBUG,buffer);

#if otaEnable && defined(ESP32) 
    Update.onProgress(webProgress);
#endif

  }

  //File Manager
  webServer.on("/file", HTTP_GET, [](AsyncWebServerRequest *request) {
          webFileManager(request);
        })
    .addMiddleware(&basicAuth);
  
  webServer.on(
          "/doUpload", HTTP_POST,
          [](AsyncWebServerRequest *request) {},[](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data,size_t len, bool final) {
            webFileManagerUpload(request, filename, index, data, len, final);
          })
    .addMiddleware(&basicAuth);
  

  // cmd
  webServer.on("/cmd", HTTP_GET, [](AsyncWebServerRequest *request) { webCmd(request); })
    .addMiddleware(&basicAuth);

  // resources
  webServer.on("/res", HTTP_GET, [](AsyncWebServerRequest *request) { webRes(request); })
    .addMiddleware(&basicAuth);

  // web setupdevice
  webServer.on("/setupDevice", HTTP_GET, [](AsyncWebServerRequest *request) { webSetupDevice(request); });

  //root
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { webRoot(request); });
  webServer.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404);
  });

  // app
  webApp();


  webServer.begin();


  _webInit = true;
  sprintf(buffer, "WEB started %s:%d", WiFi.localIP().toString().c_str(), _webPort); logPrintln(LOG_INFO,buffer);
}

void webLoop() {
  #if webSerialEnable
    if (webEnable && _webInit) {
/*
      webSerial.loop();
*/      
    }
  #endif
}


#else 
  void webLoop() {}  
  void webSetup() {}
#endif


//--------------------------------------------------------------------------------
// telnet

#if telnetEnable

  int telnetPort=23;
//  WiFiServer telnetServer(telnetPort);  // Create server object on port 23
//  WiFiClient telnetClient=NULL;
  NetworkServer telnetServer(telnetPort);  // Create server object on port 23
  NetworkClient telnetClient=NULL;

/*
//TODO #include <SHA.h>
  #include <base64.h>

//  boolean wsHandshakeDone = false;

  String computeAcceptKey(String key) {
    // Magic string to be appended as per WebSocket protocol
    key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    // SHA1 hash
    uint8_t sha1Hash[20];
//TODO    sha1(key.c_str(), key.length(), sha1Hash);

    // Base64 encode
    return base64::encode(sha1Hash, 20);
  }

  void telnetWsHandshake(String key) {
//    if (!wsHandshakeDone) {
//      wsHandshakeDone = true;

      telnetClient.println("HTTP/1.1 101 Switching Protocols");
      telnetClient.println("Upgrade: websocket");
      telnetClient.println("Connection: Upgrade");
      telnetClient.println("Sec-WebSocket-Accept: " + computeAcceptKey(key));
      telnetClient.println();

Serial.println("WebSocket Handshake completed");
  }

  char* telnetReadLine() {
    inIndex=0; boolean end=false;
    while(telnetClient.connected() && !end) {
      if(telnetClient.available()) {
          char c = telnetClient.read();
          if(c == '\r') {}
          else if (c != '\n' &&  inIndex < maxInData-1) { inData[inIndex++] = c; } // read char 
          else { end=true; }
        }
    }
    if(inIndex==0) { return NULL; }    
    inData[inIndex++] = '\0';
    return inData;
  }

  void telnetHttpRequest(char *request) {
Serial.print("REQUEST:");Serial.println(request);        
    char *line=telnetReadLine();
    while(is(line)) {
      line=telnetReadLine();
Serial.print("HEAD:");Serial.println(line);      
    }
Serial.print("BODY:");
    while(telnetClient.connected() && telnetClient.available()) { char c = telnetClient.read(); Serial.print(c); } // simple read available body
Serial.println("REQUEST END");     

  }
*/

  void telnetClientLoop() {
    if(!telnetClient) { // Check for incoming clients       
      telnetClient = telnetServer.accept(); 
      if(telnetClient) {
        sprintf(buffer,"telnetClient connect",telnetPort);logPrintln(LOG_DEBUG,buffer);
        inIndex=0;
      }
    }  
    
    if (!telnetClient) { return ; } // no client
    else if(!telnetClient.connected()) { // client close connection
      logPrintln(LOG_DEBUG,"telnetClient disconnect");
      telnetClient.stop(); telnetClient=NULL;

    }else if(telnetClient.available()) {
      char c = telnetClient.read();      
      if (c != '\n' && c != '\r' && inIndex < maxInData-1) { inData[inIndex++] = c; } // read char 
      else if(inIndex>0) { // RETURN or maxlength 
        inData[inIndex++] = '\0';
        if(equals(inData, "exit")) { 
          logPrintln(LOG_DEBUG,"telnetClient exit");
          telnetClient.stop(); telnetClient=NULL; 
//TODO HTTP        }else if(strncmp(inData, "GET",  3)==0) { telnetHttpRequest(inData); 
       } else {
          char* ret=cmdLine(inData);
          if(is(ret)) { telnetClient.println(ret); }
          inIndex = 0;
        }
      } 
    }
  }

  // log to telnet client
  void telnetLog(char *text) { if(telnetClient) { telnetClient.println(text); } }

  //------------------------------------

  void telnetSetup() {
    sprintf(buffer,"telnetServer start port:%d",telnetPort);logPrintln(LOG_INFO,buffer);    
    telnetServer.begin();
  }

  void telnetLoop() {
    telnetClientLoop();
  }

#else 
  void telnetSetup() {}
  void telnetLoop() {}
  void telnetLog(char *text) {}
#endif


//--------------------------------------------------------------------------------------
// Serial Command Line

unsigned long *cmdTime = new unsigned long(0);
int _skipCmd=0; // >0 => number of cmd to skip
char LINEEND=';';


char prgLine [maxInData]; // prgLine buffer

char* _prg=NULL;
char *_prgPtr=NULL;
boolean prgLog=true; // show each prg step in log
boolean _lastIf=false;

char* appInfo() {
   sprintf(buffer,"AppInfo %s %s CmdOs:%s bootType:%s login:%d access_level:%d",
      prgTitle,prgVersion,cmdOS,bootType,_isLogin,eeBoot.accessLevel); return buffer;
}

// $a =
// attr $a =
// $a CMD

//------------------------------------------------------------------------------------------------

/* convert param to line in buffer */
char* paramToLine(char *param) {
  sprintf(buffer,"%s %s %s %s %s %s %s %s",
    cmdParam(&param),cmdParam(&param),cmdParam(&param),cmdParam(&param),cmdParam(&param),cmdParam(&param),cmdParam(&param),cmdParam(&param)); 

  return buffer;
}

char* cmdInfo() {
  return "?|esp|stat save|load|rset|restart|set[ NAME PAS]|wifi[ SSID PAS]|scan|mqtt[ MQTTURL]|ping IP|DNS IP|sleep[ MODE TIME]|setup|time[ MS]|mode[ MODE]";
}

/* login (isAdmin=true) */
boolean cmdLogin(char *p) {
  if(login(p)) { logPrintln(LOG_SYSTEM,"login admin"); return true; }
  else { logPrintln(LOG_SYSTEM,"logout admin"); return false; }
}

// execute cmd 
char* cmdExec(char *cmd, char **param) {  
  if(!is(cmd)) { return EMPTY; } 
  sprintf(buffer,"->%s %s",cmd,to(*param)); logPrintln(LOG_DEBUG,buffer); 

  if(equals(cmd, "}")) { cmd=nextParam(param); } 

  char *ret= EMPTY;
  if(_skipCmd>0 || cmd[0]=='#') { _skipCmd--;  sprintf(buffer,"skip %s",cmd); logPrintln(LOG_DEBUG,buffer);  ret=cmd; } // skip line or comment   
  else if(cmd[0]=='$') { 
    if(!equals(nextParam(param), "=")) { cmdError("ERROR missing = after attr"); }
    else { ret=cmdSet(cmd,param);  }
  }

  else if(equals(cmd, "?")) { ret=cmdInfo(); }
  else if(equals(cmd, "esp")) { ret=espInfo();  }// show esp status
  else if(equals(cmd, "stat")) { ret=appInfo(); }// show esp status
  
  else if(equals(cmd, "login")) { cmdLogin(cmdParam(param)); }
  else if(equals(cmd, "restart")) { espRestart("cmd restart");  }// restart
  else if(equals(cmd, "sleep")) {  sleep(cmdParam(param),cmdParam(param));  }      // sleep TIMEMS MODE (e.g. sleep 5000 0) (TIMEMS=0=>EVER) (MODE=0=>WIFI_OFF)

  else if(equals(cmd, "attr")) { ret=cmdSet(nextParam(param),param); }
  else if(equals(cmd, "attrDel")) { attrDel(cmdParam(param));  }
  else if(equals(cmd, "attrClear")) { attrClear(cmdParam(param));  }
  else if(equals(cmd, "attrs")) { ret=attrInfo(); }

  else if(equals(cmd, "wait")) { cmdWait(toULong(cmdParam(param))); }// wait for next exec
  else if(equals(cmd, "exec")) { cmdExec(cmdParam(param),param); }// exec 
  else if(equals(cmd, "goto")) { ret=to(cmdGoto(_prg,cmdParam(param))); }// goto prg label or skip n steps
//  else if(equals(cmd, "=")) { ret=cmdSet(cmdParam(param),cmdParam(param),cmdParam(param)); }// return p0p1p2
  else if(equals(cmd, "if")) { ret=to(cmdIf(param));  }// if p0 <=> p2 => { }
  else if(equals(cmd, "elseif")) { ret=to(cmdElseIf(param)); }// else if p0 <=> p2 => { }
  else if(equals(cmd, "else")) { ret=to(cmdElse(param)); }// else => {}
  else if(equals(cmd, "until")) { ret=to(cmdUntil(param)); }// {} until p0 <=> p2 

  else if(equals(cmd, "random")) { int r=random(toInt(cmdParam(param)),toInt(cmdParam(param)));  sprintf(buffer,"%d",r); ret=buffer;  } // random min-max
  else if(equals(cmd,"extract")) { ret=extract(cmdParam(param),cmdParam(param),cmdParam(param)); } // extract start end str (e.g  "free:"," " from "value free:1000 colr:1" => 1000)
  else if(equals(cmd, "reset")) { ret=bootReset(cmdParam(param)); }// reset eeprom and restart    

  else if(equals(cmd, "setupDev") && isAccess(ACCESS_ADMIN)) { ret=setupDev(cmdParam(param)); } // enable/disable setupDevices
  else if(equals(cmd, "setup") && isAccess(ACCESS_ADMIN)) { ret=setupEsp(cmdParam(param),cmdParam(param),cmdParam(param),cmdParam(param),cmdParam(param)); }// setup wifi-ssid wifi-pas espPas => save&restart  

  else if(equals(cmd, "logLevel")) { ret=setLogLevel(toInt(cmdParam(param))); }  // set mode (e.g. "mode NR")
  else if(equals(cmd, "log")) { ret=paramToLine(*param); logPrintln(LOG_INFO,ret); } 

  else if(equals(cmd, "save")) { bootSave();  }// write data to eeprom
  else if(equals(cmd, "load")) { bootRead(); }// load data from eprom
  else if(equals(cmd, "conf")) {  ret=bootSet(cmdParam(param),cmdParam(param),cmdParam(param)); }      // set esp name and password (e.g. "set" or "set NAME PAS")  
  else if(equals(cmd,"access")) { setAccessLevel(toInt(cmdParam(param)));  } // set  AccessLevel (e.g. "access 5")
  else if(equals(cmd, "wifi")) { ret=wifiSet(cmdParam(param),cmdParam(param)); }      // set wifi, restart wifi and info (e.g. "wifi" or "wifi SSID PAS")  
  else if(equals(cmd, "scan")) {  ret=wifiScan(); }         // scan wifi (e.g. "scan")
  else if(equals(cmd, "time")) { ret=timeSet(cmdParam(param),cmdParam(param)); }       // set time (e.g. "time" or "time TIMEINMS")
  else if(equals(cmd, "mode")) { ret=bootMode(toInt(cmdParam(param))); }  // set mode (e.g. "mode NR")
  
  else if(equals(cmd, "ping")) {  ret=cmdPing(cmdParam(param)); }         // wifi ping  (e.g. "ping web.de")
  else if(equals(cmd, "dns")) {  ret=netDns(cmdParam(param)); }         // wifi dns resolve (e.g. "dns web.de")
   
  else if(equals(cmd, "mqttLog") && isAccess(ACCESS_READ)) { eeBoot.mqttLogEnable=toBoolean(cmdParam(param));   } // enable/disbale mqttLog
  else if(equals(cmd, "mqttPublish") && isAccess(ACCESS_USE)) { mqttPublish(cmdParam(param),cmdParam(param));  } // mqtt send topic MESSAGE
  else if(equals(cmd, "mqttConnect") && isAccess(ACCESS_READ)) { mqttOpen(toBoolean(cmdParam(param)));  }
  else if(equals(cmd, "mqttAttr") && isAccess(ACCESS_USE)) { mqttAttr(cmdParam(param),cmdParam(param));  }
  else if(equals(cmd, "mqttDel") && isAccess(ACCESS_USE)) { mqttDel(cmdParam(param));  }
  else if(equals(cmd, "mqtt")) { ret=mqttSet(cmdParam(param));  }      // set mqtt (e.g. "mqtt" or "mqtt mqtt://admin:pas@192.168.1.1:1833") 

  else if(equals(cmd, "params") && isAccess(ACCESS_USE)) { ret=paramsInfo();  }
  else if(equals(cmd, "paramsClear") && isAccess(ACCESS_USE)) { paramsClear(toInt(cmdParam(param)));  }

  else if(equals(cmd, "run")) { ret=cmdFile(cmdParam(param)); } // run prg from file 
  else if(equals(cmd, "end")) { ret=cmdFile(NULL); }// stop prg
  else if(equals(cmd, "stop")) { ret=prgStop(); } // stop/halt prg
  else if(equals(cmd, "continue")) { ret=prgContinue(); } // continue prg
  else if(equals(cmd, "next")) { ret=prgNext(cmdParam(param)); } // next prg step
  else if(equals(cmd, "error")) { cmdError(paramToLine(*param));  }// end prg with error

  else if(equals(cmd, "fsDir") && isAccess(ACCESS_USE)) { ret=fsDir(toString(cmdParam(param))); }
  else if(equals(cmd, "fsDirSize") && isAccess(ACCESS_USE)) { int count=fsDirSize(toString(cmdParam(param))); sprintf(buffer,"%d",count); ret=buffer; }
  else if(equals(cmd, "fsFile") && isAccess(ACCESS_USE)) { ret=fsFile(toString(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); }
  else if(equals(cmd, "fsCat") && isAccess(ACCESS_READ)) { fsCat(toString(cmdParam(param)));  }
  else if(equals(cmd, "fsWrite") && isAccess(ACCESS_CHANGE) ) { boolean ok=fsWrite(toString(cmdParam(param)),cmdParam(param)); }
  else if(equals(cmd, "fsDel") && isAccess(ACCESS_CHANGE)) { fsDelete(toString(cmdParam(param))); }
  else if(equals(cmd, "fsRen") && isAccess(ACCESS_CHANGE)) { fsRename(toString(cmdParam(param)),toString(cmdParam(param)));  }  
  else if(equals(cmd, "fsFormat") && isAccess(ACCESS_ADMIN)) { fsFormat();  }

  else if(equals(cmd, "fsDownload") && isAccess(ACCESS_CHANGE)) { ret=fsDownload(toString(cmdParam(param)),toString(cmdParam(param)),toInt(cmdParam(param))); }
  else if(equals(cmd, "rest") && isAccess(ACCESS_USE)) { ret=rest(cmdParam(param)); } // 
  else if(equals(cmd, "cmdRest") && isAccess(ACCESS_USE)) { ret=cmdRest(cmdParam(param)); } // call http/rest and exute retur nbody as cmd

  else if(equals(cmd, "ledInit") && isAccess(ACCESS_ADMIN)) { ret=ledInit(toInt(cmdParam(param)),toBoolean(cmdParam(param))); } 
  else if(equals(cmd, "led") && isAccess(ACCESS_USE)) { ret=ledSwitch(cmdParam(param),cmdParam(param)); }
  else if(equals(cmd, "swInit") && isAccess(ACCESS_ADMIN)) { ret=swInit(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); } //
  else if(equals(cmd, "swCmd") && isAccess(ACCESS_USE)) { ret=swCmd(toInt(cmdParam(param)),cmdParam(param)); }

  // timer 1 0 -1 -1 -1 -1 -1 "drawLine 0 0 20 20 888"
  else if(equals(cmd, "timer") && isAccess(ACCESS_USE)) { timerAdd(toBoolean(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),cmdParam(param));  }
  else if(equals(cmd, "timerDel") && isAccess(ACCESS_USE)) { timerDel(toInt(cmdParam(param)));  }
  else if(equals(cmd, "timerGet") && isAccess(ACCESS_USE)) { 
      MyEventTimer* timer=(MyEventTimer*)eventList.get(toInt(cmdParam(param))); 
      if(timer!=NULL) { ret=timer->info(); } 
    }
  else if(equals(cmd, "timers") && isAccess(ACCESS_USE)) { timerLog();  }

  else { ret=appCmd(cmd,param); }

  logPrintln(LOG_DEBUG,ret);  // show return
  return ret;
}


//------------------------------------------------------------------------------------------------

unsigned long *_prgTime = new unsigned long(0);
unsigned long _cmdWait= -1; // wait before next cmd time in ms (-1=do not wait, 1000=1s)

/* wait in prg for time in ms, until next step */
void cmdWait(unsigned long cmdWait) { _cmdWait=cmdWait; *_prgTime=1; }


void cmdError(char *error) {  
  logPrintln(LOG_ERROR,error);
//  _prgptr=NULL; 
  *_prgTime=2; // stop program
}

/* end of line (NULL = no line)*/
char* lineEnd(char* ptr) {
  if(ptr==NULL || *ptr=='\0') { return NULL; }   
  while(*ptr!=';' && *ptr!='\n' && *ptr!='\r' && *ptr!='\0') { 
    ptr++;   
  }
  return ptr;
} 


/* goto key in prg */
boolean cmdGoto(char *findPtr,char *p0) { 
  if(findPtr==NULL) { cmdError("ERROR findPtr missing"); return false; } 
  else if(!is(p0)) { sprintf(buffer,"ERROR goto dest missing"); cmdError(buffer); return false; } 
  if(isInt(p0)) { _skipCmd=toInt(p0); return true;}

  char *find=findPtr;
  while(find!=NULL) {    
    if(startWith(find,p0)) { _prgPtr=find; return true;  }    
    find=lineEnd(find); 
    if(find!=NULL) { find++; }
  }  
  sprintf(buffer,"ERROR goto '%s' missing",p0); cmdError(buffer); return false;
}

/* find up start "{" of {..} in prg, start at prgPtr */
char* subStart(char *prgPtr) { 
  int sub=0;
  while(prgPtr!=_prg) { 
    if(*prgPtr=='{') { sub--; if(sub==0) { return prgPtr; }  }
    else if(*prgPtr=='}') { sub++; }
    prgPtr--;
  }
  return NULL;
} 

/* find down end "} of {..} in prg, start at prgPtr */
char* subEnd(char *prgPtr) { 
  if(prgPtr==NULL) { return NULL; }
  int sub=0;
  while(*prgPtr!='\0') { 
    if(*prgPtr=='{') { sub++; }
    else if(*prgPtr=='}') { if(sub==0) { return ++prgPtr; }  else if(sub>0) { sub--; } }
    prgPtr++;
  }
  return NULL;
} 

/* goto/skip until */
boolean gotoSubEnd(char *prgPtr) {
  char *end=subEnd(prgPtr);
  if(end==NULL) { cmdError("ERROR subEnd not found"); return false; } else { _prgPtr=end; return true;}
}

/* start sub on "ok" otherweise skip until }  */
boolean startSub(char *param,boolean ok) {  
  char *cmdOnTrue=nextParam(&param);

  if(equals(cmdOnTrue,"{")) {     
    if(ok)  { return true; } // on true => execute next line 
    else { return gotoSubEnd(_prgPtr); }
  }else {
    if(ok) { char* ret=cmdExec(cmdOnTrue,&param); return true; } // on true => execute cmd 
    else { return false; }
  }
}

/* repeat last sub  }  */
boolean repeatSub(char *param) {
  char *start=subStart(_prgPtr);
  if(start==NULL) { cmdError("ERROR repeatSub not found"); return false; } else { _prgPtr=start; return true;}
}



/* if RULE {} or else cmd */
boolean cmdUntil(char **param) {
  boolean ok=toBoolean(calcParam(param));
  if(!ok) { return repeatSub(*param); }
  else { return false; }
}

/* if RULE {} or else cmd */
boolean cmdIf(char **param) {  
  _lastIf=toBoolean(calcParam(param));
  return startSub(*param,_lastIf);
}

/* elseif RULE {} or else cmd */
boolean cmdElseIf(char **param) {  
  if(!_lastIf) { 
    _lastIf=toBoolean(calcParam(param));
}
  return startSub(*param,_lastIf);
}

/* else {} or else cmd */
boolean cmdElse(char **param) {  
  boolean startElse=!_lastIf; if(!_lastIf) { _lastIf=true; }  
  return startSub(*param,startElse);
}

//------------------------------------



boolean pIsNumber(char *param) {
  if(param==NULL) { return false; }
  else if(*param>='0' && *param<='9') { return true; }
  else if(*param=='+' || *param=='-') { return true; }
  else { return false; }
}

boolean pIsCalc(char *param) {
  if(param==NULL) {   
    return false; }
  if(*param=='<' || *param=='>' || *param=='=' || *param=='!') { return true; }
  else if(*param=='&' || *param=='|' ) { return true; }
  else if((*param=='+' || *param=='-') && (*(param+1)<'0' || *(param+1)>'9')) { return true; } // +- without number (e.g. -1) 
  else if(*param=='*' || *param=='/') { return true; }
  else { return false; }
}

//------------------------------------

/* set-attr KEY VALUE (e.g. $a = ..  or attr $a ... )  
    return VALUE or attrInfo (if KEY missing) 
*/
char* cmdSet(char *key,char **param) {
  char* ret=cmdParam(param);
  if(key!=NULL) {  attrSet(key,ret);  }   
  return ret;
}



//--------------------------------

int xCalc(int ai,char *calc,char **param) {   
  if(!is(calc)) { return ai;}
  else if(equals(calc,"++")) { return ai+1; }
  else if(equals(calc,"--")) { return ai-1; }

  int ret=0;
  char* bb=cmdParam(param);
  int bi=toInt(bb);   
  if(equals(calc,"==")) { ret=ai==bi; }
  else if(equals(calc,"!=")) { ret=ai!=bi; }
  else if(equals(calc,">")) { ret=ai>bi; }
  else if(equals(calc,">=")) { ret=ai>=bi; }
  else if(equals(calc,"<")) { ret=ai<bi; }
  else if(equals(calc,"<=")) { ret=ai<=bi; }

  else if(equals(calc,"+")) { ret=ai+bi; }
  else if(equals(calc,"-")) { ret=ai-bi; }
  else if(equals(calc,"*")) { ret=ai*bi; }
  else if(equals(calc,"/")) { ret=ai/bi; }

  else if(equals(calc,"||")) { ret=ai || bi; }
  else if(equals(calc,"&&")) { ret=ai && bi; }

  else if(equals(calc,">>")) { ret=ai >> bi; }
  else if(equals(calc,"<<")) { ret=ai << bi; }

  else { sprintf(buffer,"ERROR unkown calc '%s'",calc); cmdError(buffer); ret=0; }
  return ret;
}

int calcParam(char **param) { return calcParam(cmdParam(param),param); }
int calcParam(char *val,char **param) {
  int a=toInt(val);
  paramSkipSpace(param); // skip spaces
  while(pIsCalc(*param))  {    
    char *calc=cmdParam(param);       
    a=xCalc(a,calc,param);
    paramSkipSpace(param); // skip spaces
  }
  return a;
}

/* read next param */
char* cmdParam(char **pp) {
    char *p1=paramNext(pp," ");
    if(p1==NULL) { return EMPTY; }

//Serial.print("a:");Serial.println(p1);
    if(*p1=='$') { // attribute
      p1++; // skip first $
//Serial.print("b:");Serial.println(p1);      
      p1=attrGet(p1);
//Serial.print("c:");Serial.println(p1);            
    }else if(*p1=='~') { // sysAttribute
      p1++; // skip first $
      p1=sysAttr(p1);
    }else if(pIsNumber(p1) || pIsCalc(p1)) {
    }else {   
      if(is(p1)) { p1=cmdExec(p1, pp); }
    }

    if(p1==NULL) { return EMPTY; } 
    paramSkipSpace(pp); // skip spaces
    if(pIsCalc(*pp)) { // next param calc 
      sprintf(buffer,"  calc after %s",p1); logPrintln(LOG_DEBUG,buffer);
      int ret=calcParam(p1,pp);
      sprintf(paramBuffer,"%d",ret);
      p1=paramBuffer;
    }
    return p1;
}

char* nextParam(char **pp) {
    if(pp==NULL || *pp==NULL || **pp=='\0') { return EMPTY; }
    while(**pp==' ' || **pp=='\t' || **pp=='}') { (*pp)++; } // skip spaces and tabs
    if(pp==NULL || *pp==NULL || **pp=='\0') { return EMPTY; }
    char* cmd = strtok_r(NULL, " ",pp); 
    if(cmd==NULL) { return EMPTY; } else { return cmd; }
}

/* parse and execute a cmd line */
char* cmdLine(char* line) {  
  char* cmd=nextParam(&line);
  return cmdExec(cmd,&line);
}

//--------------------------------

/* get next line as a copy of prg  */
char *nextCmd() {
  if(_prgPtr==NULL) { return NULL; }

  char *line_end=lineEnd(_prgPtr);
  if(line_end==NULL)  { return NULL; }
  int len=line_end-_prgPtr;
  if(len<=0) { _prgPtr+=len+1; return EMPTY; }
  memcpy( prgLine, _prgPtr, len); prgLine[len]='\0'; 
  _prgPtr+=len; // set next pos
  return prgLine;
}

/* loop next prg line */
char* prgLoop() {
  if(_prg==NULL || _prgPtr==NULL) { return "prg missing"; }
  char *line = nextCmd();  
  while(line==EMPTY) { line = nextCmd(); } // skip empty lines 
  if(line!=NULL) {  return cmdLine(line); }
  else {  
    logPrintln(LOG_DEBUG,"PRG end"); 
    if(_prg!=NULL) { delete[] _prg; _prg=NULL; _prgPtr=NULL; }  
    return "prg end";  
  }
}

/* next n steps */
char* prgNext(char *p0) {
  int steps=toInt(p0);
  if(steps<=1) { return prgLoop(); }

  char* ret=EMPTY;  
  int count=0;
  while(_prgPtr!=NULL && count++<steps) {
    ret=prgLoop();
    if(prgLog){ logPrintln(LOG_DEBUG,ret); }
  }
  return ret;
}  

char* prgContinue() { *_prgTime=0; return "continue";}
char* prgStop() { *_prgTime=2; return "stop"; }

char* cmdPrg(char* prg) {   
  if(_prg!=NULL) { delete[] _prg; _prg=NULL; _prgPtr=NULL; } // clear old prg
  if(!is(prg)) { return "prg missing"; }
  _prg=prg;
  _prgPtr=_prg;  // set ptr to prg start
  *_prgTime=0; // set prgTime to run
  return prgLoop();   
}

char* cmdFile(char* p0) {
  if(_prg!=NULL) { delete[] _prg; _prg=NULL; _prgPtr=NULL; } // clear old prg
  if(!is(p0)) { return "end"; }

  String name=toString(p0);  
  char* prg = fsRead(name);
  if(prg==NULL) { return "cmdFile missing "; }
  else { char* ret=cmdPrg(prg); return ret; }
}

//-----------------

/* call http/rest and execute return body as cmd */
char* cmdRest(char *url) {
  char* ret=rest(toString(url));  
  if(!is(ret)) { return NULL; } 
  sprintf(buffer,"cmdRest %s",ret); logPrintln(LOG_DEBUG,buffer);
  return cmdPrg(ret);
}

//-----------------

// read serial in as cmd
void cmdRead() {
    if (Serial.available() > 0) {
    char c = Serial.read();
    if (c != '\n' && c != '\r' && inIndex < maxInData-1) { inData[inIndex++] = c; } // read char 
    else if(inIndex>0) { // RETURN or maxlength 
      inData[inIndex++] = '\0';
      char* ret=cmdLine(inData);
      if(logLevel!=LOG_DEBUG) { logPrintln(LOG_SYSTEM,ret); }      
      inIndex = 0;
    }
  }
}

void cmdRead(char c) {

}

//--------------------------

// exec "startup.cmd" afer 10s
unsigned long *cmdStartTime = new unsigned long(1);
int startupWait=10000; // wait before startup.cmd

void cmdLoop() {
  // serial in
  if(serialEnable && isTimer(cmdTime, 10)) { cmdRead(); } // exec cmd 
  // prg
  if(eeMode<EE_MODE_SYSERROR) {
    if(_prgPtr!=NULL && isTimer(_prgTime, _cmdWait)) {  _cmdWait=0; prgLoop();  } 
  }
  // statup 
  if(eeMode==EE_MODE_START || eeMode==EE_MODE_OK) {
    if(_prg==NULL && isTimer(cmdStartTime, 10000)) { 
      if(fsSize("/startup.cmd")>0) { cmdFile("/startup.cmd");  } // exec startup-file cmd 
      *cmdStartTime=2;
    }
  }
}


//--------------------------------------------------------------------------------------
// Setup/Loop

void cmdOSSetup() {
  if(serialEnable) { 
    delay(1); Serial.begin(115200); 
    delay(1); Serial.println("----------------------------------------------------------------------------------------------------------------------");
  }
  freeHeapMax=ESP.getFreeHeap(); // remeber max freeHeap
  logPrintln(LOG_INFO,espInfo()); // ESP infos
  eeSetup();
  ledSetup();  
  swSetup(); 
  bootSetup();
  
  fsSetup();

  if(isModeNoSystemError()) {
    wifiSetup();  
    otaSetup();
    telnetSetup();
  }

  if(isModeOk()) {
    mqttSetup();  
    timeSteup();  
  }
}

void cmdOSLoop() {
  eeLoop();
  ledLoop(); 
  swLoop(); 
  cmdLoop();
  timeLoop();

  if(isModeNoSystemError()) {
    wifiLoop();
    otaLoop();
    telnetLoop();
  }

  if(isModeOk()) {
    mqttLoop();
    webLoop();
    timeLoop();
  }
  delay(0);
}


