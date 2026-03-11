
#define CcioPort ConnectorCOM0
#include "ClearCore.h"

// PIN ASSIGNMENTS
// analog/pwm outputs
#define CL1  IO0 // motor speed (HSPEC1)
#define CL2  IO1 // master fluid valve (PV1)

// solenoid outputs on controller
#define SV2_S1  IO2 // SLED CLAMPS open
#define SV2_S2  IO3 // SLED CLAMPS close
#define SV3_S1  IO4 // SIDE CLAMPS open
#define SV3_S2  IO5 // SIDE CLAMPS close

// ccio-8 expansion pins
#define SV4_S1  CLEARCORE_PIN_CCIOA0 // MIDDLE CLAMP open
#define SV4_S2  CLEARCORE_PIN_CCIOA1 // MIDDLE CLAMP close
#define SV5_S1  CLEARCORE_PIN_CCIOA2 // PRESSURE CYLINDERS open
#define SV5_S2  CLEARCORE_PIN_CCIOA3 // PRESSURE CYLINDERS close
#define BRAKE   CLEARCORE_PIN_CCIOA4 // BRAKE MOTORS

// inputs
#define START_BTN   DI6
#define SPEED_POT   A9
#define TIME_POT    A10

// Declares a variable used to write new states to the output. We will toggle
// this true/false.


void setup() {
Serial.begin(115200);

CcioPort.Mode(Connector::CCIO);
ConnectorCOM0.PortOpen();
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

   
    
   //CcioPort.Mode(Connector::CCIO);
   // CcioPort.PortOpen();
    

    // The connectors are all set up; start the loop with turning them all on.
     while (!Serial);
    Serial.println("1-power on 2-power off");
}

void loop() {
    int start;
   if(Serial.available()){
    start = Serial.parseInt();
    if (start == 1){
        Serial.println("Toggle ON");
    }else if(start == 2) {
        Serial.println("Toggle OFF");
    }

   }

if(start == 1){
    // Toggle the digital output state.
        digitalWrite(IO0, true);
        digitalWrite(IO1, true);
        digitalWrite(IO2, true);
        digitalWrite(IO3, true);
        digitalWrite(IO4, true);
        digitalWrite(IO5, true);
        digitalWrite(CLEARCORE_PIN_CCIOA0, true);
        digitalWrite(CLEARCORE_PIN_CCIOA1, true);
        digitalWrite(CLEARCORE_PIN_CCIOA2, true);
        digitalWrite(CLEARCORE_PIN_CCIOA3, true);
        loop();
        
    }
    else if (start == 2){
        digitalWrite(IO0, false);
        digitalWrite(IO1, false);
        digitalWrite(IO2, false);
        digitalWrite(IO3, false);
        digitalWrite(IO4, false);
        digitalWrite(IO5, false);
        digitalWrite(CLEARCORE_PIN_CCIOA0, false);
        digitalWrite(CLEARCORE_PIN_CCIOA1, false);
        digitalWrite(CLEARCORE_PIN_CCIOA2, false);
        digitalWrite(CLEARCORE_PIN_CCIOA3, false);
        loop();
    }
}
    


