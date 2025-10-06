#pragma once
#include <list>
#include "AudioNode.h"
#include "Envelope.h"

namespace Middleman {

// Consider making SamplerVoice its own class..
struct SamplerVoice {
  int playheadIndex = 0;
  bool active = false;
  bool loop = false;

  std::unique_ptr<Envelope> envelope;
  // std::unique_ptr<FilterNode> filter;

  AudioBuffer voiceBuffer;

  SamplerVoice(const AudioContext& context)
    : voiceBuffer(context),
    envelope(std::make_unique<Envelope>(context, 0.01f, 0.1f, 0.8f, 0.2f)) // revisit these envelope defaults...
  {}

  void trigger() {
    playheadIndex = 0;
    active = true;
    envelope->noteOn();
  }

  void processVoice(const AudioBuffer& sample, AudioBuffer& outputBuffer) {
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
      voiceBuffer[i] = sample[playheadIndex++];
    }

    // Apply per-voice DSP
    std::vector<const AudioBuffer*> inputs = { &voiceBuffer }; // weird but necessary since AudioNodes expect vector of inputs.
    envelope->process(inputs, voiceBuffer);

    // Sum into main output buffer
    outputBuffer += voiceBuffer;
  }
};
    
class PolyphonicSampler : public AudioNode {
  public:
  PolyphonicSampler(const AudioContext& context, AudioBuffer sample, int polyphony);

  void noteOn();
  void noteOff();
  SamplerVoice* allocateVoice();
  
  void process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) override;
  
  private:
  int polyphony;
  AudioBuffer sample;
  std::vector<SamplerVoice> voices;
  // std::list<int> activeVoices; // indicies per voice of `voices` vector above.
  std::list<SamplerVoice*> activeVoices; // indicies per voice of `voices` vector above.
};
    
} // namespace
