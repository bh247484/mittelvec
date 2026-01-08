#pragma once
#include <vector>
#include "AudioNode.h" // Assuming this defines AudioContext and AudioBuffer

namespace MittelVec {

enum class FilterMode { Lowpass, Highpass, Bandpass, Notch };

struct FilterConfig {
  FilterMode mode = FilterMode::Lowpass;
  float cutoff = 1000.0f;
  float resonance = 0.707f; // Q
};

class Filter : public AudioNode {
public:
  Filter(const AudioContext& context, const FilterConfig& config);

  void setParams(float cutoff, float resonance);
  void setMode(FilterMode mode);
  
  void process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) override;
  void applyToBuffer(AudioBuffer& buffer);

private:
  void calculateCoefficients();

  FilterMode mode;
  float cutoff;
  float resonance;
  float sampleRate;

  // Coefficients
  double b0 = 0, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
  
  // State memory (Z-delay lines)
  double z1_x = 0, z2_x = 0, z1_y = 0, z2_y = 0;
};

} // namespace MittelVec
