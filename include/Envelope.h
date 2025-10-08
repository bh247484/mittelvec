#pragma once
#include "AudioNode.h"

namespace MittelVec {

class Envelope : public AudioNode {
public:
  explicit Envelope(const AudioContext& context, float attack, float decay, float sustain, float release);

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
  bool skipSustain = true; // hard coding this for now, may eventually find use case for noteOff/sustains.
};

} // namespace