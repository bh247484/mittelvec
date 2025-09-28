#pragma once
#include "AudioNode.h"

namespace Middleman {

class Gain : public AudioNode {
public:
    explicit Gain(float gain, const AudioContext& context);

    void setGain(float gain);
    void virtual process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) override;

private:
    float gain;
};

} // namespace
