/*
 * CanalTek: skid steer hydraulic control system
 * Hardware: ClearCore Controller + CCIO-8 expansion module
 * Author: n.rago
 * Date: 12/28/2025
 *
 * All-caps identifiers reference physical hardware only
 * Test mode: SW_PREV/SW_NEXT cycle through states, SW_ENTER confirms
 */

#include "ClearCore.h"

// PIN ASSIGNMENTS
// analog outputs
#define CL1 IO0 // motor speed (HSPEC1)
#define CL2 IO1 // master fluid valve (PV1)

// solenoid outputs - mainboard
#define SV2_S1 IO2 // SLED CLAMPS open
#define SV2_S2 IO3 // SLED CLAMPS close
#define SV3_S1 IO4 // SIDE CLAMPS open
#define SV3_S2 IO5 // SIDE CLAMPS close

// solenoid outputs - CCIO-8
#define SV4_S1 CLEARCORE_PIN_CCIOA0 // MIDDLE CLAMP open
#define SV4_S2 CLEARCORE_PIN_CCIOA1 // MIDDLE CLAMP close
#define SV5_S1 CLEARCORE_PIN_CCIOA2 // PRESSURE CYLINDERS open
#define SV5_S2 CLEARCORE_PIN_CCIOA3 // PRESSURE CYLINDERS close
#define BRAKE CLEARCORE_PIN_CCIOA7 // BRAKE MOTORS

// test mode switches - CCIO-8
#define Switch1 CLEARCORE_PIN_CCIOA4 // step backward through states
#define Switch2 CLEARCORE_PIN_CCIOA5 // step forward through states
#define EnterSwitch CLEARCORE_PIN_CCIOA6 // enter/activate current state

// inputs - mainboard
#define START_BTN DI6
#define SPEED_POT A9
#define TIME_POT A10

// timing (ms)
const unsigned long clampDly = 2000;
const unsigned long spinupDly = 2000;
const unsigned long brakeDly = 5000;
const unsigned long releaseDly = 1000;
const unsigned long debounce = 200;

// state timeout margin (ms)
// time-based fallback only - replace with sensor feedback / watchdog when hardware is available
// e.g. pressure transducers or position switches on each clamp would be more reliable
const unsigned long timeoutMargin = 2000;

// state machine
enum State {
  idle,
  openClamps,
  waitPos,
  clampCtr,
  clampSled,
  ready,
  engageClamps,
  spinup,
  engagePress,
  running,
  stop,
  brakeWait,
  releaseClamps,
  releasePress,
  done,
  fault
};

// test mode cycles through this ordered list
const State testStates[] = {
  idle, openClamps, waitPos, clampCtr, clampSled,
  ready, engageClamps, spinup, engagePress, running,
  stop, brakeWait, releaseClamps, releasePress, done
};

const int testStateCount = sizeof(testStates) / sizeof(testStates[0]);

State state = idle;
unsigned long timer = 0;
unsigned long stateEntry = 0;
unsigned long runT = 0;
int mtrSpd = 0;
int fluidCtl = 0;

bool testMode = false; // toggled by holding SW_ENTER at startup
int  testIdx = 0;     // index into testStates[]
bool swNextPrev = false; // debounce flag for Switch1/Switch2
bool swEnterPrev = false; // debounce flag for EnterSwitch

// prototypes
void initPins();
void initCCIO();
void setCcio(int pin, bool val);
void fireSol(int pin, bool val);
void setAnalog(int pin, int val);
void openAll();
void closeAll();
void allOff();
void readInputs();
void enterState(State next);
bool stateTimedOut(unsigned long limit);
void triggerFault(const char* msg);
void handleTestMode();
const char* stateName(State s);

void setup() {
  Serial.begin(9600);
  uint32_t t = millis();
  while (!Serial && millis() - t < 3000) {}

  initPins();
  initCCIO();

  // hold EnterSwitch at power-on to enter test mode
  if (digitalRead(EnterSwitch) == HIGH) {
    testMode = true;
    testIdx  = 0;
    Serial.println("test mode active");
    Serial.println("Switch1/Switch2: cycle state, EnterSwitch: activate");
  } else {
    Serial.println("normal mode");
  }

  allOff();
  Serial.println("system ready");
  enterState(idle);
}

void loop() {
  readInputs();

  if (testMode) {
    handleTestMode();
    return;
  }

  switch (state) {

    // idle
    case idle:
      setAnalog(CL2, fluidCtl); // step 1: open CL2 for fluid
      if (digitalRead(START_BTN) == HIGH) {
        delay(debounce);
        Serial.println("starting pos.");
        enterState(openClamps);
      }
      break;

    // open clamps
    case openClamps:
      openAll();
      Serial.println("clamps open - position pipe, press start when done");
      enterState(waitPos);
      break;

    // wait for operator to position pipe (steps 2-4)
    case waitPos:
      if (digitalRead(START_BTN) == HIGH) {
        delay(debounce);
        Serial.println("engaging clamps");
        enterState(clampCtr);
      }
      break;

    // clamp center - step 3: close SV4
    case clampCtr:
      fireSol(SV4_S2, HIGH);
      fireSol(SV4_S1, LOW);
      if (millis() - timer > 500) {
        Serial.println("sv4 closed");
        enterState(clampSled);
      }
      if (stateTimedOut(500 + timeoutMargin))
        triggerFault("clampCtr timeout - SV4 may be stuck");
      break;

    // clamp sled - step 4: close SV2
    case clampSled:
      fireSol(SV2_S2, HIGH);
      fireSol(SV2_S1, LOW);
      if (millis() - timer > 500) {
        Serial.println("sv2 closed");
        Serial.println("ready: press start to run");
        enterState(ready);
      }
      if (stateTimedOut(500 + timeoutMargin))
        triggerFault("clampSled timeout - SV2 may be stuck");
      break;

    // ready - step 5: operator sets speed and time, press start
    case ready:
      if (digitalRead(START_BTN) == HIGH) {
        delay(debounce);
        Serial.println("cycle start...");
        enterState(engageClamps);
      }
      break;

    // engage clamps - step 6: fire SV2:S1, SV3:S1, SV4:S1
    case engageClamps:
      fireSol(SV2_S1, HIGH);
      fireSol(SV3_S1, HIGH);
      fireSol(SV4_S1, HIGH);
      if (millis() - timer >= clampDly)
        enterState(spinup);
      if (stateTimedOut(clampDly + timeoutMargin))
        triggerFault("engageClamps timeout");
      break;

    // spinup motor - fire CL1 then CL2
    case spinup:
      setAnalog(CL1, mtrSpd);
      setAnalog(CL2, fluidCtl);
      Serial.print("cl1="); Serial.print(mtrSpd);
      Serial.print(" cl2="); Serial.println(fluidCtl);
      if (millis() - timer >= spinupDly)
        enterState(engagePress);
      if (stateTimedOut(spinupDly + timeoutMargin))
        triggerFault("spinup timeout - motor may not be responding");
      break;

    // engage pressure - fire SV5:S1
    case engagePress:
      fireSol(SV5_S1, HIGH);
      fireSol(SV5_S2, LOW);
      Serial.print("sv5:s1 engaged - running for ");
      Serial.print(runT / 1000); Serial.println("s");
      enterState(running);
      break;

    // running - wait for operator run time
    case running:
      if (millis() - timer >= runT) {
        Serial.println("run complete");
        enterState(stop);
      }
      if (stateTimedOut(runT + timeoutMargin))
        triggerFault("run timeout exceeded expected duration");
      break;

    // stop motor - step 7: remove signals, fire brakes
    case stop:
      setAnalog(CL1, 0);
      setAnalog(CL2, 0);
      fireSol(BRAKE, HIGH);
      Serial.println("motor stopped, brakes engaged");
      enterState(brakeWait);
      break;

    // brake wait - hold 5 seconds
    case brakeWait:
      if (millis() - timer >= brakeDly) {
        fireSol(BRAKE, LOW);
        Serial.println("releasing clamps...");
        enterState(releaseClamps);
      }
      if (stateTimedOut(brakeDly + timeoutMargin))
        triggerFault("brakeWait timeout");
      break;

    // release clamps - turn off s1, turn on s2
    case releaseClamps:
      fireSol(SV5_S1, LOW);
      fireSol(SV2_S1, LOW);
      fireSol(SV3_S1, LOW);
      fireSol(SV4_S1, LOW);
      fireSol(SV2_S2, HIGH);
      fireSol(SV3_S2, HIGH);
      fireSol(SV4_S2, HIGH);
      if (millis() - timer >= releaseDly)
        enterState(releasePress);
      if (stateTimedOut(releaseDly + timeoutMargin))
        triggerFault("releaseClamps timeout");
      break;

    // release pressure - fire SV5:S2 for 1 second
    case releasePress:
      fireSol(SV5_S2, HIGH);
      if (millis() - timer >= releaseDly) {
        fireSol(SV5_S2, LOW);
        enterState(done);
      }
      if (stateTimedOut(releaseDly + timeoutMargin))
        triggerFault("releasePress timeout");
      break;

    // cycle complete
    case done:
      Serial.println("cycle complete");
      allOff();
      enterState(idle);
      break;

    // fault: safe shutdown, requires power cycle or reset
    // add a reset input in the future to return to idle without power cycle
    case fault:
      break;
  }
}

// TEST MODE
// SW_PREV/SW_NEXT scroll through state list, EnterSwitch activates the selected state
// outputs are driven as they would be in normal mode - use for verifying each stage
void handleTestMode() {
  bool swNext  = digitalRead(Switch2)     == HIGH;
  bool swPrev  = digitalRead(Switch1)     == HIGH;
  bool swEnter = digitalRead(EnterSwitch) == HIGH;

  // step forward
  if (swNext && !swNextPrev) {
    testIdx = (testIdx + 1) % testStateCount;
    Serial.print("test selected: ");
    Serial.println(stateName(testStates[testIdx]));
    delay(debounce);
  }

  // step backward
  if (swPrev && !swNextPrev) {
    testIdx = (testIdx - 1 + testStateCount) % testStateCount;
    Serial.print("test selected: ");
    Serial.println(stateName(testStates[testIdx]));
    delay(debounce);
  }

  // activate selected state
  if (swEnter && !swEnterPrev) {
    allOff();
    enterState(testStates[testIdx]);
    Serial.print("test entering: ");
    Serial.println(stateName(state));
    delay(debounce);
  }

  swNextPrev  = swNext || swPrev;
  swEnterPrev = swEnter;

  // run one loop iteration of the active state
  // re-uses normal mode logic so behavior is identical
  switch (state) {
    case openClamps:    openAll(); enterState(idle); break;
    case engageClamps:
      fireSol(SV2_S1, HIGH); fireSol(SV3_S1, HIGH); fireSol(SV4_S1, HIGH);
      break;
    case engagePress:
      fireSol(SV5_S1, HIGH); fireSol(SV5_S2, LOW);
      break;
    case stop:
      setAnalog(CL1, 0); setAnalog(CL2, 0); fireSol(BRAKE, HIGH);
      break;
    case releaseClamps:
      fireSol(SV2_S2, HIGH); fireSol(SV3_S2, HIGH); fireSol(SV4_S2, HIGH);
      fireSol(SV2_S1, LOW);  fireSol(SV3_S1, LOW);  fireSol(SV4_S1, LOW);
      break;
    case releasePress:
      fireSol(SV5_S2, HIGH);
      break;
    default:
      break;
  }
}

// returns printable state name for serial debug
const char* stateName(State s) {
  switch (s) {
    case idle: return "idle";
    case openClamps: return "openClamps";
    case waitPos: return "waitPos";
    case clampCtr: return "clampCtr";
    case clampSled: return "clampSled";
    case ready: return "ready";
    case engageClamps: return "engageClamps";
    case spinup: return "spinup";
    case engagePress: return "engagePress";
    case running: return "running";
    case stop: return "stop";
    case brakeWait: return "brakeWait";
    case releaseClamps: return "releaseClamps";
    case releasePress: return "releasePress";
    case done: return "done";
    case fault: return "fault";
    default: return "unknown";
  }
}

// STATE MANAGEMENT
void enterState(State next) {
  state = next;
  timer = millis();
  stateEntry = millis();
}

// returns true if current state has exceeded its time limit
bool stateTimedOut(unsigned long limit) {
  return (millis() - stateEntry >= limit);
}

// kills all outputs and locks into fault state
// future: sensor-based safe shutdown in the correct release order to avoid hydraulic damage
void triggerFault(const char* msg) {
  Serial.print("FAULT: ");
  Serial.println(msg);
  allOff();
  state = fault;
}

// INITIALIZATION
void initPins() {
  // mainboard outputs
  pinMode(CL1, OUTPUT);
  pinMode(CL2, OUTPUT);
  pinMode(SV2_S1, OUTPUT);
  pinMode(SV2_S2, OUTPUT);
  pinMode(SV3_S1, OUTPUT);
  pinMode(SV3_S2, OUTPUT);
  // mainboard input
  pinMode(START_BTN, INPUT);
  // CCIO-8 outputs + inputs configured after initCCIO()
}

void initCCIO() {
  ConnectorCOM0.Mode(Connector::CCIO); // must set mode before opening
  ConnectorCOM0.PortOpen();

  // configure CCIO-8 pins via pinMode - matches working code pattern
  pinMode(SV4_S1, OUTPUT);
  pinMode(SV4_S2, OUTPUT);
  pinMode(SV5_S1, OUTPUT);
  pinMode(SV5_S2, OUTPUT);
  pinMode(BRAKE,  OUTPUT);
  pinMode(Switch1, INPUT);
  pinMode(Switch2, INPUT);
  pinMode(EnterSwitch, INPUT);

  if (CcioMgr.CcioCount() > 0) {
    Serial.println("ccio-8 ready");
  } else {
    Serial.println("warning: ccio-8 not detected");
  }
}

// OUTPUT CONTROL
void setCcio(int pin, bool val) {
  digitalWrite(pin, val ? HIGH : LOW);
}

void fireSol(int pin, bool val) {
  digitalWrite(pin, val ? HIGH : LOW);
}

void setAnalog(int pin, int val) {
  analogWrite(pin, constrain(val, 0, 4095));
}

void openAll() {
  fireSol(SV2_S1, HIGH); fireSol(SV3_S1, HIGH);
  fireSol(SV4_S1, HIGH); fireSol(SV5_S1, HIGH);
  fireSol(SV2_S2, LOW);  fireSol(SV3_S2, LOW);
  fireSol(SV4_S2, LOW);  fireSol(SV5_S2, LOW);
}

void closeAll() {
  fireSol(SV2_S2, HIGH); fireSol(SV3_S2, HIGH);
  fireSol(SV4_S2, HIGH); fireSol(SV5_S2, HIGH);
  fireSol(SV2_S1, LOW);  fireSol(SV3_S1, LOW);
  fireSol(SV4_S1, LOW);  fireSol(SV5_S1, LOW);
}

void allOff() {
  fireSol(SV2_S1, LOW); fireSol(SV2_S2, LOW);
  fireSol(SV3_S1, LOW); fireSol(SV3_S2, LOW);
  fireSol(SV4_S1, LOW); fireSol(SV4_S2, LOW);
  fireSol(SV5_S1, LOW); fireSol(SV5_S2, LOW);
  fireSol(BRAKE,  LOW);
  setAnalog(CL1, 0);
  setAnalog(CL2, 0);
}

// INPUT HANDLING
// pots assumed for speed/time input - update when actual input hardware is confirmed
void readInputs() {
  mtrSpd = analogRead(SPEED_POT);

  // run time pot: 1-60 seconds
  int raw = analogRead(TIME_POT);
  runT = map(raw, 0, 4095, 1000, 60000);

  // fluid control scales with active coil count
  int coils = 0;
  if (digitalRead(SV2_S1) || digitalRead(SV2_S2)) coils++;
  if (digitalRead(SV3_S1) || digitalRead(SV3_S2)) coils++;
  fluidCtl = constrain(2048 + (coils * 256), 0, 4095);
}