#include "../include/PolyphonicSampler.h"

namespace MittelVec {

PolyphonicSampler::PolyphonicSampler(const AudioContext &context, AudioBuffer sample, int polyphony)
  : AudioNode(context), sample(sample), polyphony(polyphony)
{
  voices.reserve(polyphony);
  for (int i = 0; i < polyphony; ++i) {
    voices.emplace_back(context);  // constructs SamplerVoice with AudioContext
  }
}

SamplerVoice* PolyphonicSampler::allocateVoice() {
  // Try first inactive voice.
  for (SamplerVoice& voice : voices) {
    if (!voice.active) {
      return &voice;
    }
  }

  // Steal oldest voice when all voices active.
  SamplerVoice* oldestVoice = activeVoices.front();
  activeVoices.pop_front();
  return oldestVoice;
}

void PolyphonicSampler::noteOn() {
  SamplerVoice* freeVoice = allocateVoice();
  freeVoice->trigger();
  activeVoices.push_back(freeVoice);
}

void PolyphonicSampler::process(const std::vector<const AudioBuffer *> &inputs, AudioBuffer &outputBuffer) {
  outputBuffer.clear();

  for (SamplerVoice& voice : voices) {
    if (!voice.active) continue;

    voice.processVoice(sample, outputBuffer);

    if (!voice.active) {
      activeVoices.remove(&voice);
    }
  }

}

}
