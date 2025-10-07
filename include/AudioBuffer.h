#pragma once
#include "AudioContext.h"
#include <vector>
#include <cassert>
#include <algorithm>

namespace MittelVec {

class AudioBuffer {
public:
  AudioBuffer(const AudioContext& context);
  ~AudioBuffer();

  int getNumChannels() const;
  int getNumFrames() const;
  float getSampleRate() const;
  int size() const;
  void setAudioContext(AudioContext newContext);

  void clear();
  AudioBuffer& operator+=(const AudioBuffer& other);
  AudioBuffer operator+(const AudioBuffer& other) const;

  float& operator[](int index);
  const float& operator[](int index) const;

  // Made public so the vector can be accessed directly.
  std::vector<float> data;

private:
  int channels;
  int frames;
  float sampleRate;
};

} // namespace
