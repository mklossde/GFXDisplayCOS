
boolean enableSW=true;
int sw=32;

void setup() {
  delay(1); Serial.begin(115200); 
  delay(1); Serial.println("----------------------------------------------------------------------------------------------------------------------");

  if(enableSW) { pinMode(sw, INPUT_PULLDOWN); }
   
}

void loop() {
  if(enableSW) { 
//    byte swNow=digitalRead(sw);
    byte swNow=touchRead(sw);
    Serial.print("Button");Serial.println(swNow);
  }
  delay(1000);
}
