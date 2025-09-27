#pragma once
#include "AudioContext.h"
#include <vector>
#include <cassert>
#include <algorithm>

class AudioBuffer {
public:
  AudioBuffer(const AudioContext& context);
  ~AudioBuffer();

  // Accessors
  float* channelData(int ch);
  const float* channelData(int ch) const;

  int getNumChannels() const;
  int getNumFrames() const;
  float getSampleRate() const;
  int size() const;

  void clear();
  AudioBuffer& operator+=(const AudioBuffer& other);
  AudioBuffer operator+(const AudioBuffer& other) const;

  float& operator[](int index);
  const float& operator[](int index) const;

private:
  int channels;
  int frames;
  float sampleRate;
  std::vector<float> data;
};
