#ifndef GOERTZELESP32_H
#define GOERTZELESP32_H

#include <math.h>

class GoertzelESP32 {
  public:
    GoertzelESP32(int N, float targetFrequency, int sampleRate)
      : N(N), coeff(2.0 * cos(2.0 * M_PI * targetFrequency / sampleRate)) {}

    void reset() {
      s_prev = 0.0;
      s_prev2 = 0.0;
    }

    void sample(float sample) {
      float s = sample + coeff * s_prev - s_prev2;
      s_prev2 = s_prev;
      s_prev = s;
    }

    float detect() {
      return s_prev2 * s_prev2 + s_prev * s_prev - coeff * s_prev * s_prev2;
    }

  private:
    int N;
    float coeff;
    float s_prev;
    float s_prev2;
};

#endif
