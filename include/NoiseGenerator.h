#pragma once
#include "AudioNode.h"
#include <random>

namespace Middleman {
    
class NoiseGenerator : public AudioNode {
    public:
    NoiseGenerator(const AudioContext& context);
    
    void process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) override;
    
    private:
    std::random_device randomDevice;
    std::mt19937 randomGen;
    std::uniform_real_distribution<float> dist;
};
    
} // namespace
