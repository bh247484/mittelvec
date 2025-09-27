#include "../include/NoiseGenerator.h"
// #include <random>


NoiseGenerator::NoiseGenerator(const AudioContext& context)
  : AudioNode(context),
    randomGen(randomDevice()),
    dist(-1.0f, 1.0f)
{}

void NoiseGenerator::process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) {
  for (size_t i = 0; i < outputBuffer.size(); ++i) {
    outputBuffer[i] = dist(randomGen);
  }
}
