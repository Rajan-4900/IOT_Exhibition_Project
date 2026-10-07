// =====================================================
// ESP32 + L298N MOTOR TEST
// Tests LEFT and RIGHT motors separately
// =====================================================

// LEFT MOTOR CHANNEL
#define IN1 26
#define IN2 27
#define ENA 25

// RIGHT MOTOR CHANNEL
#define IN3 14
#define IN4 13
#define ENB 33


void setup() {

  Serial.begin(115200);

  // Left motor
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  // Right motor
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  stopAll();

  Serial.println("================================");
  Serial.println("MOTOR TEST STARTING");
  Serial.println("================================");

  delay(2000);
}


void loop() {

  // ===================================================
  // TEST 1 - LEFT MOTOR FORWARD
  // ===================================================

  Serial.println("TEST 1: LEFT MOTOR FORWARD");

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 220);

  delay(3000);

  stopAll();
  delay(2000);


  // ===================================================
  // TEST 2 - LEFT MOTOR BACKWARD
  // ===================================================

  Serial.println("TEST 2: LEFT MOTOR BACKWARD");

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, 220);

  delay(3000);

  stopAll();
  delay(2000);


  // ===================================================
  // TEST 3 - RIGHT MOTOR FORWARD
  // ===================================================

  Serial.println("TEST 3: RIGHT MOTOR FORWARD");

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 220);

  delay(3000);

  stopAll();
  delay(2000);


  // ===================================================
  // TEST 4 - RIGHT MOTOR BACKWARD
  // ===================================================

  Serial.println("TEST 4: RIGHT MOTOR BACKWARD");

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, 220);

  delay(3000);

  stopAll();
  delay(2000);


  // ===================================================
  // TEST 5 - BOTH MOTORS FORWARD
  // ===================================================

  Serial.println("TEST 5: BOTH MOTORS FORWARD");

  // LEFT
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 220);

  // RIGHT
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 220);

  delay(3000);

  stopAll();
  delay(2000);


  // ===================================================
  // TEST 6 - BOTH MOTORS BACKWARD
  // ===================================================

  Serial.println("TEST 6: BOTH MOTORS BACKWARD");

  // LEFT
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, 220);

  // RIGHT
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, 220);

  delay(3000);

  stopAll();
  delay(3000);


  // ===================================================
  // TEST COMPLETE
  // ===================================================

  Serial.println("================================");
  Serial.println("ALL TESTS COMPLETE");
  Serial.println("================================");

  delay(3000);
}


// =====================================================
// STOP EVERYTHING
// =====================================================

void stopAll() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}