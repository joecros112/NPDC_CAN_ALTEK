
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
#define BRAKE   CLEARCORE_PIN_CCIOA4 // BRAKE MOTORS
//inputs
#define START_BTN   DI6
#define SPEED_POT   A9
#define TIME_POT    A10

void startUP(){
  Serial.println("Startup Sequence"); //
//digitalWrite(CL2, true); // need to set CL2 to some value
}

void situatePipe(){ //User input toggle SV3:S1:S2

  Serial.print("Press 1 to toggle SV3:S1 \nPress 2 to toggle SV3:S2\nPress f to finish\n");
  while(true){
  if(Serial.available() > 0){ 
    char command = Serial.read();
    if (command == '1'){ //HOLD 1 to toggle SV3:S1 on
      digitalWrite(SV3_S1, true);
    }
    if (command == '2'){ //HOLD 2 to toggle SV3:S2 on
      digitalWrite(SV3_S2, true);
    }
    if (command == '0'){ //Should default to 0, turn SV3:S1 and SV3:S2 off
      digitalWrite(SV3_S1, false);
      digitalWrite(SV3_S2, false);
    }
    if (command == 'f'){ //done exit
      digitalWrite(SV3_S1, false);
      digitalWrite(SV3_S2, false);
      break;
    }
    if(command != '1' | command != '2' | command != '0' | command != 'f'){ //account for bad keyboard entry
      Serial.println("Invalid Entry");
}}}}

void situateFitting_centerClamp(){//User input toggle SV4:S1:S2

  Serial.print("Press 1 to toggle SV4:S1 \nPress 2 to toggle SV4:S2\nPress f to finish\n");
  while(true){
  if(Serial.available() > 0){
    char command = Serial.read();
    if (command == '1'){
      digitalWrite(SV4_S1, true); //PRESS 1 to toggle SV4:S1 on
    }
    if (command == '2'){
      digitalWrite(SV4_S2, true); //PRESS 2 to toggle SV4:S2 on
    }
    if (command == '0'){ //Default to 0, turns SV4:S1 and SV4:S2 off
      digitalWrite(SV4_S1, false);
      digitalWrite(SV4_S2, false);
    }
    if (command == 'f'){ //Exit with f
      digitalWrite(SV4_S1, false);
      digitalWrite(SV4_S2, false);
      break;
    }
    if(command != '1' | command != '2' | command != '0' | command != 'f'){
      Serial.println("Invalid Entry");
}}}}
void situateFitting_sledClamp(){
  Serial.print("Press 1 to toggle SV2:S1 \nPress 2 to toggle SV2:S2\nPress f to finish\n");
  while(true){
  if(Serial.available() > 0){
    char command = Serial.read();
    if (command == '1'){ //PRESS 1 to toggle SV2:S1 on
      digitalWrite(SV2_S1, true); 
    }
    if (command == '2'){ //PRESS 2 to toggle SV3:S2 on
      digitalWrite(SV2_S2, true);
    }
    if (command == '0'){ //Default to 0, turns SV4:S1 and SV4:S2 off
      digitalWrite(SV2_S1, false); 
      digitalWrite(SV2_S2, false);
    }
    if (command == 'f'){ // Exit with f
      digitalWrite(SV2_S1, false);
      digitalWrite(SV2_S2, false);
      break;
    }
    if(command != '1' | command != '2' | command != '0' | command != 'f'){
      Serial.println("Invalid Entry");
}}}}

void variableInput(){
  int inputRunTime = 2000;
}

void startCycle(){
  Serial.print("Press s to start\n");
  while(true){
  if(Serial.available() > 0){
    char command = Serial.read();
    if (command == 's'){
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
      digitalWrite(BRAKE, true);
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
}}}}

void brake(){

}

void setup() {
Serial.begin(115200); //baud rate
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

}

void loop() {
  Serial.println("");
    startUP();
    situatePipe();
    situateFitting_centerClamp();
    situateFitting_sledClamp();
    variableInput();
    startCycle();
    brake();

}
    

