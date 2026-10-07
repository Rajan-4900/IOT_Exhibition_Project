# 🚗 Autonomous Robot Car

![Platform](https://img.shields.io/badge/platform-Arduino%20%7C%20ESP32-informational)
![Driver](https://img.shields.io/badge/motor%20driver-L298N-orange)
![Sensor](https://img.shields.io/badge/sensor-HC--SR04-blue)
![Status](https://img.shields.io/badge/status-active-brightgreen)

A two-wheel-drive (differential drive) robot car built around an **L298N** motor driver and an **HC‑SR04** ultrasonic sensor, with two interchangeable "brains" living side by side in this repo:

| | Platform | Sketch | Behavior |
|---|---|---|---|
| 🔧 | **Arduino** (Uno/Nano/etc.) | [`Obstacle_Detection.ino`](Obstacle_Detection_Arduino/Obstacle_Detection.ino) | 🧭 Fully autonomous — a **servo-mounted** ultrasonic "head" scans left/right and picks the clearer path. |
| 📡 | **ESP32** | [`ESP_32_Obstacale_And_Web_Controlled.ino`](ESP32/Manual_Plus_Obstacle_Detection/ESP_32_Obstacale_And_Web_Controlled.ino) | 🕹️ Dual-mode — drive it live from a **phone/laptop web page**, or flip it into 🤖 **auto-pilot** on the fly. |

> 🧪 **Bonus:** [`ObstacleDetectionRobot.ino`](ESP32/ObstacleDetectionRobot/ObstacleDetectionRobot.ino) is a bare-bones motor bench-test sketch (cycles each motor forward/backward/both) — a wiring sanity check, not a driving mode, so it isn't detailed below.

---

## 📋 Table of Contents

- [🔧 Arduino — Autonomous Obstacle Avoidance](#-arduino--autonomous-obstacle-avoidance)
- [📡 ESP32 — Wi-Fi Control + Auto Obstacle Avoidance](#-esp32--wi-fi-control--auto-obstacle-avoidance)
- [⚖️ Comparing the Two Brains](#️-comparing-the-two-brains)

---

## 🔧 Arduino — Autonomous Obstacle Avoidance

📁 `Obstacle_Detection_Arduino/Obstacle_Detection.ino`

### 🧭 Concept

The robot drives forward on its own. When its front ultrasonic "eye" spots an obstacle inside the danger zone, it **halts**, swivels its servo-mounted sensor **left**, then **right**, compares the two readings, and steers toward whichever side has more breathing room. If both sides are boxed in, it **reverses** and turns right as an escape maneuver.

```
   🛑 obstacle!
        │
   ┌────┴────┐
   │  stop    │
   └────┬────┘
        │
  👀 scan LEFT ──► 👀 scan RIGHT
        │                │
        └──────┬─────────┘
               ▼
     which side is clearer?
      ↙         ↓        ↘
  ⬅️ turn    🚧 both      ➡️ turn
    left     blocked        right
               │
          🔙 reverse
               │
          ➡️ turn right
```

### 🔌 Hardware & Pin Map

| Icon | Function | Component | Pin |
|---|---|---|---|
| 📏 | Ultrasonic Trigger | HC-SR04 `TRIG` | D8 |
| 👂 | Ultrasonic Echo | HC-SR04 `ECHO` | D9 |
| 🪬 | Scanner servo | SG90 (or similar) | D10 |
| ⚡ | Left motor speed (PWM) | L298N `ENA` | D5 |
| ⚡ | Right motor speed (PWM) | L298N `ENB` | D6 |
| ↔️ | Left motor direction | L298N `IN1` | D7 |
| ↔️ | Left motor direction | L298N `IN2` | D11 |
| ↔️ | Right motor direction | L298N `IN3` | D12 |
| ↔️ | Right motor direction | L298N `IN4` | D13 |

📦 Requires the built-in **`Servo.h`** library (ships with the Arduino IDE).

### 🎛️ Key Tunables

| Knob | Default | What it controls |
|---|---|---|
| 🚧 `OBSTACLE_DISTANCE` | `35` cm | Trip-wire distance that triggers avoidance |
| 🐢🐇 `leftSpeed` / `rightSpeed` | `120` / `240` | Per-side PWM duty (0–255) — right runs hotter to offset a mechanically slower left side |
| ⏱️ `turnTime` | `2200` ms | Duration of an avoidance turn |
| ⏪ `reverseTime` | `800` ms | Duration of the backup maneuver |
| 🎯 `centerAngle`/`leftAngle`/`rightAngle` | `90°`/`30°`/`150°` | Servo sweep positions |
| ⏳ `servoSettleTime` | `700` ms | Pause so the servo physically arrives before a reading is taken |

### 🔁 Program Flow (`loop()`)

1. 📏 **Measure front distance** via `getDistance()`.
2. ✅ **Path clear** → `moveForward()`, re-check after 80 ms.
3. 🛑 **Obstacle detected**:
   - `stopMotors()` instantly.
   - 👀⬅️ Scan **left** → read `leftDistance` → recenter.
   - 👀➡️ Scan **right** → read `rightDistance` → recenter.
   - 🤔 **Decide:**
     - 🟢⬅️ Left clearer & open → `turnLeft()`
     - 🟢➡️ Right clearer & open → `turnRight()`
     - 🔴 Both blocked → `moveBackward()` then `turnRight()` as a fallback
   - 🔄 Loop restarts, re-checking the front.

### ⚙️ Core Functions

| Function | Role |
|---|---|
| 📏 `getDistance()` | Pings `TRIG`, times the `ECHO` pulse, converts to cm; returns `400` (clear) on timeout |
| ⬆️ `moveForward()` / ⬇️ `moveBackward()` | Both motors same direction, per-side speed |
| ↩️ `turnLeft()` / ↪️ `turnRight()` | Tank-style pivot (one side forward, one back) |
| ⏹️ `stopMotors()` | All direction pins LOW, PWM duty 0 |

### 🖥️ Serial Debugging

🔌 **9600 baud** — streams live distance readings plus bannered status lines (`OBSTACLE DETECTED`, `Scanning LEFT/RIGHT...`, `BOTH SIDES BLOCKED`, …) for easy tuning via the Serial Monitor.

### ✅ Setup Checklist

- [ ] 🔌 Wire L298N + HC-SR04 + servo per the pin map; power motors from a separate battery sharing ground with the Arduino
- [ ] 📦 Install the `Servo` library (built-in)
- [ ] 🎯 Select board/port → **Upload**
- [ ] 🖥️ Open Serial Monitor @ 9600 baud
- [ ] 🎛️ Tune `OBSTACLE_DISTANCE`, speeds, `turnTime` for your chassis

---

## 📡 ESP32 — Wi-Fi Control + Auto Obstacle Avoidance

📁 `ESP32/Manual_Plus_Obstacle_Detection/ESP_32_Obstacale_And_Web_Controlled.ino`

### 🧭 Concept

The ESP32 broadcasts its own Wi-Fi **access point** 📶 and hosts a built-in **web app** (HTML/CSS/JS baked into flash via `PROGMEM`). From any phone or laptop browser you can:

- 🕹️ **Drive manually** with press-and-hold on-screen buttons, or
- 🤖 **Flip to AUTO** and let a non-blocking obstacle-avoidance state machine take the wheel.

⚡ Unlike the Arduino sketch, nothing here uses blocking `delay()` — everything runs on `millis()` timers so the **web server never freezes up**.

### 🔌 Hardware & Pin Map

| Icon | Function | Component | GPIO |
|---|---|---|---|
| ↔️ | Left motor direction | L298N `IN1` | 14 |
| ↔️ | Left motor direction | L298N `IN2` | 27 |
| ↔️ | Right motor direction | L298N `IN3` | 26 |
| ↔️ | Right motor direction | L298N `IN4` | 25 |
| ⚡ | Left motor speed (LEDC) | L298N `ENA` | 33 |
| ⚡ | Right motor speed (LEDC) | L298N `ENB` | 32 |
| 📏 | Ultrasonic Trigger | HC-SR04 `TRIG` | 4 |
| 👂 | Ultrasonic Echo | HC-SR04 `ECHO` | 5 |

📦 Requires the ESP32 Arduino core's **`WiFi.h`** + **`WebServer.h`**, and the pin-based LEDC API (`ledcAttach` / `ledcWrite`, core ≥ 3.x).

### 📶 Wi-Fi Access Point

| Setting | Value |
|---|---|
| 🏷️ SSID | `ESP32_Car` |
| 🔑 Password | `12345678` |
| 📡 Mode | Soft-AP (`WIFI_AP`) — no router needed |
| 🌐 URL | Printed on boot via Serial (typically `http://192.168.4.1/`) |

👉 Join `ESP32_Car` from your device, then browse to the printed IP.

### 🎚️ PWM Configuration

| Setting | Value |
|---|---|
| 🎵 Frequency | `20,000` Hz (inaudible whine) |
| 📐 Resolution | 8-bit (0–255 duty) |
| 🕹️ `manualSpeed` | `230` |
| 🤖 `autoSpeed` | `200` |

### 🖼️ Web Control Page

- 🌙 Dark-themed, mobile-first, zero external dependencies (inline CSS/JS)
- 🕹️ **Manual pad:** ▲ Forward · ◀ Left · ▶ Right · ▼ Backward — all **press-and-hold** (`onpointerdown`/`onpointerup`/`onpointercancel`); release = instant stop
- 🛑 Dedicated **STOP** button
- 🔁 **Mode switch:** 🤖 `AUTO MODE` ↔ 🎮 `MANUAL MODE`
- 🛡️ Safety net: loses finger/pointer or the tab goes to background (`visibilitychange`) → auto-stop
- 📊 Live status panel: connection + current mode
- 🔄 Talks to the ESP32 via plain `fetch()` calls — no WebSocket, no page reloads

### 🌐 HTTP Endpoints

| Route | Params | Purpose |
|---|---|---|
| `GET /` | — | 🖼️ Serves the control page |
| `GET /cmd` | `dir`=`f`/`b`/`l`/`r`/`s`, `state`=`0`/`1` | 🕹️ Set/clear a manual move flag (ignored in AUTO); `s` = instant stop |
| `GET /mode` | `auto`=`0`/`1` | 🔁 Switch MANUAL ↔ AUTO |
| *anything else* | — | ❌ `404 Not found` |

### 🕹️ Manual Mode Logic

Four flags (`forwardCmd`, `backCmd`, `leftCmd`, `rightCmd`) are mutually exclusive — pressing one clears the rest. Each loop tick, whichever flag is active drives the motors; none active → `driveStop()`.

### 🤖 Auto Mode — Non-blocking State Machine

```
   ┌─────────┐
   │ 🔵 IDLE │
   └────┬────┘
        ▼
   ┌───────────┐   🚧 obstacle <20cm   ┌────────────┐
   │ ▶️ FORWARD │ ───────────────────► │ ⏪ BACKWARD │
   └─────┬─────┘                       └──────┬─────┘
         ▲                                    ▼
         │                              ┌───────────┐
         └────────── after turn ────────┤ 🔄 TURN   │
                 (left/right alternates) └───────────┘
```

| State | Behavior |
|---|---|
| 🔵 `AUTO_IDLE` | Stop once → go to `AUTO_FORWARD` |
| ▶️ `AUTO_FORWARD` | Drive forward @ `autoSpeed`; obstacle `< 20 cm` → stop, go to `AUTO_BACKWARD` |
| ⏪ `AUTO_BACKWARD` | Reverse for `600 ms` → go to `AUTO_TURN` |
| 🔄 `AUTO_TURN` | Turn for `700 ms`, **alternating** left/right each time (avoids repeating into the same corner) → back to `AUTO_FORWARD` |

🔁 Switching to AUTO resets manual flags & re-enters `AUTO_FORWARD`; switching back to MANUAL resets to `AUTO_IDLE` and stops.

### ⚙️ Core Functions

| Function | Role |
|---|---|
| ⚡ `setMotorSpeed(l, r)` | `ledcWrite()` on both ENA/ENB pins |
| ⬆️⬇️↩️↪️ `driveForward/Backward/Left/Right(speed)` | Set direction pins + apply speed |
| ⏹️ `driveStop()` | All direction pins LOW, speed 0 |
| 📏 `getDistanceCm()` | Trigger/echo pulse → cm; `-1.0` on timeout |

### ✅ Setup Checklist

- [ ] 🔌 Wire L298N + HC-SR04 per the pin map; share ground with motor battery
- [ ] 📦 Install the ESP32 board package (Boards Manager → "esp32")
- [ ] 🎯 Select board/port → **Upload**
- [ ] 🖥️ Open Serial Monitor @ **115200 baud**, note the printed IP
- [ ] 📱 Join Wi-Fi `ESP32_Car` (password `12345678`)
- [ ] 🌐 Browse to the printed IP → drive 🕹️ or flip to 🤖 **AUTO**

---

## ⚖️ Comparing the Two Brains

| | 🔧 Arduino | 📡 ESP32 |
|---|---|---|
| 🎮 Control | Autonomous only | Manual (Wi-Fi) + Autonomous, live switch |
| 👀 Sensing | Servo-scanned, compares L vs R | Fixed forward-facing only |
| 🧠 Turn choice | Picks the clearer side | Always alternates |
| ⏱️ Timing model | Blocking `delay()` | Non-blocking `millis()` state machine |
| 🖥️ Interface | Serial Monitor only | Full web UI, self-hosted |
| ⚡ PWM API | `analogWrite()` (AVR) | `ledcAttach()` / `ledcWrite()` (ESP32 LEDC) |

🏁 Both share the same L298N chassis and ultrasonic-distance trick — the **Arduino** is a self-contained autonomous wanderer, while the **ESP32** is a remote-controllable car with an autonomous fallback mode baked in.
