      


//------------------------------------------------------------
// web

char* cmdSetMatrix(char* p0,char* p1,char* p2,char* p3,char* p4,char* p5) {  
  if(is(p0)) { 
    if(!isAccess(ACCESS_ADMIN))  { return "ACCESS DENIED setMatrix"; }
    if(is(p0,1,3)) { eeDisplay.pX=toInt(p0); }
    if(is(p1,1,3)) { eeDisplay.pY=toInt(p1); }
    if(is(p2,1,2)) { eeDisplay.panelChain=toInt(p2); }  
    if(is(p3,1,3)) { eeDisplay.brightness=toInt(p3);}
    if(is(p4,1,2)) { eeDisplay.rotation=toInt(p4);}
    if(is(p5,1,64)) { strcpy(eeDisplay.pins,p5); }
    displaySave();
  }
  return displayInfo();
}

char* cmdSetMatrix2(boolean dmaBuffer, boolean displayBuffer,int latBlanking,boolean clkphase,char *driver) {
  eeDisplay.dmaBuffer=dmaBuffer;
  eeDisplay.displayBuffer=displayBuffer;
//  eeDisplay.latBlanking=latBlanking;
//  eeDisplay.clkphase=clkphase;
//  if(is(driver)) { eeDisplay.driver=copy(driver); }
  displaySave();
  return displayInfo();
}

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
  File root = FILESYSTEM.open(rootDir);
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


//------------------------------------------------------------
// cmd

char* matrixCmd(char *cmd, char **param) {
    // drawOff => switch display off by clear and stop all
    if(equals(cmd, "drawOff")) { drawOff(); return EMPTY; }
    // drawClear => clear display
    else if(equals(cmd, "drawClear")) { drawClear(); return EMPTY; } 
    // drawStop => stop all (not clear)
    else if(equals(cmd, "drawStop")) { drawFileClose(); return EMPTY; } 
    // draw => draw buffer, draw content from buffer on display or flip dma buffer
    else if(equals(cmd, "draw")) { draw(); return EMPTY; } 

    // brightness n - set up brightness of dislpay
    else if(equals(cmd, "brightness")) { int b=toInt(cmdParam(param)); displayBrightness(b); return EMPTY; }
    else if(equals(cmd, "rotation")) { int b=toInt(cmdParam(param)); return displayRotation(b); }

    //drawColor r g b - calculate 444 color and set as default color    
    // default color for all draw-commands (use by draw command if no or -1 is given as color)
    else if(equals(cmd, "drawColor")) { uint16_t col=toColor444(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)));drawColor(col); sprintf(buffer,"%d",col); return buffer; }
    // drawColor565 r g b - calculate 565 color and set as default color 
    else if(equals(cmd, "drawColor565")) { uint16_t col=toColor565(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); drawColor(col); sprintf(buffer,"%d",col); return buffer; }

    //fillScreen c - fill screen with color	
    else if(equals(cmd, "fillScreen")) { fillScreen(toInt(cmdParam(param))); return EMPTY; }
    // drawPixel x y c - draw a pixel at x y
    else if(equals(cmd, "drawPixel")) { drawPixel(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }
    // drawLine x y x2 y2 c - draw a line (from x,y to x2,y2)
    else if(equals(cmd, "drawLine")) { drawLine(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }
    // draw line with thick 
    else if(equals(cmd, "drawWLine")) { drawLine(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }
    // rawRect x y w h c - draw rect (rect from x,y to x+w,y+h)
    else if(equals(cmd, "drawRect")) { drawRect(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }
    // fillRect x y w h c - draw a filled rect
    else if(equals(cmd, "fillRect")) { fillRect(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }
    // drawTriangle x y x2 y2 x3 y3 c - draw a trinagle (from x,y to x2,y2 to x3,y3 to x,y)
    else if(equals(cmd, "drawTriangle")) { drawTriangle(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }
    // fillTriangle x y x2 y2 x3 y3 c -  draw a filled triangle
    else if(equals(cmd, "fillTriangle")) { fillTriangle(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }

   // rawRect x y w h c - draw rect (rect from x,y to x+w,y+h)
    else if(equals(cmd, "drawRoundRect")) { drawRoundRect(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }
    // fillRect x y w h c - draw a filled rect
    else if(equals(cmd, "fillRoundRect")) { fillRoundRect(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }
 


    // drawCircle x y w c - draw circle at x y with radius w 
    else if(equals(cmd, "drawCircle")) { drawCircle(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }
    // fillCircle x y w c - draw filled circle at x y with radius w 
    else if(equals(cmd, "fillCircle")) { fillCircle(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }
    // drawArc x y angel seg rx ry w c - draw a segment arc at x,y start at angel (0=top), end after seg, with radiusX and radiusY and thikness w 
    else if(equals(cmd, "drawArc")) { drawArc(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }
    // drawSegment s i - set segemnt step (segmentStep=6,segmentInc=6) segmentStep=6 => full arc have 60 segments, 3=120
//    else if(equals(cmd, "drawSegment")) { drawSegment(toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }
    
    // drawFull x y w h p value max, c1,c2 - draw a full-element at x,y with w,h. full=100/max*value will be in color c2 and offset p
    else if(equals(cmd, "drawFull")) { drawFull(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }
    else if(equals(cmd, "drawOn")) { drawOn(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toBoolean(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }
    else if(equals(cmd, "drawGauge")) { drawGauge(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }

    // valueFull x y value max c1 c2 - show value with name at full
    else if(equals(cmd, "valueFull")) { valueFull(toInt(cmdParam(param)),toInt(cmdParam(param)),cmdParam(param),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }
    else if(equals(cmd, "valueOn")) { valueOn(toInt(cmdParam(param)),toInt(cmdParam(param)),cmdParam(param),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }
    else if(equals(cmd, "valueGauge")) { valueGauge(toInt(cmdParam(param)),toInt(cmdParam(param)),cmdParam(param),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }


    // drawText x y c size text - draw text at x y with size 
    else if(equals(cmd, "drawText")) { drawText(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),cmdParam(param),toInt(cmdParam(param))); return EMPTY; }

    // set page
    else if(equals(cmd, "page")) { int p=pageSet(toInt(cmdParam(param))); sprintf(buffer,"%d",p); return buffer; }
    // next page
    else if(equals(cmd, "pagePriv")) { int p=pageChange(-1);sprintf(buffer,"%d",p); return buffer; }
    // next page
    else if(equals(cmd, "pageNext")) {  int p=pageChange(1);sprintf(buffer,"%d",p); return buffer; }

    // set page
    else if(equals(cmd, "pages")) { return pageList();  }
//    else if(equals(cmd, "pageAdd")) { return pageAdd(cmdParam(param));  }
    else if(equals(cmd, "pageDel")) { return pageDel(toInt(cmdParam(param)));  }

    // drawFile file type x y - draw a gif/icon at x,y
    else if(equals(cmd, "drawFile")) { char *f=cmdParam(param); drawFile(f,f,toInt(cmdParam(param)),toInt(cmdParam(param)),false); return EMPTY; }    
    // drawUrl url x y - draw content of url (gif/icon) at a x 
    else if(equals(cmd, "drawUrl")) { drawUrl(toString(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),false); return EMPTY; }

    /* write wifi icon as wifi.bm1 file 
    else if(equals(cmd, "writeIcon")) { 
      uint8_t* uint8Ptr = (uint8_t*)reinterpret_cast<const uint8_t*>(wifi_image1bit);
      fsWriteBin("wifi.bm1",uint8Ptr,sizeof(wifi_image1bit)); return EMPTY; 
    } 
    */   
    // drawIcon x y w h c file - draw icon (bm1) at x,y with w,h of color 
    else if(equals(cmd, "drawIcon")) { drawIcon(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),cmdParam(param)); return EMPTY; }
  
    // drawTime x y c - draw time (format hh:mm:ss) at x,y of color c 
    else if(equals(cmd, "drawTime")) { drawTime(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }
    // drawDate x y c - draw date (format dd.mm.yyyy) at x,y of color c
    else if(equals(cmd, "drawDate")) { drawDate(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY; }

    // effect type step speed a b - start effect type with n-steps with speed in ms between steps
    else if(equals(cmd, "effect")) { effectStart(toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param)),toInt(cmdParam(param))); return EMPTY;  } // start effect 

    // matrix sizeX sizeY chain brightness rotation pins - config hub75 dislpay/connection
	  // e.g. matrix 64 64 1 90 0 0,15,4,16,27,17,5,18,19,21,12,33,25,22
    else if(equals(cmd, "matrix")) { return cmdSetMatrix(cmdParam(param),cmdParam(param),cmdParam(param),cmdParam(param),cmdParam(param),cmdParam(param));  }
    // buffer dmaBuffer displayBuffer - (0=off/1=on) enable dmsBuffer or displayBuffer 
    else if(equals(cmd, "matrix2")) { return cmdSetMatrix2(toBoolean(cmdParam(param)),toBoolean(cmdParam(param)),toInt(cmdParam(param)),toBoolean(cmdParam(param)),cmdParam(param));  }

    else { return cmd; }
}




