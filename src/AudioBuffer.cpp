#include "../include/AudioBuffer.h"
#include "../include/AudioGraph.h"

namespace MittelVec {

AudioBuffer::AudioBuffer(const AudioContext& context)
: channels(context.numChannels), frames(context.bufferSize), sampleRate(context.sampleRate)
{
  data.resize(static_cast<int>(channels) * frames, 0.0f);
}

AudioBuffer::~AudioBuffer() = default;
    
int AudioBuffer::getNumChannels() const { return channels; }
int AudioBuffer::getNumFrames() const { return frames; }
float AudioBuffer::getSampleRate() const { return sampleRate; }
int AudioBuffer::size() const { return data.size(); }

// Clear buffer
void AudioBuffer::clear() { std::fill(data.begin(), data.end(), 0.0f); }

// Operator overloads
AudioBuffer& AudioBuffer::operator+=(const AudioBuffer& other) {
  assert(channels == other.channels && frames == other.frames);
  for (int i = 0; i < data.size(); ++i) {
    data[i] += other.data[i];
  }
  return *this;
}
    
AudioBuffer AudioBuffer::operator+(const AudioBuffer& other) const {
  AudioBuffer result(*this);
  result += other;
  return result;
}

float& AudioBuffer::operator[](int index) {
  return data[index];
}

const float& AudioBuffer::operator[](int index) const {
  return data[index];
}

} // namespace
