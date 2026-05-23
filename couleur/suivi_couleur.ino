#include <HUSKYLENS.h>
#include <Wire.h>
#include <Metro.h>

// ─── VITESSES POUR LE SUIVI ─────────────────────────────────
const int speedForward  = 90;   
const int speedTurn     = 180;  
const int speedSlow     = 30;   

// ─── PINS MOTEURS (CHEROKEY 4WD) ────────────────────────────
const int speedPin_M1     = 5;
const int speedPin_M2     = 6;
const int directionPin_M1 = 4;
const int directionPin_M2 = 7;

// ─── HUSKYLENS ──────────────────────────────────────────────
HUSKYLENS huskylens;

const int CENTER    = 160;  // Milieu de l'écran (320 / 2)
const int DEAD_ZONE = 40;   // Zone de confort pour aller tout droit

// ─── TIMERS ─────────────────────────────────────────────────
Metro huskylensTimer = Metro(30);   // Rafraîchissement ultra-rapide (30ms)
Metro debugTimer     = Metro(500);  

enum State {
  FOLLOW,       
  TRACK_LEFT,   
  TRACK_RIGHT,  
  LOST          
};

State currentState = LOST;

int  colorX     = CENTER; 
bool colorFound = false;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Attente de la caméra
  while (!huskylens.begin(Wire)) {
    Serial.println(F("HuskyLens non détecté..."));
    delay(500);
  }

  // Activation stricte du mode Couleur
  huskylens.writeAlgorithm(ALGORITHM_COLOR_RECOGNITION);
  Serial.println(F("HuskyLens OK - Mode Color Recognition"));
  
  // Initialisation des moteurs à l'arrêt complet
  carStop();
  delay(500);
}

void loop() {
  if (huskylensTimer.check()) {
    readColorPosition();

    State newState = computeState();

    if (newState != currentState) {
      currentState = newState;
      applyState(currentState);
    }
  }

  if (debugTimer.check()) {
    Serial.print(F("State: "));
    Serial.print(stateLabel(currentState));
    Serial.print(F(" | ColorX: "));
    Serial.println(colorFound ? String(colorX) : "RETIRE / ABSENT");
  }
}

// ════════════════════════════════════════════════════════════
// LECTURE DE LA CAMERA (SANS REBOND POUR ARRET LOGIQUE)
// ════════════════════════════════════════════════════════════
void readColorPosition() {
  if (!huskylens.request()) {
    colorFound = false;
    return;
  }

  if (huskylens.available()) {
    HUSKYLENSResult result = huskylens.read();

    // On cherche le bloc de couleur noir appris
    if (result.command == COMMAND_RETURN_BLOCK) {
      colorX = result.xCenter; 
      colorFound = true; // L'objet est présent
      return; 
    }
  }
  
  // Si rien n'est vu à ce cycle précis, l'objet est considéré comme absent
  colorFound = false; 
}

// ════════════════════════════════════════════════════════════
// LOGIQUE DE DÉCISION
// ════════════════════════════════════════════════════════════
State computeState() {
  if (!colorFound) return LOST; // Plus de couleur = ARRÊT IMMÉDIAT

  int error = colorX - CENTER;

  if (error < -DEAD_ZONE) {
    return TRACK_LEFT;
  }
  else if (error > DEAD_ZONE) {
    return TRACK_RIGHT;
  }
  
  return FOLLOW;
}

// ════════════════════════════════════════════════════════════
// CONTRÔLE DES ÉTATS
// ════════════════════════════════════════════════════════════
void applyState(State s) {
  switch (s) {
    case FOLLOW:
      carAdvance(speedForward, speedForward);
      break;
      
    case TRACK_LEFT:
      carAdjust(speedSlow, speedTurn); 
      break;
      
    case TRACK_RIGHT:
      carAdjust(speedTurn, speedSlow);
      break;
      
    case LOST:
      carStop(); // Utilisation de ta fonction Stop d'origine
      break;
  }
}

// ════════════════════════════════════════════════════════════
const char* stateLabel(State s) {
  switch (s) {
    case FOLLOW:      return "FOLLOW";
    case TRACK_LEFT:  return "TRACK_LEFT";
    case TRACK_RIGHT: return "TRACK_RIGHT";
    case LOST:        return "LOST_STOP";
    default:          return "?";
  }
}

// ════════════════════════════════════════════════════════════
// LES FONCTIONS MOTEURS SÉCURISÉES
// ════════════════════════════════════════════════════════════

void carStop(){  
  digitalWrite(speedPin_M2, 0);
  digitalWrite(directionPin_M1, LOW);
  digitalWrite(speedPin_M1, 0);
  digitalWrite(directionPin_M2, LOW);
}

void carAdvance(int leftSpeed, int rightSpeed) {
  analogWrite(speedPin_M2, leftSpeed);   digitalWrite(directionPin_M1, HIGH);
  analogWrite(speedPin_M1, rightSpeed);  digitalWrite(directionPin_M2, LOW);
}

void carAdjust(int leftSpeed, int rightSpeed) {
  analogWrite(speedPin_M2, leftSpeed);   digitalWrite(directionPin_M1, HIGH);
  analogWrite(speedPin_M1, rightSpeed);  digitalWrite(directionPin_M2, LOW);
}
