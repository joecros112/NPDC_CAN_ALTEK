#include "ClearCore.h"
#define CcioPort ConnectorCOM0
// analog/pwm outputs
#define CL1  IO0 // motor speed (HSPEC1)
#define CL2  IO1 // master fluid valve (PV1)
//solenoid outputs on controller
#define SV2_S1  IO2 // SLED CLAMPS open
#define SV2_S2  IO3 // SLED CLAMPS close
#define SV3_S1  IO4 // SIDE CLAMPS open
#define SV3_S2  IO5 // SIDE CLAMPS close
//ccio-8 expansion pins
#define SV4_S1  CLEARCORE_PIN_CCIOA0 // MIDDLE CLAMP open
#define SV4_S2  CLEARCORE_PIN_CCIOA1 // MIDDLE CLAMP close
#define SV5_S1  CLEARCORE_PIN_CCIOA2 // PRESSURE CYLINDERS open
#define SV5_S2  CLEARCORE_PIN_CCIOA3 // PRESSURE CYLINDERS close
//#define BRAKE   CLEARCORE_PIN_CCIOA4 // BRAKE MOTORS
//inputs
#define Switch1 CLEARCORE_PIN_CCIOA4
#define Switch2 CLEARCORE_PIN_CCIOA5
#define EnterSwitch CLEARCORE_PIN_CCIOA6
#define START_BTN   DI6
#define SPEED_POT   A9
#define TIME_POT    A10
//constant


void startUP(){ //1. Startup, set CL2 to some value
  Serial.println("Startup Sequence Initiated"); 
  //digitalWrite(CL2, true); // need to set CL2 to some value
  Serial.println("Startup Sequence Complete");
}

void situatePipe(){ //2. User input toggle SV3:S1:S2 to situate pipe
  
Serial.println("Situate The Pipe:");
Serial.print("Press 1 to toggle SV3:S1 \nPress 2 to toggle SV3:S2\nPress f to finish\nPress r to reset\n");
while(true){
while(digitalRead(Switch1)){
  digitalWrite(SV3_S1, true);
}
while(digitalRead(Switch2)){
  digitalWrite(SV3_S2, true);
}
digitalWrite(SV3_S1, false);
digitalWrite(SV3_S2, false);
if(digitalRead(EnterSwitch)){
  delay(3000);
  break;
  
}
}
}
  

void situateFitting_centerClamp(){//User input toggle SV4:S1:S2
  Serial.println("Situate the Center Fitting:");
  Serial.print("Press 1 to toggle SV4:S1 \nPress 2 to toggle SV4:S2\nPress f to finish\nPress r to reset\n");
  while(true){
    while(digitalRead(Switch1)){
      digitalWrite(SV4_S1, true);
}
    while(digitalRead(Switch2)){
      digitalWrite(SV4_S2, true);
}
    digitalWrite(SV4_S1, false);
    digitalWrite(SV4_S2, false);
    if(digitalRead(EnterSwitch)){
      digitalWrite(SV4_S1, false);
      digitalWrite(SV4_S2, false);
      delay(3000);
      break;
      
}
}
}
 
void situateFitting_sledClamp(){
  Serial.println("Situate the Sled Clamp Fitting");
  Serial.print("Press 1 to toggle SV2:S1 \nPress 2 to toggle SV2:S2\nPress f to finish\nPress r to reset\n");
  while(true){
while(digitalRead(Switch1)){
  digitalWrite(SV2_S1, true);
}
while(digitalRead(Switch2)){
  digitalWrite(SV2_S2, true);
}
digitalWrite(SV2_S1, false);
digitalWrite(SV2_S2, false);
if(digitalRead(EnterSwitch)){
  digitalWrite(SV4_S1, false);
  digitalWrite(SV4_S2, false);
  delay(3000);
  break;
}
}
}
  

void variableInput(){
  Serial.print("Please enter the input runtime: ");
  int inputRunTime = 2000;
  Serial.println("");
}

void startCycle(){

  Serial.println("Cycle Ready to Initiate");
  Serial.print("Press enter to start: ");
while(true){
    if (digitalRead(EnterSwitch)){
      digitalWrite(SV2_S1, true);
      digitalWrite(SV2_S2, true);
      digitalWrite(SV4_S1, true);
      delay(2000);
      digitalWrite(CL1, true);//FIRE CL1 here
      digitalWrite(CL2, true);//FIRE CL2 here
      delay(2000);
      digitalWrite(SV5_S1, true); 
      //POSSIBLY fire SV5_S2 here;
      delay(2000); //VARIALBE RUNTIME HERE
      digitalWrite(CL1, false);
      digitalWrite(CL2, false);//STOP CL1, CL2
      //digitalWrite(BRAKE, true);
      delay(5000);
      digitalWrite(SV5_S1, false);
      digitalWrite(SV2_S1, false);
      digitalWrite(SV3_S1, false);
      digitalWrite(SV4_S1, false);
      digitalWrite(SV2_S2, false);
      digitalWrite(SV3_S2, false);
      digitalWrite(SV4_S2, false);
      delay(1000);
      digitalWrite(SV5_S2, true);
      delay(1000);
      digitalWrite(SV5_S2, false);
      Serial.println("Cycle Complete");
      delay(3000);
      break;
      

}
}}

void brake(){
Serial.println("Initiating Braking in 3... 2... 1...");
Serial.println("Braking Complete");
}

void setup() {
Serial.begin(9600); //baud rate
CcioPort.Mode(Connector::CCIO); //Connect Expansion Board
CcioPort.PortOpen();
    //MAINBOARD
    pinMode(CL1, OUTPUT);
    pinMode(CL2, OUTPUT);
    pinMode(SV2_S1, OUTPUT);
    pinMode(SV2_S2, OUTPUT);
    pinMode(SV3_S1, OUTPUT);
    pinMode(SV3_S2, OUTPUT);
    //EXPANSIONBOARD
    pinMode(SV4_S1, OUTPUT);
    pinMode(SV4_S2, OUTPUT);
    pinMode(SV5_S1, OUTPUT);
    pinMode(SV5_S2, OUTPUT);
    pinMode(Switch1, INPUT);
    pinMode(Switch2, INPUT);
    pinMode(EnterSwitch, INPUT);
    


}

void loop() {
  Serial.println("");

    startUP();
    Serial.println(".........................................................");
    situatePipe();
    Serial.println(".........................................................");
    situateFitting_centerClamp();
    Serial.println(".........................................................");
    situateFitting_sledClamp();
    Serial.println(".........................................................");
    variableInput();
    Serial.println(".........................................................");
    startCycle();
    Serial.println(".........................................................");
    brake();
    Serial.println(".........................................................");
    loop();
    

}
    


