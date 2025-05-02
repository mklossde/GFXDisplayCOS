
//---------------------------------------------------------------
// Measure

/* draw full-bar of value% of max% (e.g. 5 10 => half) e.g. drawFull 10 40 5 20 1 10 50 888 2016 */
void drawFull(int x,int y,int w,int h,int p,int value,int max,int col1,int col2) {
  if(col1==-1) { col1=_color; }  if(col2==-1) { col2=_color; }  
  drawRect(x,y,w,h,col1);  
  if(w>h) { 
    float step=(float)w/(float)max; int w2=(int)((float)value*step);
    if(w2>p*2) {fillRect(x+p,y+p,w2-(p*2),h-(p*2),col2); }
  } else { 
    float step=(float)h/(float)max; int w2=(int)((float)value*step);
    if(w2>p*2) { fillRect(x+p,y+p+h-w2, w-(p*2),w2-(p*2),col2); }
  } 
}

/* draw full-bar of value% of max% (e.g. 5 10 => half) e.g. drawFull 10 40 5 20 1 10 50 888 2016 */
void drawBar(int x,int y,int w,int h,int p,char *value,char *min,char *med,char *max,int col1,int col2,int col3) {
  int minInt=toInt(min),valueInt=toInt(value),maxInt=toInt(max)-minInt;
  int medInt=toInt(med);
    
  int colVal=col2,c=col2; if(medInt>0 && valueInt>=medInt) { c=col3; colVal=col1; }
  drawRect(x,y,w,h,col1);  
  if(w>h) { 
    float step=(float)w/(float)maxInt; int w2=(int)((float)valueInt*step);
    if(w2>p*2) {fillRect(x+p,y+p,w2-(p*2),h-(p*2),c); }
    if(medInt>0) {
      int medX=step*(float)medInt;
      drawLine(medX,y+1,medX,y+h-1,col1);
    }

    drawText(x,y+h+5,1,min,col1,-1);
    drawText(x+w,y+h+5,1,max,col1,1);
    int colVal=col1; if(medInt>0 && valueInt>=medInt) { colVal=col3; }
//    int h2=h/2;
    drawText(x+w/2,y+h+5,fontSize,value,colVal,0); 
        
  } else { 
    float step=(float)h/(float)maxInt; int w2=(int)((float)valueInt*step);
    if(w2>p*2) { fillRect(x+p,y+p+h-w2, w-(p*2),w2-(p*2),c); }

  }    
}

/* drawGauge 10 10 5 1 5 10 888 2016 

  d1 = thick circle
  d2 = thick needle
*/
void drawGauge(int x,int y,int w,int d1,int p,char *value,char *min,char *med,char *max,int col1,int col2,int col3) {
//void drawGauge(int x,int y,int w,int d1,int p,char *value,char *min,char *max,int col1,int col2) {
  if(col1==-1) { col1=_color; } 
  int minInt=toInt(min),valueInt=toInt(value),maxInt=toInt(max)-minInt;
  int medInt=toInt(med);
  float angelMax=((float)180/(float)maxInt);
  if(medInt<=0)  {
    drawArc(x,y, 1, 270, 180, w,w, d1, col1);
  }else {
    int angelMed=angelMax*(float)medInt;
    drawArc(x,y, 1, 270, angelMed, w,w, d1, col1);
    drawArc(x,y, 1, 270+angelMed, 180-angelMed, w,w, d1, col3);
  }
  drawLine(x-w,y,x+w,y,col1);
  int v=angelMax*(float)valueInt;
  drawArc(x,y,1,270+v,2,w-p,w-p,w,col2);
  drawText(x-w,y+5,1,min,col1,-1);
  drawText(x+w,y+5,1,max,col1,1);
  int colVal=col1; if(medInt>0 && valueInt>=medInt) { colVal=col3; }
  drawText(x,y+5,fontSize,value,colVal,0);
}

/* draw on button e.g. drawOn 10 10 5 2 1 -1 2016 */
void drawOn(int x,int y,int w,int p,char *value,char *min,char *med,char *max,int col1,int col2,int col3) {
  fillCircle(x,y,w,col1);  
  boolean on=toBoolean(value);
  int minInt=toInt(min),valueInt=toInt(value),maxInt=toInt(max)-minInt;
  int medInt=toInt(med);
  if(medInt>0 && valueInt>medInt) { col2=col3; }
  if(on) { fillCircle(x,y,w-p,col2); }
  drawText(x,y-fontSize*4,fontSize,value,col1,0);
}

//-------------------------------------------

/* draw name+value+full 
void valueFull(int x,int y,int w,char *text,char *value,char *min,char *med,char *max,int col1,int col2,int col3) {
  drawValue(x,y,text,value,min,max,col1,col2);
  int valInt=toInt(value);
  int maxInt=toInt(max);
  if(valInt<0) { col1=col2; valInt=valInt*-1; } 
  drawFull(x+40,y,20,8,1,valInt,maxInt,_color,col1);
}
*/

/* draw name+value+on (on=vlaue>max) 
void valueOn(int x,int y,int w,char *text,char *value,char *min,char *med,char *max,int col1,int col2,int col3) {
  drawValue(x,y,text,value,min,max,col1,col2);
  if(value>max) { col1=col2; }
  drawOn(x+53,y+7,5,1,true,_color,col1);
}
*/

// void drawGauge(int x,int y,int w,int d1,int p,char *value,char *min,char *max,int col1,int col2) {
/* draw name+value+guage 
void valueGauge(int x,int y,int w,char *text,char *value,char *min,char *med,char *max,int col1,int col2,int col3) {
  int w2=w/2;
  drawText(x+w2,y,fontSize,text,col1,0);
  int d1=w/20;
  drawGauge(x+w2,y+w2+fontSize*8+5,w2,d1,1,value,min,med,max,col1,col2,col3);
}
*/

/* draw value with name+value */
void drawValue(int type,int x,int y,int w,char *text,char *value,char *min,char *med,char *max,int col1,int col2,int col3) {  
  int w2=w/2;
  drawText(x+w2,y,fontSize,text,col1,0);

  if(type==0) { 
    if(col1<0) { col1=12678;}if(col2<0) { col2=63488;}if(col3<0) { col3=43136;}
    int d1=w/20;
    drawGauge(x+w2,y+w2+fontSize*8+5,w2,d1,1,value,min,med,max,col1,col2,col3);
  }else if(type==1) { 
    drawBar(x,y+fontSize*8+5,w,w/4,2,value,min,med,max,col1,col2,col3);
  }else {

    drawOn(x+w2,y+w2,w2,4,value,min,med,max,col1,col2,col3);
  }
}

