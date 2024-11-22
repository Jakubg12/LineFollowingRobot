#include <Servo.h>
const int GREEN = 7;
const int YELLOW = 12;
const int RED = 13;
const int leftButton = 4;
const int rightButton = 2;
const int irBlaster = 3;
int lServoStop;
int rServoStop;
Servo leftServo;
Servo rightServo;
int servoStops[] = {60, 60}; //default value of servoStops array is 60 to decrease calibration time (stop default somewhere between 60 and 90 on both servos)
int midSensor;
int rightSensor;
int leftSensor;
int irCount;


#define RLSE
void setup() {
  Serial.begin(9600);

  rightServo.attach(5);
  leftServo.attach(6);
  rightServo.write(0);
  leftServo.write(0);

  pinMode(GREEN, OUTPUT);      // setup green LED pin as output
  pinMode(YELLOW, OUTPUT);     // setup yellow LED pin as output
  pinMode(RED, OUTPUT);        // setup red LED pin as output
  pinMode(leftButton, INPUT);  //setup left button pin as input
  pinMode(rightButton, INPUT); //setup right button pin as input

  pinMode(A0, INPUT);         //setup mid sensor pin as input
  pinMode(A1, INPUT);         //setup right sensor pin as input
  pinMode(A2, INPUT);         //setup left sensor pin as input
  pinMode(irBlaster, OUTPUT); //setup irBlaster pin as output
  pinMode(2, INPUT);          //setup rightButton and ir Reciever pin as input

#ifdef RLSE
  caliBtnPress();
#endif
#ifdef IRTEST
  irRead();
#endif

}

void setLEDs (int green_state, int yellow_state, int red_state) {
  digitalWrite(YELLOW, yellow_state);
  digitalWrite(GREEN, green_state);
  digitalWrite(RED, red_state);

  return;
}

void Calibration(int btn) {     //Allows buttons to be used to calibrate servo 'stop speed', left button changes left servo speed, right button changes right servo speed.
                                //Writes value to stop array at each execution
  switch (btn) {
    case 1:

      if (lServoStop < 181) {
        delay(200);
        servoStops[0]++;
        leftServo.write(servoStops[0]);
        Serial.println(servoStops[0]);
        delay(200);
      }

      break;

    case 2:
      if (rServoStop < 181) {
        delay(200);
        servoStops[1]++;
        rightServo.write(servoStops[1]);
        Serial.println(servoStops[1]);
        delay(200);
      }
      break;
  }

  return;
}

void caliBtnPress() {                     //Reads button presses to determine which servo speed to change, lights up leds to show which button is being pressed (green and red for left, yellow for right)
  while (true) {                          //Invokes 'Calibration()' function to assign change stop array values
    if (digitalRead(leftButton) == LOW) { //If both buttons are pressed, breaks and goes into main 'loop()' function
      setLEDs(1, 0, 1);
      Calibration(1);
    }
    else {
      setLEDs(0, 0, 0);
    }

    if (digitalRead(rightButton) == LOW) {
      setLEDs(0, 1, 0);
      Calibration(2);
    }
    else
    {
      setLEDs(0, 0, 0);
    }
    if (digitalRead(rightButton) == LOW && digitalRead(leftButton) == LOW) {
      break;
    }

  }
}

void motorSpeedsLeft() {                //Turns left servo forward (robot turns left, forward)
  delay(20);
  leftServo.write(servoStops[0] + 50);
}

void motorSpeedsRight() {               //Turns right servo backwards (robot turns right, forward)
  delay(20);
  rightServo.write(servoStops[1] - 50);
}

void motorSpeedsRightBk() {             //Turns right servo forward (robot turns left, reverse)
  delay(20);
  rightServo.write(servoStops[1] + 50);
}

void motorSpeedsBoth() {               //Turns right servo backwards and left servo forward, robot goes forward
  rightServo.write(servoStops[1] - 50);
  delay(20);
  leftServo.write(servoStops[0] + 50);
  delay(20);
}

void motorSpeedsStop() {              //Sets speed of both servos to calibrated 'stop speed', robot stops
  leftServo.write(servoStops[0]);
  delay(20);
  rightServo.write(servoStops[1]);
  delay(20);
}


void irRead() {                             //Turns on the ir led, waits half a second and reads the digital signal from the ir reciever.
                                            //If reciever detects ir signal, it increases 'irCount' by 1, else resets it to 0
  tone(irBlaster, 38000);
  delay(500);
  if (digitalRead(2) == LOW) {
    Serial.println("Somethings there");

    irCount++;
  } else {
    Serial.println("Nothings there");
    irCount = 0;
  }

  noTone(irBlaster);
}

void obstacleAvoidance() {            //Invokes 'irRead()' three times in a row, if each time its if statement is true, irCount will be more than 3 and avoidance sequence will start
  irRead();
  delay(50);
  irRead();
  delay(50);
  irRead();
  if (irCount >= 3) {
    motorSpeedsRightBk();
    delay(3000);
    motorSpeedsStop();
    delay(100);
    motorSpeedsBoth();
    delay(3000);
    motorSpeedsStop();
    delay(100);
    motorSpeedsRight();
    delay(3000);
    motorSpeedsStop();
    delay(100);
    motorSpeedsBoth();
    delay(3000);
    motorSpeedsStop();
    delay(100);
    motorSpeedsRight();
    delay(3000);
    motorSpeedsStop();


    while (rightSensor > 600 || rightSensor > 350) {
      while ((rightSensor > 600 || rightSensor > 350) && (leftSensor > 600 || leftSensor > 350)) { //Once obstacle is avoided, robot will go forward and 'ldrDetect()' will execute as long as both side ldrs are on white surface
        motorSpeedsBoth();                                                                         //When only one side ldr is reading above the threshhold, robot will turn left and exit out of the loop
        ldrDetect();
        delay(100);
      }
      motorSpeedsStop();
      motorSpeedsLeft();
      ldrDetect();

      delay(100);
    }
  }
  Serial.println("Obstacle avoidance complete");
}

void ldrDetect() {                              //Will assign midSensor/rightSensor/leftSensor variables based off readings from the respective sensors and print them out
  midSensor = analogRead(A1);
  delay(20);
  rightSensor = analogRead(A0);
  delay(20);
  leftSensor = analogRead(A2);
  Serial.println(" ");
  Serial.print(leftSensor);
  Serial.print(" ");
  Serial.print(midSensor);
  Serial.print(" ");
  Serial.print(rightSensor);
}

void loop() {

#ifdef RLSE
  obstacleAvoidance();                                                                      //Check if theres an obstacle in front

  ldrDetect();                                                                              //Check if on path



  delay(20);

  while (rightSensor < 600 || rightSensor < 350) {                                           //If only right sensor is on black or gray line, turn right and check if condition is still true. If condition is no longer true, break
    if ((rightSensor < 600 || rightSensor < 350) && (leftSensor < 600 || leftSensor < 350)) {
      break;
    }
    motorSpeedsStop();
    motorSpeedsLeft();
    ldrDetect();
    delay(100);
  }



  while (leftSensor < 600 || leftSensor < 350) {                                           //If only left sensor is on black or gray line, turn left and check if condition is still true. If condition is no longer true, break
    if ((rightSensor < 600 || rightSensor < 350) && (leftSensor < 600 || leftSensor < 350)) {
      break;
    }
    motorSpeedsStop();
    motorSpeedsRight();
    ldrDetect();
    delay(100);
  }

  delay(20);

  //If middle sensor is on black or gray line, drive forward and check if condition is true. If condition is no longer true, break
  if (midSensor > 460 && midSensor <= 590) {
    delay(10);
    motorSpeedsBoth();
  } else {
    motorSpeedsStop();
  }



  delay(20);

#endif

}
