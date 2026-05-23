#include <HUSKYLENS.h>
#include <Wire.h>
#include <Metro.h>

const int speedForward = 80;
const int speedTurn    = 210;
const int speedSlow    = 20;
const int speedBack    = 170;

const int speedPin_M1     = 5;
const int speedPin_M2     = 6;
const int directionPin_M1 = 4;
const int directionPin_M2 = 7;

HUSKYLENS huskylens;

const int CENTER    = 160;
const int DEAD_ZONE = 70; // ← réduit pour anticiper les virages

Metro huskylensTimer = Metro(20); // ← plus rapide
Metro debugTimer     = Metro(500);
Metro scanTimer      = Metro(1);

enum State {
  FOLLOW,
  ADJUST_LEFT,
  ADJUST_RIGHT,
  SCAN_LEFT,
  SCAN_CENTER,
  SCAN_RIGHT
};

State currentState = FOLLOW;

int  lineX        = CENTER;
int  targetX      = CENTER;
bool lineFound    = false;

int lostCounter        = 0;
const int LOST_CONFIRM = 4; // frames perdues avant de scanner

const int SCAN_TURN_DURATION   = 400;
const int SCAN_CENTER_DURATION = 200;

bool scanning = false;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  while (!huskylens.begin(Wire)) {
    Serial.println(F("HuskyLens non détecté..."));
    delay(500);
  }

  huskylens.writeAlgorithm(ALGORITHM_LINE_TRACKING);
  Serial.println(F("HuskyLens OK"));
  delay(500);
  carAdvance(speedForward, speedForward);
}

void loop() {

  if (huskylensTimer.check()) {
    readHuskyLens();

    // Ligne retrouvée pendant un scan → fonce immédiatement
    if (lineFound && scanning) {
      scanning = false;
      lostCounter = 0;
      enterState(FOLLOW);
      return;
    }

    // Pas en scan → logique normale
    if (!scanning) {
      State newState = computeState();
      if (newState != currentState) {
        enterState(newState);
      }
    }
  }

  // Timer scan actif uniquement pendant le scan
  if (scanning && scanTimer.check()) {
    switch (currentState) {
      case SCAN_LEFT:   enterState(SCAN_CENTER); break;
      case SCAN_CENTER: enterState(SCAN_RIGHT);  break;
      case SCAN_RIGHT:  enterState(SCAN_LEFT);   break;
      default: break;
    }
  }

  if (debugTimer.check()) {
    Serial.print(F("State: "));
    Serial.print(stateLabel(currentState));
    Serial.print(F(" | lineX: "));
    Serial.print(lineX);
    Serial.print(F(" | lost: "));
    Serial.println(lostCounter);
  }
}

// ════════════════════════════════════════════════════════════
void readHuskyLens() {
  if (!huskylens.request()) {
    lostCounter++;
    if (lostCounter >= LOST_CONFIRM) lineFound = false;
    return;
  }

  bool found = false;
  while (huskylens.available()) {
    HUSKYLENSResult result = huskylens.read();
    if (result.command == COMMAND_RETURN_ARROW) {
      lineX   = result.xOrigin;
      targetX = result.xTarget;
      found   = true;
    }
  }

  if (found) {
    lineFound   = true;
    lostCounter = 0;
  } else {
    lostCounter++;
    if (lostCounter >= LOST_CONFIRM) lineFound = false;
  }
}

// ════════════════════════════════════════════════════════════
State computeState() {
  if (!lineFound) {
    scanning = true;
    return SCAN_LEFT;
  }

  int directionError = targetX - CENTER;
  if (abs(directionError) > DEAD_ZONE) {
    return (directionError < 0) ? ADJUST_LEFT : ADJUST_RIGHT;
  }

  int error = lineX - CENTER;
  if (error < -DEAD_ZONE) return ADJUST_LEFT;
  if (error >  DEAD_ZONE) return ADJUST_RIGHT;

  return FOLLOW;
}

// ════════════════════════════════════════════════════════════
void enterState(State next) {
  currentState = next;

  switch (next) {

    case FOLLOW:
      // Fonce direct sans hésitation
      carAdvance(speedForward, speedForward);
      break;

    case ADJUST_LEFT:
      carAdjust(speedSlow, speedTurn);
      break;

    case ADJUST_RIGHT:
      carAdjust(speedTurn, speedSlow);
      break;

    case SCAN_LEFT:
      carBack(speedBack, speedSlow);
      scanTimer.interval(SCAN_TURN_DURATION);
      scanTimer.reset();
      break;

    case SCAN_CENTER:
      carBack(speedBack, speedBack);
      scanTimer.interval(SCAN_CENTER_DURATION);
      scanTimer.reset();
      break;

    case SCAN_RIGHT:
      carBack(speedSlow, speedBack);
      scanTimer.interval(SCAN_TURN_DURATION);
      scanTimer.reset();
      break;
  }
}

// ════════════════════════════════════════════════════════════
const char* stateLabel(State s) {
  switch (s) {
    case FOLLOW:       return "FOLLOW";
    case ADJUST_LEFT:  return "ADJUST_LEFT";
    case ADJUST_RIGHT: return "ADJUST_RIGHT";
    case SCAN_LEFT:    return "SCAN_LEFT";
    case SCAN_CENTER:  return "SCAN_CENTER";
    case SCAN_RIGHT:   return "SCAN_RIGHT";
    default:           return "?";
  }
}

// ════════════════════════════════════════════════════════════
void carStop() {
  analogWrite(speedPin_M2, 0);  digitalWrite(directionPin_M1, LOW);
  analogWrite(speedPin_M1, 0);  digitalWrite(directionPin_M2, LOW);
}

void carAdvance(int leftSpeed, int rightSpeed) {
  analogWrite(speedPin_M2, leftSpeed);  digitalWrite(directionPin_M1, HIGH);
  analogWrite(speedPin_M1, rightSpeed); digitalWrite(directionPin_M2, LOW);
}

void carAdjust(int leftSpeed, int rightSpeed) {
  analogWrite(speedPin_M2, leftSpeed);  digitalWrite(directionPin_M1, HIGH);
  analogWrite(speedPin_M1, rightSpeed); digitalWrite(directionPin_M2, LOW);
}

void carBack(int leftSpeed, int rightSpeed) {
  analogWrite(speedPin_M2, leftSpeed);  digitalWrite(directionPin_M1, LOW);
  analogWrite(speedPin_M1, rightSpeed); digitalWrite(directionPin_M2, HIGH);
}
