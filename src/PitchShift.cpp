#include "../include/PitchShift.h"
#include <cmath>

namespace MittelVec {

PitchShift::PitchShift(const AudioContext& context, int semitoneShift)
  : AudioNode(context), currentDelay(0.0), ringWriteIdx(0), ratio(convertSemitoneToRatio(semitoneShift)) {
    // Preallocate ringBuffer.
    ringBuffer.resize(context.bufferSize * context.numChannels);

    // Use below if you want custom window size.
    // Set a window size of ~20ms (adjust to taste)
    // int windowSize = static_cast<int>(context.sampleRate * 0.020);
    // ringBuffer.resize(windowSize);
  }

  
void PitchShift::process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) {
  if (inputs.empty()) {
    outputBuffer.clear();
    return;
  }

  const int numInputSamples = inputs[0]->getNumFrames();
  
  // Sum all input buffers into the output buffer
  // Revisit this... Seems wasteful if inputs.length < 1.
  for (const auto* inputBufferPtr : inputs) {
    for (int i = 0; i < outputBuffer.size(); ++i) {
      outputBuffer[i] += (*inputBufferPtr)[i];
    }
  }

  applyToBuffer(outputBuffer);
}

void PitchShift::applyToBuffer(AudioBuffer& buffer) {
  const int numInputSamples = buffer.getNumFrames();
  const double ringSize = static_cast<double>(ringBuffer.size());
  // Not sure this margin is strictly necessary but may mitigate some pops/clicks.
  const double safetyMargin = 20.0;

  for (int i = 0; i < numInputSamples; ++i) {
    // Write sample into ringBuffer
    ringBuffer[ringWriteIdx] = buffer[i];

    // Offset for Dual Tap delay.
    currentDelay += (1.0 - ratio);

    // Wrap currentDelay
    if (currentDelay >= ringSize) currentDelay -= ringSize;
    if (currentDelay < 0) currentDelay += ringSize;

    double tapAPos = static_cast<double>(ringWriteIdx) - (currentDelay + safetyMargin);
    if (tapAPos < 0) tapAPos += ringSize; // wrap

    double tapBPos = tapAPos + (ringSize * 0.5); // Offset by 180 deg
    if (tapBPos >= ringSize) tapBPos -= ringSize; // wrap

    // Triangle window for crossfading between taps.
    // Consider using Hamming window.
    float windowPhase = static_cast<float>(currentDelay / ringSize);
    float gainA = 1.0f - std::abs((windowPhase * 2.0f) - 1.0f);
    float gainB = 1.0f - gainA;

    float tapA = getCubicSample(tapAPos);
    float tapB = getCubicSample(tapBPos);
    buffer[i] = (tapA * gainA) + (tapB * gainB);

    // Increment write index.
    ringWriteIdx = (ringWriteIdx + 1) % static_cast<int>(ringSize);
  }
}

void PitchShift::setPitch(int semitoneShift) {
  ratio = convertSemitoneToRatio(semitoneShift);
}

void PitchShift::reset() {
  // samplePosition = 0.0;
  currentDelay = 0.0;
  ringWriteIdx = 0;
  std::fill(ringBuffer.begin(), ringBuffer.end(), 0.0f);
}

float PitchShift::getLerpSample(double samplePosition) {
  int index0 = static_cast<int>(std::floor(samplePosition));
  int index1 = (index0 + 1) % static_cast<int>(ringBuffer.size());
  float fraction = static_cast<float>(samplePosition - std::floor(samplePosition));

  return lerp(ringBuffer[index0], ringBuffer[index1], fraction);
}

float PitchShift::getCubicSample(double samplePosition) {
  int i = static_cast<int>(std::floor(samplePosition));
  float fraction = static_cast<float>(samplePosition - i);
  int size = static_cast<int>(ringBuffer.size());

  // Get 4 samples for cubic interpolation: i-1, i, i+1, i+2
  float sm1 = ringBuffer[(i - 1 + size) % size];
  float s0  = ringBuffer[i % size];
  float s1  = ringBuffer[(i + 1) % size];
  float s2  = ringBuffer[(i + 2) % size];

  return cubicInterpolation(sm1, s0, s1, s2, fraction);
}

float PitchShift::lerp(float a, float b, float fraction) {
  return a + (b - a) * fraction;
}

// sm1 = s-1
float PitchShift::cubicInterpolation(float sm1, float s0, float s1, float s2, float fraction) {
  float a = -0.5f * sm1 + 1.5f * s0 - 1.5f * s1 + 0.5f * s2;
  float b = sm1 - 2.5f * s0 + 2.0f * s1 - 0.5f * s2;
  float c = -0.5f * sm1 + 0.5f * s1;
  float d = s0;

  return d + fraction * (c + fraction * (b + fraction * a));
}

// void Gain::sincInterpolation(const float* inputData, size_t numInputSamples, double position, int numTaps) {
  
// }

double PitchShift::convertSemitoneToRatio(int semitoneShift) {
  const double exponent = static_cast<double>(semitoneShift) / 12.0;
    
  // The base is 2 (for an octave)
  return std::pow(2.0, exponent);
}

} // namespace
