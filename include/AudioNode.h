#include "AudioBuffer.h"
#include "AudioContext.h"
#include <vector>
#include <memory>

class AudioNode {
public:
  AudioNode(const AudioContext& context);
  virtual ~AudioNode() = default;

  virtual void process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) = 0;
  
  AudioBuffer outputBuffer;
};
