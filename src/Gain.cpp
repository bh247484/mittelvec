#include "../include/Gain.h"

namespace MittelVec {

Gain::Gain(const AudioContext& context, float gain)
  : AudioNode(context), gain(gain) {}

void Gain::setGain(float newGain) {
  this->gain = newGain;
}

void Gain::process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) {
  outputBuffer.clear();

  if (inputs.empty()) {
    return;
  }

  // Sum all input buffers into the output buffer
  for (const auto* inputBufferPtr : inputs) {
    for (int i = 0; i < outputBuffer.size(); ++i) {
      outputBuffer[i] += (*inputBufferPtr)[i];
    }
  }

  // Apply gain to the summed signal
  for (int i = 0; i < outputBuffer.size(); ++i) {
    outputBuffer[i] *= gain;
  }
}

} // namespace
