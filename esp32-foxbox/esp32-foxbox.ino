#include "GoertzelESP32.h"

const int samples = 200; // Number of samples per tone
const float targetFreqs[2] = {697, 1209}; // Row and column frequencies for the "1" key
const char dtmfCode = '1';

const int audioPin = 36; // Connect the audio input to GPIO 36 (ADC1_0)

void setup() {
  Serial.begin(115200);
}

void loop() {
  int row = -1, col = -1;

  for (int i = 0; i < 2; i++) {
    GoertzelESP32 goertzel(samples, targetFreqs[i], 8000);

    for (int j = 0; j < samples; j++) {
      goertzel.sample(analogRead(audioPin));
    }

    float magnitude = goertzel.detect();
    if (magnitude > 1000) { // Threshold for tone detection
      if (i == 0) {
        row = 0;
      } else {
        col = 0;
      }
    }
  }

  if (row == 0 && col == 0) {
    Serial.print("DTMF tone detected: ");
    Serial.println(dtmfCode);
    delay(500); // Avoid multiple detections for a single tone
  }
}
