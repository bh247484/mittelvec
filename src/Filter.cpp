#include "../include/Filter.h"
#include <cmath>

// C++ 17 doesn't have PI constant.
// We can use arccosine of -1.0 to get pi instead.
// When x == -1.0 on the unit circle, theta == pi.
const double PI = std::acos(-1.0);

namespace MittelVec {

Filter::Filter(const AudioContext& context, const FilterConfig& config)
  : AudioNode(context), 
    mode(config.mode), 
    cutoff(config.cutoff), 
    resonance(config.resonance), 
    sampleRate(static_cast<float>(context.sampleRate)) 
{
  calculateCoefficients();
}

void Filter::calculateCoefficients() {
  double w0 = 2.0 * PI * cutoff / sampleRate;
  double alpha = std::sin(w0) / (2.0 * resonance);
  double cosW0 = std::cos(w0);

  double a0 = 0;

  switch (mode) {
    case FilterMode::Lowpass:
      b0 = (1.0 - cosW0) / 2.0;
      b1 = 1.0 - cosW0;
      b2 = (1.0 - cosW0) / 2.0;
      a0 = 1.0 + alpha;
      a1 = -2.0 * cosW0;
      a2 = 1.0 - alpha;
      break;
    case FilterMode::Highpass:
      b0 = (1.0 + cosW0) / 2.0;
      b1 = -(1.0 + cosW0);
      b2 = (1.0 + cosW0) / 2.0;
      a0 = 1.0 + alpha;
      a1 = -2.0 * cosW0;
      a2 = 1.0 - alpha;
      break;
    case FilterMode::Bandpass:
      b0 = alpha;
      b1 = 0;
      b2 = -alpha;
      a0 = 1.0 + alpha;
      a1 = -2.0 * cosW0;
      a2 = 1.0 - alpha;
      break;
    case FilterMode::Notch:
      b0 = 1.0;
      b1 = -2.0 * cosW0;
      b2 = 1.0;
      a0 = 1.0 + alpha;
      a1 = -2.0 * cosW0;
      a2 = 1.0 - alpha;
      break;
  }

  // Normalize all coefficients by a0
  b0 /= a0; b1 /= a0; b2 /= a0;
  a1 /= a0; a2 /= a0;

  // Debug...
  // printf("Filter: F=%f, Q=%f, SR=%f | b0=%f, a1=%f\n", cutoff, resonance, sampleRate, b0, a1);
}

void Filter::setParams(float newCutoff, float newResonance) {
  cutoff = newCutoff;
  resonance = newResonance;
  calculateCoefficients();
}

void Filter::setMode(FilterMode newMode) {
  mode = newMode;
  calculateCoefficients();
}

void Filter::process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) {
  if (inputs.empty()) return;
  outputBuffer.clear();

  // Sum inputs
  for (const auto* inputBufferPtr : inputs) {
    for (int i = 0; i < outputBuffer.size(); ++i) {
      outputBuffer[i] += (*inputBufferPtr)[i];
    }
  }

  applyToBuffer(outputBuffer);
}

void Filter::applyToBuffer(AudioBuffer& buffer) {
  for (int i = 0; i < buffer.size(); ++i) {
    double x = buffer[i];
    
    // Difference Equation (Direct Form I)
    double y = (b0 * x) + (b1 * z1_x) + (b2 * z2_x) - (a1 * z1_y) - (a2 * z2_y);

    // Update state
    z2_x = z1_x;
    z1_x = x;
    z2_y = z1_y;
    z1_y = y;

    buffer[i] = static_cast<float>(y);
  }
}

} // namespace MittelVec
