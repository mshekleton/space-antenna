/*
  ESP32-WROOM-DA (Freenove) — Alt/Az -> X/Y Mount Controller (FastAccelStepper)
  + Persistent XOFF
  + Maintenance positions (HOME/ZENITH/MOTOR/ACCESS, N/S/E/W, NE/NW/SE/SW)

  Key changes vs your prior sketch:
  - Removed all *_Low pins (they were undefined and also problematic on WROOM when mapped to flash pins).
  - Uses only WROOM-safe GPIOs (no GPIO6–11, no 34–39 as outputs, no >39).
  - LED_BUILTIN is defined if missing.

  Serial commands:
    AZ,ALT            e.g. 110,25         (Az deg, Alt deg)
    XOFF=NNN          e.g. XOFF=30        (CW degrees from North to +X axis; saved in NVS)
    XOFF?             prints current XOFF
    POS?              prints last commanded X/Y (deg + steps)
    HOME | ZENITH     -> X=90, Y=90
    MOTOR | ACCESS    -> X=90, Y=90
    N S E W           -> horizon cardinal (uses Alt=0, Az=dir, then converts using XOFF)
    NE NW SE SW       -> horizon intercardinal
    HELP              prints help

  Assumptions:
    - Azimuth convention: 0=N, 90=E, increasing clockwise.
    - XOFF is CW degrees from North to the mount +X axis (once set, persists).
    - Output X,Y are clamped to [0,180] degrees.
    - Steps per degree = 10 for both axes.

  Notes on limit pins:
    - This sketch does NOT home. It only defines limit pins for future use.
    - If your limit switches are wired, we can add homing logic next.
*/

#include <Arduino.h>
#include "FastAccelStepper.h"
#include <Preferences.h>
#include <math.h>

// --------------------------- LED ---------------------------
#ifndef LED_BUILTIN
#define LED_BUILTIN 2  // typical on ESP32 dev boards
#endif
#define LED_PIN LED_BUILTIN

// --------------------------- WROOM-SAFE PINS ---------------------------
// IMPORTANT: Do NOT use GPIO6..11 (SPI flash). Avoid GPIO34..39 for OUTPUT (input-only).

// X axis
#define dirPinStepperX     25
#define enablePinStepperX  26
#define stepPinStepperX    27
#define limitPinX          32   // input OK

// Y axis
#define dirPinStepperY     14
#define enablePinStepperY  33   // output-capable; avoids strap pins
#define stepPinStepperY    13
#define limitPinY          15   // input OK, but note: GPIO15 is a strap pin (usually fine if switch doesn't pull it at boot)

// --------------------------- CONFIG ---------------------------
#define STEPS_PER_DEG 10.0f

#define X_MIN_DEG 0.0f
#define X_MAX_DEG 180.0f
#define Y_MIN_DEG 0.0f
#define Y_MAX_DEG 180.0f

#define ALT_MIN_DEG 0.0f
#define ALT_MAX_DEG 90.0f

#define SPEED_US_PER_STEP 1000
#define ACCEL_STEPS_PER_S2 500

static const char* NVS_NAMESPACE = "xy_mount";
static const char* NVS_KEY_XOFF  = "xoff_deg";

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper* stepperX = nullptr;
FastAccelStepper* stepperY = nullptr;

Preferences prefs;
float g_xoff_deg = 0.0f;

// Track last commanded position (for POS?)
float g_last_x_deg = NAN;
float g_last_y_deg = NAN;
long  g_last_x_steps = 0;
long  g_last_y_steps = 0;

// --------------------------- MATH HELPERS ---------------------------
static inline float wrap360(float d) {
  float x = fmodf(d, 360.0f);
  if (x < 0) x += 360.0f;
  return x;
}
static inline float deg2rad(float d) { return d * (float)M_PI / 180.0f; }
static inline float rad2deg(float r) { return r * 180.0f / (float)M_PI; }
static inline float clampf(float v, float lo, float hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

// Rotate ENU vector about Up axis by clockwise degrees (looking down from above).
static void rotateAboutUpCW(float e, float n, float u, float cw_deg, float& e2, float& n2, float& u2) {
  float th = deg2rad(wrap360(cw_deg));
  float c = cosf(th);
  float s = sinf(th);
  e2 =  c * e + s * n;
  n2 = -s * e + c * n;
  u2 = u;
}

// Convert Alt/Az (Alt 0..90, Az 0=N,90=E) to ENU unit vector
static void altAzToENU(float alt_deg, float az_deg, float& e, float& n, float& u) {
  float alt = deg2rad(alt_deg);
  float az  = deg2rad(wrap360(az_deg));
  e = cosf(alt) * sinf(az);
  n = cosf(alt) * cosf(az);
  u = sinf(alt);
}

// Convert ENU vector in mount frame to X/Y angles (degrees)
// X = atan2( sqrt(E^2 + U^2), N )
// Y = atan2( sqrt(N^2 + U^2), E )
static void enuToXYAngles(float e, float n, float u, float& x_deg, float& y_deg) {
  float r = sqrtf(e*e + n*n + u*u);
  if (r <= 0.0f) { x_deg = 90.0f; y_deg = 90.0f; return; }
  e /= r; n /= r; u /= r;

  float x = atan2f(sqrtf(e*e + u*u), n);
  float y = atan2f(sqrtf(n*n + u*u), e);

  x_deg = clampf(rad2deg(x), X_MIN_DEG, X_MAX_DEG);
  y_deg = clampf(rad2deg(y), Y_MIN_DEG, Y_MAX_DEG);
}

static void altAzToXY(float alt_deg, float az_deg, float xoff_deg, float& x_deg, float& y_deg) {
  float e, n, u;
  altAzToENU(alt_deg, az_deg, e, n, u);

  // World -> mount frame: rotate CCW by xoff (equivalently CW by -xoff)
  float em, nm, um;
  rotateAboutUpCW(e, n, u, -xoff_deg, em, nm, um);

  enuToXYAngles(em, nm, um, x_deg, y_deg);
}

// --------------------------- MOTION HELPERS ---------------------------
static void moveXYDegrees(float x_deg, float y_deg) {
  x_deg = clampf(x_deg, X_MIN_DEG, X_MAX_DEG);
  y_deg = clampf(y_deg, Y_MIN_DEG, Y_MAX_DEG);

  long x_steps = lroundf(x_deg * STEPS_PER_DEG);
  long y_steps = lroundf(y_deg * STEPS_PER_DEG);

  long x_min_steps = (long)lroundf(X_MIN_DEG * STEPS_PER_DEG);
  long x_max_steps = (long)lroundf(X_MAX_DEG * STEPS_PER_DEG);
  long y_min_steps = (long)lroundf(Y_MIN_DEG * STEPS_PER_DEG);
  long y_max_steps = (long)lroundf(Y_MAX_DEG * STEPS_PER_DEG);

  if (x_steps < x_min_steps) x_steps = x_min_steps;
  if (x_steps > x_max_steps) x_steps = x_max_steps;
  if (y_steps < y_min_steps) y_steps = y_min_steps;
  if (y_steps > y_max_steps) y_steps = y_max_steps;

  g_last_x_deg = x_deg;
  g_last_y_deg = y_deg;
  g_last_x_steps = x_steps;
  g_last_y_steps = y_steps;

  Serial.print("Move X=");
  Serial.print(x_deg, 3);
  Serial.print(" Y=");
  Serial.print(y_deg, 3);
  Serial.print("  (steps X=");
  Serial.print(x_steps);
  Serial.print(" Y=");
  Serial.print(y_steps);
  Serial.println(")");

  if (stepperX) stepperX->moveTo(x_steps);
  if (stepperY) stepperY->moveTo(y_steps);

  // blink LED briefly (optional)
  digitalWrite(LED_PIN, HIGH);
  delay(20);
  digitalWrite(LED_PIN, LOW);
}

static void moveToHorizonAz(float az_deg) {
  float x_deg, y_deg;
  altAzToXY(0.0f, az_deg, g_xoff_deg, x_deg, y_deg);
  moveXYDegrees(x_deg, y_deg);
}

// --------------------------- SERIAL HELPERS ---------------------------
static void printHelp() {
  Serial.println();
  Serial.println("Commands:");
  Serial.println("  AZ,ALT            e.g. 110,25");
  Serial.println("  XOFF=NNN          e.g. XOFF=30     (CW deg from North to +X axis; saved)");
  Serial.println("  XOFF?             show current xoff");
  Serial.println("  POS?              show last commanded X/Y");
  Serial.println("Maintenance:");
  Serial.println("  HOME | ZENITH     -> X=90, Y=90");
  Serial.println("  MOTOR | ACCESS    -> X=90, Y=90");
  Serial.println("  N S E W           -> horizon cardinal points");
  Serial.println("  NE NW SE SW       -> horizon intercardinal points");
  Serial.println("  HELP              -> this help");
  Serial.println();
  Serial.println("Az convention: 0=N, 90=E, increasing clockwise. Alt: 0..90.");
  Serial.println("XOFF: CW degrees from North to mount +X axis (persisted).");
  Serial.println("Steps: 10 per degree; X/Y range 0..180.");
  Serial.println();
}

static bool parseAzAlt(const String& s, float& az, float& alt) {
  int comma = s.indexOf(',');
  if (comma < 0) return false;
  String a = s.substring(0, comma);
  String b = s.substring(comma + 1);
  a.trim(); b.trim();
  az = a.toFloat();
  alt = b.toFloat();
  return true;
}

static bool isPrefixNoCase(const String& s, const char* prefix) {
  String p(prefix);
  if (s.length() < p.length()) return false;
  String head = s.substring(0, p.length());
  head.toUpperCase();
  p.toUpperCase();
  return head == p;
}

static String upperTrim(String s) {
  s.trim();
  s.toUpperCase();
  return s;
}

// --------------------------- SETUP / LOOP ---------------------------
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Limit pins are inputs (no homing logic yet)
  pinMode(limitPinX, INPUT_PULLUP);
  pinMode(limitPinY, INPUT_PULLUP);

  engine.init();

  stepperX = engine.stepperConnectToPin(stepPinStepperX);
  stepperY = engine.stepperConnectToPin(stepPinStepperY);

  if (!stepperX || !stepperY) {
    Serial.println("ERROR: Could not init steppers. Check step pins and engine init.");
    return;
  }

  // Configure steppers
  stepperX->setDirectionPin(dirPinStepperX);
  stepperX->setEnablePin(enablePinStepperX);
  stepperX->setAutoEnable(true);
  stepperX->setSpeedInUs(SPEED_US_PER_STEP);
  stepperX->setAcceleration(ACCEL_STEPS_PER_S2);

  stepperY->setDirectionPin(dirPinStepperY);
  stepperY->setEnablePin(enablePinStepperY);
  stepperY->setAutoEnable(true);
  stepperY->setSpeedInUs(SPEED_US_PER_STEP);
  stepperY->setAcceleration(ACCEL_STEPS_PER_S2);

  // Load persistent XOFF
  prefs.begin(NVS_NAMESPACE, false);
  g_xoff_deg = prefs.getFloat(NVS_KEY_XOFF, 0.0f);
  prefs.end();

  Serial.println("X-Y mount controller ready (ESP32-WROOM).");
  Serial.print("Current XOFF (CW from North) = ");
  Serial.println(g_xoff_deg, 3);
  printHelp();
}

void loop() {
  if (Serial.available() <= 0) return;

  String input = Serial.readStringUntil('\n');
  input.trim();
  if (input.length() == 0) return;

  String cmd = upperTrim(input);

  // HELP
  if (cmd == "HELP") { printHelp(); return; }

  // POS?
  if (cmd == "POS?") {
    if (isnan(g_last_x_deg) || isnan(g_last_y_deg)) {
      Serial.println("No position commanded yet.");
    } else {
      Serial.print("Last X=");
      Serial.print(g_last_x_deg, 3);
      Serial.print(" (");
      Serial.print(g_last_x_steps);
      Serial.print(" steps), Y=");
      Serial.print(g_last_y_deg, 3);
      Serial.print(" (");
      Serial.print(g_last_y_steps);
      Serial.println(" steps)");
    }
    return;
  }

  // XOFF?
  if (cmd == "XOFF?") {
    Serial.print("XOFF=");
    Serial.println(g_xoff_deg, 3);
    return;
  }

  // XOFF=...
  if (isPrefixNoCase(cmd, "XOFF=")) {
    String val = cmd.substring(5);
    val.trim();
    float xoff = wrap360(val.toFloat());
    g_xoff_deg = xoff;

    prefs.begin(NVS_NAMESPACE, false);
    prefs.putFloat(NVS_KEY_XOFF, g_xoff_deg);
    prefs.end();

    Serial.print("Saved XOFF=");
    Serial.println(g_xoff_deg, 3);
    return;
  }

  // Maintenance: direct positions
  if (cmd == "HOME" || cmd == "ZENITH" || cmd == "MOTOR" || cmd == "ACCESS") {
    moveXYDegrees(90.0f, 90.0f);
    return;
  }

  // Maintenance: horizon cardinals (earth-frame)
  if (cmd == "N")  { moveToHorizonAz(0.0f);   return; }
  if (cmd == "NE") { moveToHorizonAz(45.0f);  return; }
  if (cmd == "E")  { moveToHorizonAz(90.0f);  return; }
  if (cmd == "SE") { moveToHorizonAz(135.0f); return; }
  if (cmd == "S")  { moveToHorizonAz(180.0f); return; }
  if (cmd == "SW") { moveToHorizonAz(225.0f); return; }
  if (cmd == "W")  { moveToHorizonAz(270.0f); return; }
  if (cmd == "NW") { moveToHorizonAz(315.0f); return; }

  // Otherwise: expect "AZ,ALT"
  float az = 0.0f, alt = 0.0f;
  if (!parseAzAlt(input, az, alt)) {
    Serial.println("Invalid input. Use AZ,ALT or maintenance commands. Type HELP.");
    return;
  }

  alt = clampf(alt, ALT_MIN_DEG, ALT_MAX_DEG);

  float x_deg = 90.0f, y_deg = 90.0f;
  altAzToXY(alt, az, g_xoff_deg, x_deg, y_deg);

  Serial.print("AZ=");
  Serial.print(az, 3);
  Serial.print(" ALT=");
  Serial.print(alt, 3);
  Serial.print(" -> X=");
  Serial.print(x_deg, 3);
  Serial.print(" Y=");
  Serial.println(y_deg, 3);

  moveXYDegrees(x_deg, y_deg);
}
