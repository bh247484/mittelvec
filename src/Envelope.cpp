#include "../include/Envelope.h"

namespace MittelVec {

Envelope::Envelope(const AudioContext& context, float attack, float decay, float sustain, float release)
  : AudioNode(context), attack(attack), decay(decay), sustain(sustain), release(release), sampleRate(context.sampleRate) {}

float Envelope::getNextLevel() {
  switch (state) {
  case Idle:
    break;
  case Attack:
    currentLevel += 1.0 / (attack * sampleRate);

    // Clamp level and transition to decay.
    if (currentLevel >= 1.0) {
      currentLevel = 1.0;
      state = Decay;
    }
    break;
  case Decay:
    currentLevel -= (1.0 - sustain) / (decay * sampleRate);
    
    if (currentLevel <= sustain) {
      currentLevel = sustain;
      state = Sustain;
    }
    break;
  case Sustain:
    if (skipSustain) {
      state = Release;
    }
    break;
  case Release:
    currentLevel -= sustain / (release * sampleRate);
    
    if (currentLevel <= 0.0) {
      currentLevel = 0.0;
      state = Idle;
    }
    break;
  default:
    break;
  }

  return currentLevel;
}

void Envelope::noteOn() {
  state = Attack;
  currentLevel = 0; // confirm this...
}

void Envelope::noteOff() {
  state = Release;
}

void Envelope::reset() {
  currentLevel = 0;
  state = Idle;
}

bool const Envelope::isActive() {
  return state != Idle;
}

void Envelope::process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) {
  if (inputs.empty()) return;
  outputBuffer.clear();

  // Sum all input buffers into the output buffer
  for (const auto* inputBufferPtr : inputs) {
    for (int i = 0; i < outputBuffer.size(); ++i) {
      outputBuffer[i] += (*inputBufferPtr)[i];
    }
  }

  // Apply envelope to the summed signal
  for (int i = 0; i < outputBuffer.size(); ++i) {
    outputBuffer[i] *= getNextLevel();
  }
}

/**
 * Applies envelope directly to incoming buffer.
 * Allows for inline processing as opposed to more modular node/graph style `process` method approach.
 */
void Envelope::applyToBuffer(AudioBuffer& buffer) {
  for (int i = 0; i < buffer.size(); ++i) {
    buffer[i] *= getNextLevel();
  }
}

} // namespace MittelVec