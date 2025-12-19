#include "../include/PitchShift.h"
#include <cmath>

namespace MittelVec {

PitchShift::PitchShift(const AudioContext& context, int semitoneShift)
  : AudioNode(context), ratio(convertSemitoneToRatio(semitoneShift)) {
    // Preallocate ringBuffer.
    ringBuffer.resize(context.bufferSize * context.numChannels);
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
  const int ringSize = ringBuffer.size();

  // Write sample into ringBuffer.
  for (int i = 0; i < numInputSamples; ++i) {
    ringBuffer[ringWriteIdx] = buffer[i];
    ringWriteIdx = (ringWriteIdx + 1) % ringSize;
  }

  for (int i = 0; i < numInputSamples; ++i) {
    int index0 = static_cast<int>(samplePosition);
    int index1 = (index0 + 1) % ringSize;
    float fraction = static_cast<float>(samplePosition - (double) index0);

    float sampleA = ringBuffer[index0];
    float sampleB = ringBuffer[index1];

    // Write the result back into the original buffer
    buffer[i] = lerp(sampleA, sampleB, fraction);

    // Increment phase by the pitch ratio.
    samplePosition += ratio;

    // Wrap the read head around the ring buffer
    if (samplePosition >= static_cast<double>(ringSize)) {
      samplePosition -= static_cast<double>(ringSize);
    }
  }
}

void PitchShift::setPitch(int semitoneShift) {
  ratio = convertSemitoneToRatio(semitoneShift);
}

void PitchShift::resetSamplePosition() {
  samplePosition = 0.0;
  ringWriteIdx = 0;
}

float PitchShift::lerp(float a, float b, float fraction) {
  return a + (b - a) * fraction;
}

// sm1 = s-1
float PitchShift::cubicInterpolation(float sm1, float s0, float s1, float s2, float fraction) {
  float a = -0.5 * sm1 + 1.5 * s0 - 1.5 * s1 + 0.5 * s2;
  float b = sm1 - 2.5 * s0 + 2.0 * s1 - 0.5 * s2;
  float c = -0.5 * sm1 + 0.5 * s1;
  float d = s0;
  
  // confirm this is correct...
  float x = sm1 - s0;

  // Expanded version of the cubic polynomial: ax^3 + bx^2 + cx + d
  float cubic = d + x * (c + x * (b + x * a));
  
  // Confirm this is correct use of `fraction`.
  return cubic * fraction;
}

// void Gain::sincInterpolation(const float* inputData, size_t numInputSamples, double position, int numTaps) {
  
// }

double PitchShift::convertSemitoneToRatio(int semitoneShift) {
  const double exponent = static_cast<double>(semitoneShift) / 12.0;
    
  // The base is 2 (for an octave)
  return std::pow(2.0, exponent);
}

} // namespace
