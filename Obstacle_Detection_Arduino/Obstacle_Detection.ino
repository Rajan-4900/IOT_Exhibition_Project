#include <Servo.h>

// ==========================================
// PIN CONNECTIONS
// ==========================================

// Ultrasonic
#define TRIG_PIN 8
#define ECHO_PIN 9

// Servo
#define SERVO_PIN 10

// L298N
#define ENA 5
#define ENB 6

#define IN1 7
#define IN2 11
#define IN3 12
#define IN4 13

// ==========================================
// SETTINGS
// ==========================================

#define OBSTACLE_DISTANCE 35

// Motor speed
// Left side is slower, so give it slightly more power
int leftSpeed = 120;
int rightSpeed = 240;

// How long the robot turns when avoiding
int turnTime = 2200;

// Reverse time if both sides are blocked
int reverseTime = 800;

// Servo scan positions
int centerAngle = 90;
int leftAngle = 30;
int rightAngle = 150;

// Time for servo to reach position
int servoSettleTime = 700;

// ==========================================
// VARIABLES
// ==========================================

Servo scannerServo;

int frontDistance = 0;
int leftDistance = 0;
int rightDistance = 0;

// ==========================================
// SETUP
// ==========================================

void setup() {

  Serial.begin(9600);

  // Ultrasonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Motor driver
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Servo
  scannerServo.attach(SERVO_PIN);

  // Start servo at center
  scannerServo.write(centerAngle);

  delay(1000);

  // Stop motors at startup
  stopMotors();

  Serial.println("SMART OBSTACLE AVOIDANCE READY");
}

// ==========================================
// MAIN LOOP
// ==========================================

void loop() {

  // Always check front
  frontDistance = getDistance();

  Serial.print("Front Distance: ");
  Serial.print(frontDistance);
  Serial.println(" cm");

  // ========================================
  // NO OBSTACLE
  // CONTINUE MOVING FORWARD
  // ========================================

  if (frontDistance > OBSTACLE_DISTANCE) {

    moveForward();

    // Keep checking continuously
    delay(80);
  }

  // ========================================
  // OBSTACLE DETECTED
  // ========================================

  else {

    Serial.println("======================");
    Serial.println("OBSTACLE DETECTED");
    Serial.println("======================");

    // Stop immediately
    stopMotors();

    delay(400);

    // ======================================
    // SCAN LEFT
    // ======================================

    Serial.println("Scanning LEFT...");

    scannerServo.write(leftAngle);

    delay(servoSettleTime);

    leftDistance = getDistance();

    Serial.print("LEFT = ");
    Serial.print(leftDistance);
    Serial.println(" cm");

    delay(300);

    // ======================================
    // RETURN TO CENTER
    // ======================================

    scannerServo.write(centerAngle);

    delay(500);

    // ======================================
    // SCAN RIGHT
    // ======================================

    Serial.println("Scanning RIGHT...");

    scannerServo.write(rightAngle);

    delay(servoSettleTime);

    rightDistance = getDistance();

    Serial.print("RIGHT = ");
    Serial.print(rightDistance);
    Serial.println(" cm");

    delay(300);

    // ======================================
    // RETURN SERVO TO CENTER
    // ======================================

    scannerServo.write(centerAngle);

    delay(500);

    // ======================================
    // DECIDE WHICH SIDE IS FREE
    // ======================================

    Serial.println("Checking best direction...");

    // -----------------------------
    // LEFT HAS MORE SPACE
    // -----------------------------

    if (leftDistance > rightDistance &&
        leftDistance > OBSTACLE_DISTANCE) {

      Serial.println("LEFT SIDE IS CLEAR");
      Serial.println("Turning LEFT...");

      turnLeft();

      // Long rotation
      delay(turnTime);

      stopMotors();

      delay(300);
    }

    // -----------------------------
    // RIGHT HAS MORE SPACE
    // -----------------------------

    else if (rightDistance > leftDistance &&
             rightDistance > OBSTACLE_DISTANCE) {

      Serial.println("RIGHT SIDE IS CLEAR");
      Serial.println("Turning RIGHT...");

      turnRight();

      // Long rotation
      delay(turnTime);

      stopMotors();

      delay(300);
    }

    // -----------------------------
    // BOTH SIDES BLOCKED
    // -----------------------------

    else {

      Serial.println("BOTH SIDES BLOCKED");
      Serial.println("REVERSING...");

      moveBackward();

      delay(reverseTime);

      stopMotors();

      delay(300);

      // Turn right as backup
      Serial.println("Backup RIGHT turn...");

      turnRight();

      delay(turnTime);

      stopMotors();

      delay(300);
    }

    // After turning, the loop starts again.
    // Robot checks front and continues forward
    // if the path is clear.
  }
}

// ==========================================
// ULTRASONIC DISTANCE
// ==========================================

int getDistance() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  // No echo
  if (duration == 0) {
    return 400;
  }

  int distance = duration * 0.034 / 2;

  // Limit distance
  if (distance <= 0) {
    distance = 400;
  }

  if (distance > 400) {
    distance = 400;
  }

  return distance;
}

// ==========================================
// MOVE FORWARD
// ==========================================

void moveForward() {

  // LEFT
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  // RIGHT
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, leftSpeed);
  analogWrite(ENB, rightSpeed);
}

// ==========================================
// MOVE BACKWARD
// ==========================================

void moveBackward() {

  // LEFT
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  // RIGHT
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, leftSpeed);
  analogWrite(ENB, rightSpeed);
}

// ==========================================
// TURN LEFT
// ==========================================

void turnLeft() {

  // LEFT BACKWARD
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  // RIGHT FORWARD
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, leftSpeed);
  analogWrite(ENB, rightSpeed);
}

// ==========================================
// TURN RIGHT
// ==========================================

void turnRight() {

  // LEFT FORWARD
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  // RIGHT BACKWARD
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, leftSpeed);
  analogWrite(ENB, rightSpeed);
}

// ==========================================
// STOP
// ==========================================

void stopMotors() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}