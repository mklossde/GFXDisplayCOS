

//------------------------------------------------------------
// web

#if webEnable

void matrixWebSetupSet(AsyncWebServerRequest *request) {
  String v;
  v=webParam(request,"pixelX"); if(is(v,1,5)) {  eeDisplay.pX=v.toInt(); }
  v=webParam(request,"pixelY"); if(is(v,1,5)) {  eeDisplay.pY=v.toInt(); }
  v=webParam(request,"chain"); if(is(v,1,2)) {  eeDisplay.panelChain=v.toInt(); }
  v=webParam(request,"pins"); if(is(v,1,64)) {  v.toCharArray(eeDisplay.pins, sizeof(eeDisplay.pins)); }
  v=webParam(request,"brightness"); if(is(v,1,4)) {  eeDisplay.brightness=v.toInt(); }
  v=webParam(request,"rotation"); if(is(v,1,2)) {  eeDisplay.rotation=v.toInt(); }
  displaySave();
} 

void matrixWebSetup(AsyncWebServerRequest *request) {
  String message; if (request->hasParam("ok")) { matrixWebSetupSet(request); message="set"; }

  String html = ""; html = pageHead(html, "MatrixHup");
  html+= "[<a href='/config'>network</a>][<a href='/appSetup'>app</a>]";
  html = pageForm(html, "MatrixHub75 config");
  html = pageInput(html, "pixelX", to(eeDisplay.pX));
  html = pageInput(html, "pixelY", to(eeDisplay.pY));
  html = pageInput(html, "chain", to(eeDisplay.panelChain));
  html = pageInput(html, "pins", eeDisplay.pins);
  html = pageInput(html, "brightness", to(eeDisplay.brightness));
  html = pageInput(html, "rotation", to(eeDisplay.rotation));
  html = pageButton(html, "ok", "ok");
  html = pageFormEnd(html);
  html = pageEnd(html,message);
  request->send(200, "text/html", html);
}

void matrixWeb(AsyncWebServerRequest *request) {
  String message;
  if (request->hasParam("drawOff")) {  drawOff();
  }else if (request->hasParam("page")) { 
    String nr=webParam(request,"nr"); 
    pageSet(nr.toInt());
  }else if (request->hasParam("pageNext")) { pageChange(+1);  
  }else if (request->hasParam("pagePriv")) { pageChange(-1);  
  }else if (request->hasParam("drawFile")) { 
    String name=webParam(request,"name");  
    char *file=(char*)name.c_str();
    drawFile(file,file,0,0,true);
  }else if (request->hasParam("drawCmd")) {
    String name=webParam(request,"name");  
    String ret=cmdFile((char*)name.c_str());
  }else if (request->hasParam("drawUrl")) {
    drawUrl(webParam(request,"url"),0,0,true);
  }

  String html = ""; html = pageHead(html, "MatrixHup");
  File root = FILESYSTEM.open(rootDir,"r");
  File foundfile = root.openNextFile();
  html+="[<a href=?page=1&nr=1>Title</a>][<a href=?pageNext=1>NextPage</a>][<a href=?pagePriv=1>PrivPage</a>][<a href=?drawOff=1>OFF</a>]";
  html+="<table><tr>";
  int cols=0;
  while (foundfile) { 
    String name = String(foundfile.name());
    if(name.endsWith(".gif") || name.endsWith(".bm1")) { 
      html += "<td><a href='?drawFile=1&name=" + rootDir + name + "'><img src='/res?name="+ rootDir +name+"' width='64' height='64'/><br>" + name + "</a></td>";
      if(cols++>10) { cols=0; html+="</tr><tr>";}
    }else if(name.endsWith(".cmd")) {
      html += "<td><a href='?drawCmd=1&name=" + rootDir + name + "'>CMD<br>" + name + "</a></td>";
      if(cols++>10) { cols=0; html+="</tr><tr>";}
    }
    foundfile = root.openNextFile();
    
  }
  html+="</tr></table><hr>";
//  html = pageUpload(html, "Draw gif/bm1/cmd", "/drawUpload");
//  html += "<form method='GET'>Draw URL gif/bm1/cmd<input type='text' size='64' name='url'/><input type='submit' name='drawUrl' value='ok'/></form>";
  root.close();
  foundfile.close();


  html = pageEnd(html,message);
  request->send(200, "text/html", html);
}

//---------------------------------------------------------

/* draw upload */
void drawUpload(AsyncWebServerRequest *request, String file, size_t index, uint8_t *data, size_t len, bool final) {
  if(!isWebAccess(ACCESS_READ)) { request->send(403, "text/html"); }
  sprintf(buffer, "upload %s %d index:%d", file.c_str(), len, index);logPrintln(LOG_DEBUG,buffer);

  File ff;
  if (!index) {     
    FILESYSTEM.remove(rootDir + uploadFile); // remove old file 
    ff = SPIFFS.open(rootDir + uploadFile, FILE_WRITE); 
  }else { ff = SPIFFS.open(rootDir + uploadFile, FILE_APPEND); }

  for (size_t i = 0; i < len; i++) { ff.write(data[i]); }
  ff.close();
  if (final) {
    sprintf(buffer, "uploaded %s => %s %d", file.c_str(),uploadFile, (index + len)); logPrintln(LOG_DEBUG,buffer);
    char* name=copy(file); 
    drawFile(uploadFile,name,0,0,true);
    delete name;
    request->redirect("/app");
  }
}


#endif
