#pragma once
#include "AudioNode.h"

namespace MittelVec {

struct EnvConfig {
  float attack;
  float decay;
  float sustain;
  float release;
};

class Envelope : public AudioNode {
public:
  explicit Envelope(const AudioContext& context, const EnvConfig& config = { 0.01f, 0.1f, 0.8f, 0.2f }); // Revisit these defaults values.

  enum State { Idle, Attack, Decay, Sustain, Release };

  // Revisit when you need dynamic, run time param changes.
  // Will have to reconfigure to atomics anyway.
  // void setAttack(float attack);
  // void setDecay(float decay);
  // void setSustain(float sustain);
  // void setRelease(float release);

  float getNextLevel();
  
  void noteOn();
  void noteOff();
  void reset();
  bool const isActive();

  void virtual process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) override;

  void applyToBuffer(AudioBuffer& buffer);

private:
  State state;
  float attack, decay, sustain, release;
  float sampleRate;
  float currentLevel;
  bool skipSustain = false; // hard coding this for now, may eventually find use case for noteOff/sustains.
};

} // namespace