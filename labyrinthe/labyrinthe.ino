#include <Servo.h>
#include <Metro.h>

// ─── CONFIGURATION DES VITESSES ─────────────────────────────
int speedForward = 150; // Légèrement baissé pour donner le temps de manoeuvrer
int speedTurn    = 160;
int speedBack    = 120; // Augmenté pour un recul franc face au mur

// Le temps entre deux mesures dépend de la vitesse d'avance
int sensorDelay = 50; 

// ─── TIMERS (METRO) ─────────────────────────────────────────
Metro measureDistance = Metro(sensorDelay);
Metro sweepServo      = Metro(sensorDelay);  // Balayage un peu plus rapide (25ms)
Metro actionTimer     = Metro(400); // Chronomètre pour les phases d'évitement

// ─── PINS MOTEURS (CHEROKEY 4WD) ────────────────────────────
int speedPin_M1     = 5;
int speedPin_M2     = 6;
int directionPin_M1 = 4;
int directionPin_M2 = 7;

unsigned long actualDistance = 0;

// ─── SERVO ET BALAYAGE ──────────────────────────────────────
Servo myservo;
int pos = 90;
int sweepFlag = 1;

// ─── CAPTEUR ULTRASON (URM37) ───────────────────────────────
int URPWM = 3;
int URTRIG = 10;
uint8_t EnPwmCmd[4] = {0x44, 0x02, 0xbb, 0x01};

// ─── MACHINE À ÉTATS POUR LE LABYRINTHE ──────────────────────
enum RobotState {
  DRIVE_FORWARD,  // Phase 1 : Avance et cherche la sortie
  EVADE_BACK,     // Phase 2 : Obstacle détecté -> Recul d'urgence
  EVADE_TURN      // Phase 3 : Pivotement pour s'orienter ailleurs
};

RobotState currentState = DRIVE_FORWARD;
int turnDirection = 0; // 1 = Droite, -1 = Gauche

void setup() {
  myservo.attach(9);
  Serial.begin(9600);
  SensorSetup();
  
  carStop();
  delay(1000); // Pause de sécurité au démarrage
}

void loop() {

  // 1. Balayage permanent et fluide du servo-moteur
  if (sweepServo.check()) {
    servoSweep();
  }

  // 2. Mesure de distance (Uniquement en marche avant pour ne pas fausser l'évitement)
  if (currentState == DRIVE_FORWARD && measureDistance.check()) {
    sensorDelay = 200 - speedForward;
    if (sensorDelay < 30) sensorDelay = 30; // Sécurité processeur
    measureDistance.interval(sensorDelay);

    actualDistance = MeasureDistance();
    Serial.print(F("Distance: "));
    Serial.println(actualDistance);
  }

  // 3. AUTOMATE DE NAVIGATION (Séquences de mouvements strictes)
  switch (currentState) {
    
    case DRIVE_FORWARD:
      carAdvance(speedForward, speedForward);
      
      // Si un mur du labyrinthe est détecté à 30 cm ou moins
      if (actualDistance <= 30) {
        carStop(); // Arrêt immédiat pour couper l'inertie
        
        // Décision du sens de rotation selon la position du servo
        if (pos >= 90) {
          turnDirection = 1;  // Mur détecté à gauche -> On tournera à DROITE
        } else {
          turnDirection = -1; // Mur détecté à droite -> On tournera à GAUCHE
        }
        
        // Déclenchement de la phase de RECUL
        carBack(speedBack, speedBack);
        actionTimer.interval(400); // Recule pendant 400 ms
        actionTimer.reset();
        currentState = EVADE_BACK;
      }
      break;

    case EVADE_BACK:
      // On attend la fin des 400 ms de recul
      if (actionTimer.check()) {
        carStop(); // Arrêt transitoire pour préserver le pont en H
        
        // Déclenchement de la phase de PIVOT
        if (turnDirection == 1) {
          carTurnRight(speedTurn, speedTurn);
        } else {
          carTurnLeft(speedTurn, speedTurn);
        }
        
        actionTimer.interval(550); // Pivote pendant 550 ms (Ajustable pour faire un angle droit)
        actionTimer.reset();
        currentState = EVADE_TURN;
      }
      break;

    case EVADE_TURN:
      // On attend la fin des 550 ms de rotation
      if (actionTimer.check()) {
        carStop();
        actualDistance = 100; // Reset virtuel pour effacer le mur de la mémoire
        currentState = DRIVE_FORWARD; // Repart en marche avant
      }
      break;
  }
}

// ════════════════════════════════════════════════════════════
// CAPTEUR ULTRASON (URM37)
// ════════════════════════════════════════════════════════════
void SensorSetup() {
  pinMode(URTRIG, OUTPUT);
  digitalWrite(URTRIG, HIGH);
  pinMode(URPWM, INPUT);

  for (int i = 0; i < 4; i++) {
    Serial.write(EnPwmCmd[i]);
  }
}
int MeasureDistance() { // a low pull on pin COMP/TRIG  triggering a sensor reading
  digitalWrite(URTRIG, LOW);
  digitalWrite(URTRIG, HIGH);               // reading Pin PWM will output pulses
  unsigned long distance = pulseIn(URPWM, LOW);
  if (distance == 1000) {          // the reading is invalid.
    Serial.print("Invalid");
  } else {
    distance = distance / 50;       // every 50us low level stands for 1cm
  }
  return distance;
}

// ════════════════════════════════════════════════════════════
// SERVO (Balayage angulaire restreint pour maximiser la réactivité)
// ════════════════════════════════════════════════════════════
void servoSweep() {
  if (sweepFlag) {
    pos += 6; // Incrément légèrement augmenté pour accélérer le balayage
    if (pos >= 125) sweepFlag = 0;
  } 
  else {
    pos -= 6;
    if (pos <= 55) sweepFlag = 1;
  }
  myservo.write(pos);
}

// ════════════════════════════════════════════════════════════
// FONCTIONS MOTEURS CHEROKEY 4WD (REPRISES DE TON CODE ORIGINAL)
// ════════════════════════════════════════════════════════════
void carStop(){                 
  digitalWrite(speedPin_M2, 0);
  digitalWrite(directionPin_M1, LOW);
  digitalWrite(speedPin_M1, 0);
  digitalWrite(directionPin_M2, LOW);
}

void carBack(int leftSpeed, int rightSpeed){         
  analogWrite(speedPin_M2, leftSpeed);
  digitalWrite(directionPin_M1, LOW);
  analogWrite(speedPin_M1, rightSpeed);
  digitalWrite(directionPin_M2, HIGH);
}

void carAdvance(int leftSpeed, int rightSpeed){       
  analogWrite(speedPin_M2, leftSpeed);
  digitalWrite(directionPin_M1, HIGH);
  analogWrite(speedPin_M1, rightSpeed);
  digitalWrite(directionPin_M2, LOW);
}

void carTurnLeft(int leftSpeed, int rightSpeed){ 
  analogWrite(speedPin_M2, leftSpeed);              
  digitalWrite(directionPin_M1, HIGH);
  analogWrite(speedPin_M1, rightSpeed);
  digitalWrite(directionPin_M2, HIGH);     
}

void carTurnRight(int leftSpeed, int rightSpeed){      
  analogWrite(speedPin_M2, leftSpeed);
  digitalWrite(directionPin_M1, LOW);
  analogWrite(speedPin_M1, rightSpeed);
  digitalWrite(directionPin_M2, LOW);
}
