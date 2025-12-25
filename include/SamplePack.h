#pragma once
#include "./AudioGraph.h"
#include "./Gain.h"
#include "./Sampler.h"
#include <optional>

namespace MittelVec {

struct SamplePackItem {
  std::string slug;
  std::string fileName;
  int polyphony;
  bool loop;
  float gain;
  int pitchShift;
  std::optional<EnvConfig> envConfig;

  // Constructor enforces required fields and default value for polyphony.
  SamplePackItem(
    std::string slug,
    std::string fileName,
    int polyphony = 1,
    bool loop = false,
    float gain = 1.0,
    int pitchShift = 0,
    std::optional<EnvConfig> env = std::nullopt
  ) : slug(slug), fileName(fileName), polyphony(polyphony), loop(loop),
      gain(gain), pitchShift(pitchShift), envConfig(env) {}
};

class SamplePack {
public:
  SamplePack(AudioGraph& graph, std::vector<SamplePackItem> samplePackItems, std::string samplesDir, float gain = 1.0);
  void triggerSample(std::string slug);

  std::unordered_map<std::string, Sampler*> samplers;
  std::unique_ptr<Gain> output;

private:
  AudioGraph& graph;
};

} // namespace
