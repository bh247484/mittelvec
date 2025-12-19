#pragma once
#include "AudioNode.h"

namespace MittelVec {

class PitchShift : public AudioNode {
public:
  explicit PitchShift(const AudioContext& context, int semitoneShift);

  void virtual process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) override;
  void applyToBuffer(AudioBuffer& buffer);
  void shiftInto(const AudioBuffer& source, AudioBuffer& target, int& playhead);
  void setPitch(int semitoneShift);
  void resetSamplePosition();

private:
  double samplePosition = 0.0;
  double ratio;
  std::vector<float> ringBuffer;
  int ringWriteIdx;

  float lerp(float a, float b, float fraction);
  float cubicInterpolation(float sm1, float s0, float s1, float s2, float fraction);
  // float sincInterpolation(const float* inputData, size_t numInputSamples, double position, int numTaps);
  double convertSemitoneToRatio(int semitoneShift);
};

} // namespace
