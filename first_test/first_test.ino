const int speedPin_M1     = 5;
const int speedPin_M2     = 6;
const int directionPin_M1 = 4;
const int directionPin_M2 = 7;

void setup(){                                 // Serial initialization

  Serial.begin(9600);                         // Sets the baud rate to 9600
carTurnLeft(200,200);
delay(500);
carStop();
  

}

void loop(){

}

void carStop(){                 //  Motor Stop
  digitalWrite(speedPin_M2,0);
  digitalWrite(directionPin_M1,LOW);
  digitalWrite(speedPin_M1,0);
  digitalWrite(directionPin_M2,LOW);
}

void carBack(int leftSpeed,int rightSpeed){         //Move backward
  analogWrite (speedPin_M2,leftSpeed);
  digitalWrite(directionPin_M1,LOW);
  analogWrite (speedPin_M1,rightSpeed);
  digitalWrite(directionPin_M2,HIGH);
}

void carAdvance(int leftSpeed,int rightSpeed){       //Move forward
  analogWrite (speedPin_M2,leftSpeed);
  digitalWrite(directionPin_M1,HIGH);
  analogWrite (speedPin_M1,rightSpeed);
  digitalWrite(directionPin_M2,LOW);
}

void carTurnLeft(int leftSpeed,int rightSpeed){ 
  analogWrite (speedPin_M2,leftSpeed);              //PWM Speed Control
  digitalWrite(directionPin_M1,HIGH);
  analogWrite (speedPin_M1,rightSpeed);
  digitalWrite(directionPin_M2,HIGH);     //Turn Left
  
}
void carTurnRight(int leftSpeed,int rightSpeed){      //Turn Right
  analogWrite (speedPin_M2,leftSpeed);
  digitalWrite(directionPin_M1,LOW);
  analogWrite (speedPin_M1,rightSpeed);
  digitalWrite(directionPin_M2,LOW);
}

/*void setup(void) {
  int i;
  for (i = 4; i <= 7; i++)
    pinMode(i, OUTPUT);
  Serial.begin(9600);      //Set Baud Rate
  Serial.println("hello. w = forward, d = turn right, a = turn left, s = backward, x = stop, z = hello world"); //Display instructions in the serial monitor
  digitalWrite(E1, LOW);
  digitalWrite(E2, LOW);
}

void loop(void) {
  if (Serial.available()) {
    char val = Serial.read();
    if (val != -1)
    {
      switch (val)
      {
        case 'a'://Move Forward
          Serial.println("turn  left");
          advance (255, 255);  //move forward at max speed
          delay (1000);
          stop();
          break;
        case 'd'://Move Backward
          Serial.println(" turn right");
          back_off (255, 255);  //move backwards at max speed
          delay (1000);
          stop();
          break;
        case 's'://Turn Left
          Serial.println("going backward");
          turn_L (255, 255);
          delay (1000);
          stop();
          break;
        case 'w'://Turn Right
          Serial.println("going forward");
          turn_R (255, 255);
          delay (1000);
          stop();
          break;
        case 'z':
          Serial.println("hello world!");
          break;
        case 'x':
          Serial.println("stopping");
          stop();
          break;
      }
    }
    else stop();
  }
}*/
