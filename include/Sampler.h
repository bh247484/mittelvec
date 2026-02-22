#pragma once
#include <list>
#include <optional>
#include "AudioNode.h"
#include "Envelope.h"
#include "PitchShift.h"
#include "Filter.h"

namespace MittelVec {

// Consider making SamplerVoice its own class..
struct SamplerVoice {
  int playheadIndex = 0;
  bool active = false;

  std::unique_ptr<Envelope> envelope;
  std::unique_ptr<PitchShift> pitchShifter;
  std::unique_ptr<Filter> filter;

  AudioBuffer voiceBuffer;

  SamplerVoice(const AudioContext& context)
    : voiceBuffer(context),
    envelope(std::make_unique<Envelope>(context)),
    pitchShifter(std::make_unique<PitchShift>(context, 0)),
    filter(std::make_unique<Filter>(context, FilterConfig { FilterMode::Lowpass, 500.0f, 0.5f }))
  {}

  void trigger() {
    playheadIndex = 0;
    active = true;
    envelope->noteOn();
    pitchShifter->reset();
  }

  void processVoice(
    const AudioBuffer& sample,
    AudioBuffer& outputBuffer,
    bool loop,
    float gain,
    int pitchShift,
    std::optional<EnvConfig> envConfig,
    std::optional<FilterConfig> filterConfig
  ) {
    if (!active) return;
    
    voiceBuffer.clear();
    for (int i = 0; i < voiceBuffer.size(); ++i) {
      // If voice has reached the end of the sample.  
      if (playheadIndex >= sample.size()) {
        if (loop) {
          playheadIndex = 0;
          if (envConfig.has_value()) envelope->noteOn();
        } else {
          if (envConfig.has_value()) envelope->reset();
          active = false;
          playheadIndex = 0;
          break;
        }
      }

      // Write sample data into voiceBuffer.
      voiceBuffer[i] = sample[playheadIndex++] * gain;
    }

    // Apply per-voice DSP
    if (pitchShift && pitchShift != 0) {
      // This are probably being reset redundantly across processVoice calls.
      // Should cache the values or something.
      pitchShifter->setPitch(pitchShift);
      pitchShifter->applyToBuffer(voiceBuffer);
    }

    if (envConfig.has_value()) {
      envelope->applyToBuffer(voiceBuffer);
      if (!envelope->isActive()) {
        active = false;
        playheadIndex = 0;
      }
    }

    if (filterConfig.has_value()) {
      filter->setMode(filterConfig->mode);
      filter->setParams(filterConfig->cutoff, filterConfig->resonance);
      filter->applyToBuffer(voiceBuffer);
    }

    // Sum into main output buffer
    outputBuffer += voiceBuffer;
  }
};
    
class Sampler : public AudioNode {
  public:
  Sampler(
    const AudioContext& context,
    std::string samplePath,
    int polyphony,
    bool loop = false,
    float gain = 1.0f,
    int pitchShift = 0,
    std::optional<EnvConfig> envConfig = std::nullopt,
    std::optional<FilterConfig> filterConfig = std::nullopt
  );

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
  int pitchShift;
  std::optional<EnvConfig> envConfig;
  std::optional<FilterConfig> filterConfig;
};
    
} // namespace
