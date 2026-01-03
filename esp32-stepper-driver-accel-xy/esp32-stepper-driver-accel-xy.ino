/*
  ESP32 Alt/Az -> X-Y Mount Controller (FastAccelStepper) with Maintenance Positions

  Serial commands:
    AZ,ALT            e.g. 110,25
    XOFF=NNN          e.g. XOFF=30   (CW deg from North to +X axis; saved)
    XOFF?             print current XOFF
    HOME              move to zenith (X=90, Y=90)  (alias: ZENITH)
    MOTOR             move to motor-access position (X=90, Y=90)  (alias: ACCESS)
    N / S / E / W     move to horizon cardinal points (X-Y geometry)
    NE / NW / SE / SW move to horizon intercardinal points
    POS?              print current last commanded X/Y in degrees and steps
    HELP              help text

  Notes:
    - Cardinal moves are implemented by commanding Alt=0 and Az=direction, then converting with your current XOFF.
      This makes them consistent with the same geometry used for normal tracking.
    - If you want cardinal positions to ignore XOFF (i.e., be "mount-frame" not "earth-frame"), tell me and I will
      add a flag or separate commands (e.g., MN/MX).
*/

#include <Arduino.h>
#include "FastAccelStepper.h"
#include <Preferences.h>
#include <math.h>

// --------------------------- PINS (EDIT THESE) ---------------------------
#define dirPinStepperX        14
#define dirPinStepperXLow     13
#define enablePinStepperX     12
#define enablePinStepperXLow  11
#define stepPinStepperX       10
#define stepPinStepperXLow     9
#define limitPinX             46
#define limitPinXLow           3

#define dirPinStepperY         4
#define dirPinStepperYLow      5
#define enablePinStepperY      6
#define enablePinStepperYLow   7
#define stepPinStepperY       15
#define stepPinStepperYLow    16
#define limitPinY             17
#define limitPinYLow          18

#define LED_PIN 38

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

static void altAzToENU(float alt_deg, float az_deg, float& e, float& n, float& u) {
  float alt = deg2rad(alt_deg);
  float az  = deg2rad(wrap360(az_deg));
  e = cosf(alt) * sinf(az);
  n = cosf(alt) * cosf(az);
  u = sinf(alt);
}

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

  // Convert world frame -> mount frame by rotating CCW by xoff (i.e. CW by -xoff)
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
}

static void moveToCardinalAz(float az_deg) {
  // Horizon (Alt=0) at requested azimuth, using same conversion and current XOFF
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
  Serial.println("  HOME | ZENITH      -> X=90, Y=90");
  Serial.println("  MOTOR | ACCESS     -> X=90, Y=90");
  Serial.println("  N S E W            -> horizon cardinal points");
  Serial.println("  NE NW SE SW        -> horizon intercardinal points");
  Serial.println("  HELP               -> this help");
  Serial.println();
  Serial.println("Az convention: 0=N, 90=E, increasing clockwise. Alt: 0..90.");
  Serial.println("XOFF: CW degrees from North to mount +X axis.");
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

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Low pins (unchanged pattern from your original code)
  pinMode(dirPinStepperXLow, OUTPUT);
  pinMode(enablePinStepperXLow, OUTPUT);
  pinMode(stepPinStepperXLow, OUTPUT);
  pinMode(limitPinXLow, OUTPUT);

  digitalWrite(dirPinStepperXLow, LOW);
  digitalWrite(enablePinStepperXLow, HIGH);
  digitalWrite(stepPinStepperXLow, LOW);
  digitalWrite(limitPinXLow, LOW);

  pinMode(dirPinStepperYLow, OUTPUT);
  pinMode(enablePinStepperYLow, OUTPUT);
  pinMode(stepPinStepperYLow, OUTPUT);
  pinMode(limitPinYLow, OUTPUT);

  digitalWrite(dirPinStepperYLow, LOW);
  digitalWrite(enablePinStepperYLow, HIGH);
  digitalWrite(stepPinStepperYLow, LOW);
  digitalWrite(limitPinYLow, LOW);

  // Load XOFF
  prefs.begin(NVS_NAMESPACE, false);
  g_xoff_deg = prefs.getFloat(NVS_KEY_XOFF, 0.0f);
  prefs.end();

  engine.init();

  stepperX = engine.stepperConnectToPin(stepPinStepperX);
  stepperY = engine.stepperConnectToPin(stepPinStepperY);

  if (stepperX && stepperY) {
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

    Serial.println("X-Y mount controller ready.");
    Serial.print("Current XOFF (CW from North) = ");
    Serial.println(g_xoff_deg, 3);
    printHelp();
  } else {
    Serial.println("ERROR: Could not init steppers. Check pins and engine init.");
  }
}

void loop() {
  if (Serial.available() <= 0) return;

  String input = Serial.readStringUntil('\n');
  input.trim();
  if (input.length() == 0) return;

  String cmd = upperTrim(input);

  // HELP
  if (cmd == "HELP") {
    printHelp();
    return;
  }

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

  // Maintenance aliases: HOME/ZENITH/MOTOR/ACCESS -> 90,90
  if (cmd == "HOME" || cmd == "ZENITH" || cmd == "MOTOR" || cmd == "ACCESS") {
    moveXYDegrees(90.0f, 90.0f);
    return;
  }

  // Cardinal / intercardinal horizon points (earth-frame)
  if (cmd == "N")  { moveToCardinalAz(0.0f);   return; }
  if (cmd == "NE") { moveToCardinalAz(45.0f);  return; }
  if (cmd == "E")  { moveToCardinalAz(90.0f);  return; }
  if (cmd == "SE") { moveToCardinalAz(135.0f); return; }
  if (cmd == "S")  { moveToCardinalAz(180.0f); return; }
  if (cmd == "SW") { moveToCardinalAz(225.0f); return; }
  if (cmd == "W")  { moveToCardinalAz(270.0f); return; }
  if (cmd == "NW") { moveToCardinalAz(315.0f); return; }

  // Otherwise expect "AZ,ALT"
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
