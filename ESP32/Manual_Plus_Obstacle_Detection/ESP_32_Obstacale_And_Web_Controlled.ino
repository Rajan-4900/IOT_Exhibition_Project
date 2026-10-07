#include <WiFi.h>
#include <WebServer.h>

// =====================================================
// WIFI SETTINGS
// =====================================================
const char* apSsid     = "ESP32_Car";
const char* apPassword = "12345678";

WebServer server(80);

// =====================================================
// L298N MOTOR PINS
// =====================================================

// LEFT MOTOR
#define IN1 14
#define IN2 27

// RIGHT MOTOR
#define IN3 26
#define IN4 25

// L298N ENABLE PINS
#define ENA_PIN 33
#define ENB_PIN 32

// =====================================================
// PWM SETTINGS
// =====================================================

const int PWM_FREQ = 20000;
const int PWM_RES  = 8;

// Speed
uint8_t manualSpeed = 230;
uint8_t autoSpeed   = 200;

// =====================================================
// ULTRASONIC SENSOR
// =====================================================

#define TRIG_PIN 4
#define ECHO_PIN 5

// Obstacle distance
const float OBSTACLE_DIST_CM = 20.0;

// =====================================================
// MANUAL COMMAND FLAGS
// =====================================================

bool forwardCmd = false;
bool backCmd    = false;
bool leftCmd    = false;
bool rightCmd   = false;

// false = Manual
// true  = Automatic obstacle avoidance
bool obstacleMode = false;

// =====================================================
// AUTO STATE
// =====================================================

enum AutoState {
  AUTO_IDLE,
  AUTO_FORWARD,
  AUTO_BACKWARD,
  AUTO_TURN
};

AutoState autoState = AUTO_IDLE;

unsigned long autoStateStart = 0;

bool turnLeftNext = true;

// Auto timings
const unsigned long BACK_TIME = 600;
const unsigned long TURN_TIME = 700;


// =====================================================
// HTML WEB PAGE
// =====================================================

const char index_html[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta charset="UTF-8">

<meta name="viewport"
      content="width=device-width,
               initial-scale=1.0,
               maximum-scale=1.0,
               user-scalable=no">

<title>ESP32 Car Control</title>

<style>

* {
  box-sizing: border-box;
  -webkit-user-select: none;
  user-select: none;
}

body {

  margin: 0;

  padding: 20px;

  background: #111;

  color: white;

  font-family: Arial, sans-serif;

  text-align: center;

}

h1 {
  margin-bottom: 5px;
}

.status {

  margin: 10px auto;

  padding: 12px;

  max-width: 400px;

  background: #222;

  border-radius: 10px;

}

#connection {
  color: #00ff66;
}

.mode {

  margin: 20px auto;

  max-width: 500px;

}

button {

  border: none;

  border-radius: 12px;

  padding: 20px;

  margin: 6px;

  min-width: 110px;

  min-height: 65px;

  font-size: 18px;

  font-weight: bold;

  color: white;

  background: #333;

  touch-action: none;

}

button:active {

  transform: scale(0.95);

  background: #555;

}

.forward {
  background: #1565c0;
}

.backward {
  background: #6a1b9a;
}

.left {
  background: #ef6c00;
}

.right {
  background: #ef6c00;
}

.stop {
  background: #c62828;
}

.auto {
  background: #2e7d32;
}

.manual {
  background: #455a64;
}

.row {
  margin: 5px;
}

.info {

  margin-top: 20px;

  background: #222;

  padding: 15px;

  border-radius: 10px;

}

</style>

</head>


<body>

<h1>🚗 ESP32 Car</h1>

<div class="status">

  <div>
    Connection:
    <span id="connection">Connected</span>
  </div>

  <div>
    Mode:
    <span id="mode">MANUAL</span>
  </div>

</div>


<!-- ========================= -->
<!-- MANUAL CONTROL -->
<!-- ========================= -->

<h2>Manual Control</h2>

<div class="row">

<button class="forward"
        onpointerdown="sendCmd('f',1)"
        onpointerup="sendCmd('f',0)"
        onpointercancel="sendCmd('f',0)">
  ▲<br>Forward
</button>

</div>


<div class="row">

<button class="left"
        onpointerdown="sendCmd('l',1)"
        onpointerup="sendCmd('l',0)"
        onpointercancel="sendCmd('l',0)">
  ◀<br>Left
</button>


<button class="stop"
        onclick="stopCar()">
  STOP
</button>


<button class="right"
        onpointerdown="sendCmd('r',1)"
        onpointerup="sendCmd('r',0)"
        onpointercancel="sendCmd('r',0)">
  ▶<br>Right
</button>

</div>


<div class="row">

<button class="backward"
        onpointerdown="sendCmd('b',1)"
        onpointerup="sendCmd('b',0)"
        onpointercancel="sendCmd('b',0)">
  ▼<br>Backward
</button>

</div>


<!-- ========================= -->
<!-- MODE CONTROL -->
<!-- ========================= -->

<div class="mode">

<h2>Driving Mode</h2>

<button class="auto"
        onclick="setMode(1)">
  🤖 AUTO MODE
</button>

<button class="manual"
        onclick="setMode(0)">
  🎮 MANUAL MODE
</button>

</div>


<div class="info">

<h3>Ultrasonic Sensor</h3>

<div>
Distance:
<span id="distance">--</span> cm
</div>

</div>


<script>


// =====================================================
// SEND COMMAND
// =====================================================

function sendCmd(direction, state) {

  fetch(
    "/cmd?dir=" +
    direction +
    "&state=" +
    state
  )
  .then(response => {

    if (!response.ok) {

      console.log(
        "Command error:",
        response.status
      );

    }

  })
  .catch(error => {

    console.log(
      "Connection error:",
      error
    );

  });

}


// =====================================================
// STOP
// =====================================================

function stopCar() {

  sendCmd("s", 1);

}


// =====================================================
// CHANGE MODE
// =====================================================

function setMode(auto) {

  fetch(
    "/mode?auto=" +
    auto
  )

  .then(response => {

    if (response.ok) {

      if (auto == 1) {

        document.getElementById(
          "mode"
        ).innerText = "AUTO";

      }

      else {

        document.getElementById(
          "mode"
        ).innerText = "MANUAL";

      }

    }

  })

  .catch(error => {

    console.log(
      "Mode error:",
      error
    );

  });

}


// =====================================================
// STOP CAR IF FINGER/MOUSE LEAVES PAGE
// =====================================================

document.addEventListener(
  "pointercancel",
  function() {

    stopCar();

  }
);


document.addEventListener(
  "visibilitychange",
  function() {

    if (document.hidden) {

      stopCar();

    }

  }
);

</script>


</body>

</html>

)rawliteral";


// =====================================================
// MOTOR SPEED
// =====================================================

void setMotorSpeed(
  uint8_t leftSpeed,
  uint8_t rightSpeed
) {

  // IMPORTANT:
  // New ESP32 Arduino LEDC API uses PIN numbers

  ledcWrite(
    ENA_PIN,
    leftSpeed
  );

  ledcWrite(
    ENB_PIN,
    rightSpeed
  );
}


// =====================================================
// STOP
// =====================================================

void driveStop() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  setMotorSpeed(0, 0);

}


// =====================================================
// FORWARD
// =====================================================

void driveForward(
  uint8_t speed
) {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  setMotorSpeed(
    speed,
    speed
  );

}


// =====================================================
// BACKWARD
// =====================================================

void driveBackward(
  uint8_t speed
) {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  setMotorSpeed(
    speed,
    speed
  );

}


// =====================================================
// LEFT
// =====================================================

void driveLeft(
  uint8_t speed
) {

  // LEFT MOTOR BACKWARD
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  // RIGHT MOTOR FORWARD
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  setMotorSpeed(
    speed,
    speed
  );

}


// =====================================================
// RIGHT
// =====================================================

void driveRight(
  uint8_t speed
) {

  // LEFT MOTOR FORWARD
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  // RIGHT MOTOR BACKWARD
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  setMotorSpeed(
    speed,
    speed
  );

}


// =====================================================
// ULTRASONIC DISTANCE
// =====================================================

float getDistanceCm() {

  digitalWrite(
    TRIG_PIN,
    LOW
  );

  delayMicroseconds(2);

  digitalWrite(
    TRIG_PIN,
    HIGH
  );

  delayMicroseconds(10);

  digitalWrite(
    TRIG_PIN,
    LOW
  );


  unsigned long duration =
    pulseIn(
      ECHO_PIN,
      HIGH,
      30000UL
    );


  if (duration == 0) {

    return -1.0;

  }


  float distance =
    (duration * 0.0343) / 2.0;


  return distance;

}


// =====================================================
// MANUAL MOVEMENT
// =====================================================

void handleManualMovement() {

  if (forwardCmd) {

    driveForward(
      manualSpeed
    );

  }

  else if (backCmd) {

    driveBackward(
      manualSpeed
    );

  }

  else if (leftCmd) {

    driveLeft(
      manualSpeed
    );

  }

  else if (rightCmd) {

    driveRight(
      manualSpeed
    );

  }

  else {

    driveStop();

  }

}


// =====================================================
// AUTO OBSTACLE AVOIDANCE
// =====================================================

void handleAutoObstacle() {

  unsigned long now =
    millis();


  switch (autoState) {


    // =================================
    // IDLE
    // =================================

    case AUTO_IDLE:

      driveStop();

      autoState =
        AUTO_FORWARD;

      break;


    // =================================
    // MOVE FORWARD
    // =================================

    case AUTO_FORWARD:
    {

      float distance =
        getDistanceCm();


      Serial.print(
        "Distance: "
      );

      Serial.println(
        distance
      );


      if (
        distance > 0 &&
        distance < OBSTACLE_DIST_CM
      ) {

        Serial.println(
          "Obstacle detected!"
        );

        driveStop();


        autoState =
          AUTO_BACKWARD;


        autoStateStart =
          now;

      }

      else {

        driveForward(
          autoSpeed
        );

      }

      break;

    }


    // =================================
    // BACKWARD
    // =================================

    case AUTO_BACKWARD:

      if (
        now -
        autoStateStart
        >= BACK_TIME
      ) {

        driveStop();


        autoState =
          AUTO_TURN;


        autoStateStart =
          now;

      }

      else {

        driveBackward(
          autoSpeed
        );

      }

      break;


    // =================================
    // TURN
    // =================================

    case AUTO_TURN:

      if (turnLeftNext) {

        Serial.println(
          "Turning LEFT"
        );

        driveLeft(
          autoSpeed
        );

      }

      else {

        Serial.println(
          "Turning RIGHT"
        );

        driveRight(
          autoSpeed
        );

      }


      if (
        now -
        autoStateStart
        >= TURN_TIME
      ) {

        driveStop();


        turnLeftNext =
          !turnLeftNext;


        autoState =
          AUTO_FORWARD;

      }

      break;

  }

}


// =====================================================
// ROOT PAGE
// =====================================================

void handleRoot() {

  server.send_P(
    200,
    "text/html",
    index_html
  );

}


// =====================================================
// COMMAND HANDLER
// =====================================================

void handleCmd() {

  if (
    !server.hasArg("dir") ||
    !server.hasArg("state")
  ) {

    server.send(
      400,
      "text/plain",
      "Missing parameters"
    );

    return;

  }


  String dir =
    server.arg("dir");


  int state =
    server.arg("state").toInt();


  Serial.print(
    "COMMAND: "
  );

  Serial.print(
    dir
  );

  Serial.print(
    " | STATE: "
  );

  Serial.println(
    state
  );


  // Only manual mode accepts
  // manual movement commands

  if (!obstacleMode) {

    bool pressed =
      (state != 0);


    // FORWARD

    if (dir == "f") {

      forwardCmd =
        pressed;


      if (pressed) {

        backCmd =
          leftCmd =
          rightCmd =
          false;

      }

    }


    // BACKWARD

    else if (dir == "b") {

      backCmd =
        pressed;


      if (pressed) {

        forwardCmd =
          leftCmd =
          rightCmd =
          false;

      }

    }


    // LEFT

    else if (dir == "l") {

      leftCmd =
        pressed;


      if (pressed) {

        forwardCmd =
          backCmd =
          rightCmd =
          false;

      }

    }


    // RIGHT

    else if (dir == "r") {

      rightCmd =
        pressed;


      if (pressed) {

        forwardCmd =
          backCmd =
          leftCmd =
          false;

      }

    }


    // STOP

    else if (dir == "s") {

      forwardCmd =
        false;

      backCmd =
        false;

      leftCmd =
        false;

      rightCmd =
        false;


      driveStop();

    }

  }


  server.send(
    200,
    "text/plain",
    "OK"
  );

}


// =====================================================
// MODE HANDLER
// =====================================================

void handleMode() {

  if (!server.hasArg("auto")) {

    server.send(
      400,
      "text/plain",
      "Missing auto parameter"
    );

    return;

  }


  int autoVal =
    server.arg("auto").toInt();


  Serial.print(
    "MODE: "
  );

  Serial.println(
    autoVal
  );


  // =================================
  // AUTO MODE
  // =================================

  if (autoVal == 1) {

    obstacleMode =
      true;


    forwardCmd =
      false;

    backCmd =
      false;

    leftCmd =
      false;

    rightCmd =
      false;


    autoState =
      AUTO_FORWARD;


    autoStateStart =
      millis();


    Serial.println(
      "AUTO MODE ON"
    );

  }


  // =================================
  // MANUAL MODE
  // =================================

  else {

    obstacleMode =
      false;


    autoState =
      AUTO_IDLE;


    driveStop();


    Serial.println(
      "MANUAL MODE ON"
    );

  }


  server.send(
    200,
    "text/plain",
    "Mode changed"
  );

}


// =====================================================
// NOT FOUND
// =====================================================

void handleNotFound() {

  server.send(
    404,
    "text/plain",
    "Not found"
  );

}


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(
    115200
  );


  // =================================
  // MOTOR PINS
  // =================================

  pinMode(
    IN1,
    OUTPUT
  );

  pinMode(
    IN2,
    OUTPUT
  );

  pinMode(
    IN3,
    OUTPUT
  );

  pinMode(
    IN4,
    OUTPUT
  );


  // =================================
  // PWM PINS
  // =================================

  pinMode(
    ENA_PIN,
    OUTPUT
  );

  pinMode(
    ENB_PIN,
    OUTPUT
  );


  // =================================
  // ESP32 PWM
  // =================================

  ledcAttach(
    ENA_PIN,
    PWM_FREQ,
    PWM_RES
  );

  ledcAttach(
    ENB_PIN,
    PWM_FREQ,
    PWM_RES
  );


  // Stop motors initially

  driveStop();


  // =================================
  // ULTRASONIC
  // =================================

  pinMode(
    TRIG_PIN,
    OUTPUT
  );

  pinMode(
    ECHO_PIN,
    INPUT
  );


  // =================================
  // WIFI ACCESS POINT
  // =================================

  WiFi.mode(
    WIFI_AP
  );


  WiFi.softAP(
    apSsid,
    apPassword
  );


  Serial.println();
  Serial.println(
    "================================"
  );

  Serial.println(
    "ESP32 CAR STARTED"
  );

  Serial.println(
    "================================"
  );


  Serial.print(
    "WiFi Name: "
  );

  Serial.println(
    apSsid
  );


  Serial.print(
    "WiFi Password: "
  );

  Serial.println(
    apPassword
  );


  Serial.print(
    "Open Chrome: http://"
  );

  Serial.println(
    WiFi.softAPIP()
  );


  Serial.println(
    "================================"
  );


  // =================================
  // WEB SERVER
  // =================================

  server.on(
    "/",
    handleRoot
  );


  server.on(
    "/cmd",
    handleCmd
  );


  server.on(
    "/mode",
    handleMode
  );


  server.onNotFound(
    handleNotFound
  );


  server.begin();


  Serial.println(
    "HTTP SERVER STARTED"
  );

}


// =====================================================
// LOOP
// =====================================================

void loop() {

  // Handle browser requests

  server.handleClient();


  // =================================
  // AUTO MODE
  // =================================

  if (obstacleMode) {

    handleAutoObstacle();

  }


  // =================================
  // MANUAL MODE
  // =================================

  else {

    handleManualMovement();

  }


  delay(5);

}