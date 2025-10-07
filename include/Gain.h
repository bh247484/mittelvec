#pragma once
#include "AudioNode.h"

namespace MittelVec {

class Gain : public AudioNode {
public:
    explicit Gain(const AudioContext& context, float gain);

    void setGain(float gain);
    void virtual process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) override;

private:
    float gain;
};

} // namespace
