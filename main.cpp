#include <SoftwareSerial.h>

// HC-05: Arduino pin 2 receives, pin 3 transmits
SoftwareSerial bluetooth(2, 3);

// L298N motor driver pins
const int ENA = 5;  // Left motor speed (PWM)
const int IN1 = 7;
const int IN2 = 8;

const int ENB = 6;  // Right motor speed (PWM)
const int IN3 = 9;
const int IN4 = 10;

// HC-SR04 ultrasonic sensor pins
const int TRIG_PIN = 12;
const int ECHO_PIN = 11;

const int MOTOR_SPEED = 170;       // 0–255
const int SAFE_DISTANCE_CM = 20;

bool autonomousMode = false;
char manualCommand = 'S';

void setup() {
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  bluetooth.begin(9600);
  stopRobot();
}

void loop() {
  readBluetooth();

  if (autonomousMode) {
    avoidObstacles();
  } else {
    runManualCommand();
  }
}

void readBluetooth() {
  while (bluetooth.available() > 0) {
    char command = bluetooth.read();

    // Convert lowercase commands to uppercase
    if (command >= 'a' && command <= 'z') {
      command = command - 'a' + 'A';
    }

    switch (command) {
      case 'A':
        autonomousMode = true;
        manualCommand = 'S';
        stopRobot();
        break;

      case 'M':
        autonomousMode = false;
        manualCommand = 'S';
        stopRobot();
        break;

      case 'F':
      case 'B':
      case 'L':
      case 'R':
      case 'S':
        autonomousMode = false;
        manualCommand = command;
        break;
    }
  }
}

long getDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Timeout prevents the Arduino from waiting too long for an echo
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return -1;  // No echo received
  }

  return duration * 0.0343 / 2;
}

void avoidObstacles() {
  long distance = getDistanceCm();

  if (distance > SAFE_DISTANCE_CM) {
    moveForward();
  } else {
    stopRobot();
    delay(150);

    moveBackward();
    delay(300);

    stopRobot();
    delay(100);

    turnRight();
    delay(350);

    stopRobot();
    delay(100);
  }
}

void runManualCommand() {
  switch (manualCommand) {
    case 'F':
      moveForward();
      break;
    case 'B':
      moveBackward();
      break;
    case 'L':
      turnLeft();
      break;
    case 'R':
      turnRight();
      break;
    default:
      stopRobot();
      break;
  }
}

void moveForward() {
  setMotors(MOTOR_SPEED, MOTOR_SPEED, true, true);
}

void moveBackward() {
  setMotors(MOTOR_SPEED, MOTOR_SPEED, false, false);
}

void turnLeft() {
  setMotors(MOTOR_SPEED, MOTOR_SPEED, false, true);
}

void turnRight() {
  setMotors(MOTOR_SPEED, MOTOR_SPEED, true, false);
}

void stopRobot() {
  setMotors(0, 0, true, true);
}

void setMotors(int leftSpeed, int rightSpeed,
               bool leftForward, bool rightForward) {
  digitalWrite(IN1, leftForward ? HIGH : LOW);
  digitalWrite(IN2, leftForward ? LOW : HIGH);

  digitalWrite(IN3, rightForward ? HIGH : LOW);
  digitalWrite(IN4, rightForward ? LOW : HIGH);

  analogWrite(ENA, leftSpeed);
  analogWrite(ENB, rightSpeed);
}
