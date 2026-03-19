#This is the code that you control with the switches and potentiometers
#include "ClearCore.h"
#define CcioPort ConnectorCOM0
// analog/pwm outputs
#define CL1  IO0 // motor speed (HSPEC1)
#define CL2  IO1 // master fluid valve (PV1)
//solenoid outputs on controller
#define SV2_S1  IO2 // SLED CLAMPS ope#include "ClearCore.h"
#define CcioPort ConnectorCOM0
// analog/pwm outputs#include "ClearCore.h"
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
#define Switch1 DI8
#define Switch2 DI7
#define EnterSwitch DI6
#define ResetSwitch CLEARCORE_PIN_CCIOA4
#define SPEED_POT   A9
#define TIME_POT    A10
//constant
#define SPEED_POT_RESOLUTION 12//resolution of analog in, 8, 10, 12
#define TIME_POT_RESOLUTION 12
#define baudRate 9600


void startUP(){ //1. Startup, Machine turns on. set CL2 to some value. CL2 Value may need to continually change
  Serial.println("Startup Sequence Initiated"); 
  //digitalWrite(CL2, true); // need to set CL2 to some value
  Serial.println("Startup Sequence Complete");
}

void situatePipe(){ //2.Situate the pipe into place on both left and right side. User input toggle SV3:S1:S2 to situate pipe
  Serial.println("Situate The Pipe:");
  Serial.print("Press 1 to toggle SV3:S1 \nPress 2 to toggle SV3:S2\nPress Enter to finish\nPress Return to reset\n");
  while(true){
    digitalWrite(SV3_S1, false);
    digitalWrite(SV3_S2, false);
    while(digitalRead(Switch1)){
      digitalWrite(SV3_S1, true);
    }
    while(digitalRead(Switch2)){
      digitalWrite(SV3_S2, true);
    }
    digitalWrite(SV3_S1, false);
    digitalWrite(SV3_S2, false);
    if(digitalRead(ResetSwitch)){
      Serial.println("Restarting Startup Sequence");
      delay(1000);
      startUP();
    }
    if(digitalRead(EnterSwitch)){
      Serial.println("Finished");
      Serial.println(".........................................................");
      delay(3000);
      break;
    }
  } 
}
  

void situateFitting_centerClamp(){//3.Situate the fitting into the center clamp. User input toggle SV4:S1:S2
  Serial.println("Situate the Center Fitting:");
  Serial.print("Press 1 to toggle SV4:S1 \nPress 2 to toggle SV4:S2\nPress f to finish\nPress r to reset\n");
  while(true){
    digitalWrite(SV4_S1, false);
    digitalWrite(SV4_S2, false);
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
      Serial.println("Finished");
      Serial.println(".........................................................");
      delay(3000);
      break;
    }
    if(digitalRead(ResetSwitch)){
      Serial.println("Reseting...");
      Serial.println(".........................................................");
      delay(1000);
      situatePipe();
    }
  }
}
 
void situateFitting_sledClamp(){//4. situate fitting into the sled clamps. This requires a manual toggle of SV2:S1:S2
  Serial.println("Situate the Sled Clamp Fitting");
  Serial.print("Press 1 to toggle SV2:S1 \nPress 2 to toggle SV2:S2\nPress f to finish\nPress r to reset\n");
  while(true){
    digitalWrite(SV2_S1, false);
    digitalWrite(SV2_S2, false);
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
      Serial.println("Finished");
      Serial.println(".........................................................");
      delay(3000);
      break;
    }
    if(digitalRead(ResetSwitch)){
      Serial.println("Reseting...");
      Serial.println(".........................................................");
      delay(1000);
      situateFitting_centerClamp();
    }
  }
}
  

float variableInput(){//5.variable input, the operator will input the desired motor speed and run time which will control cl1 and cl2
  Serial.print("Please enter the input runtime: ");
  int inputRunTime;
  while(true){
    inputRunTime = analogRead(TIME_POT);
    Serial.print("current time is: " );
    Serial.print(inputRunTime * 0.01);
    Serial.println("s.");
    if(digitalRead(EnterSwitch)){
      break;
    }
  }
  return inputRunTime * 0.01;
  delay(1000);
  Serial.println("");
}

void startCycle(float variableRunTime){//6.Operator initiated spinning sequence
/*
This should fire SV2:S1, SV3:S1 and SV4:S1. Wait two seconds, fire precalculated singals into CL1 and CL2 (spinning), wait two seconds. Fire SV5:S1 and start timer for inputted run time
*/

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
      delay(variableRunTime * 1000); //VARIALBE RUNTIME HERE
    
      break;
}
}}

void brake(){//7.Brake sequence
/*
after run time, remove signals from above. wait 5 seconds, fire brake moters, turn off SV5:S1, SV2:S1, SV3:S1, SV4:S1
turn on SV2:S2, SV3:S2, SV4:S2 wait one second, Fire SV5:S2
*/
  Serial.println("Initiating Braking");
  digitalWrite(CL1, false);
  digitalWrite(CL2, false);//STOP CL1, CL2
  digitalWrite(BRAKE, true); //fire brake motors
  delay(5000);
  digitalWrite(SV5_S1, false);
  digitalWrite(SV2_S1, false);
  digitalWrite(SV3_S1, false);
  digitalWrite(SV4_S1, false);
  digitalWrite(SV2_S2, true);
  digitalWrite(SV3_S2, true);
  digitalWrite(SV4_S2, true);
  delay(1000);
  digitalWrite(SV5_S2, true);
  delay(1000);
  digitalWrite(SV5_S2, false);
  Serial.println("Cycle Complete");
  digitalWrite(SV2_S2, false);
  digitalWrite(SV3_S2, false);
  digitalWrite(SV4_S2, false);
  delay(3000);
  Serial.println("Braking Complete");
  delay(1000);
}

void shutOFF(){
  digitalWrite(SV2_S1, false);
  digitalWrite(SV2_S2, false);
  digitalWrite(SV3_S1, false);
  digitalWrite(SV3_S2, false);
  digitalWrite(SV4_S1, false);
  digitalWrite(SV4_S2, false);
  digitalWrite(SV5_S1, false);
  digitalWrite(SV5_S2, false);
  digitalWrite(CL1, false);
  digitalWrite(CL2, false);
}

void setup() {
Serial.begin(baudRate); //baud rate
CcioPort.Mode(Connector::CCIO); //Connect Expansion Board
CcioPort.PortOpen();
uint32_t timeout = 5000;
uint32_t startTime = millis();
while(!Serial && millis() - startTime < timeout){
  continue;
}
    
    pinMode(CL1, OUTPUT);
    pinMode(CL2, OUTPUT);

    pinMode(SV2_S1, OUTPUT);
    pinMode(SV2_S2, OUTPUT);
    pinMode(SV3_S1, OUTPUT);
    pinMode(SV3_S2, OUTPUT);
    pinMode(SV4_S1, OUTPUT);
    pinMode(SV4_S2, OUTPUT);
    pinMode(SV5_S1, OUTPUT);
    pinMode(SV5_S2, OUTPUT);

    pinMode(Switch1, INPUT);
    pinMode(Switch2, INPUT);

    pinMode(EnterSwitch, INPUT);
    pinMode(ResetSwitch, INPUT);
    
    analogReadResolution(TIME_POT_RESOLUTION);


}

void loop() {
  Serial.println("");

    startUP();
    situatePipe();
    Serial.println(".........................................................");
    situateFitting_centerClamp();
    Serial.println(".........................................................");
    situateFitting_sledClamp();
    Serial.println(".........................................................");
    int variableRunTime = variableInput();
    Serial.println(".........................................................");
    startCycle(variableRunTime);
    Serial.println(".........................................................");
    brake();
    shutOFF();
    Serial.println(".........................................................");
    loop();
    

}
    


