#pragma once
#include <list>
#include "AudioNode.h"
#include "Envelope.h"

namespace MittelVec {

// Consider making SamplerVoice its own class..
struct SamplerVoice {
  int playheadIndex = 0;
  bool active = false;

  std::unique_ptr<Envelope> envelope;
  // std::unique_ptr<FilterNode> filter;

  AudioBuffer voiceBuffer;

  SamplerVoice(const AudioContext& context)
    : voiceBuffer(context),
    envelope(std::make_unique<Envelope>(context))
  {}

  void trigger() {
    playheadIndex = 0;
    active = true;
    envelope->noteOn();
  }

  void processVoice(const AudioBuffer& sample, AudioBuffer& outputBuffer, bool loop, float gain) {
    if (!active) return;
    
    voiceBuffer.clear();
    for (size_t i = 0; i < voiceBuffer.size(); ++i) {
      // If voice has reached the end of the sample.  
      if (playheadIndex >= sample.size()) {
        if (loop) {
          playheadIndex = 0;
          envelope->noteOn();
        } else {
          envelope->reset();
          active = false;
          playheadIndex = 0;
          break;
        }
      }

      // Write sample data into voiceBuffer.
      voiceBuffer[i] = sample[playheadIndex++] * gain;
    }

    // Apply per-voice DSP
    envelope->applyToBuffer(voiceBuffer);

    // Sum into main output buffer
    outputBuffer += voiceBuffer;
  }
};
    
class Sampler : public AudioNode {
  public:
  Sampler(const AudioContext& context, std::string samplePath, int polyphony, bool loop = false, float gain = 1.0f);

  void noteOn();
  void noteOff();
  SamplerVoice* allocateVoice();
  
  void process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) override;
  
  private:
  int polyphony;
  AudioBuffer sample;
  std::vector<SamplerVoice> voices;
  std::list<SamplerVoice*> activeVoices; // indicies per voice of `voices` vector above.
  bool loop;
  float gain;
};
    
} // namespace
