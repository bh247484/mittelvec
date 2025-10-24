#pragma once
#include "./AudioGraph.h"
#include "./Gain.h"
#include "./Sampler.h"

namespace MittelVec {

struct SamplePackItem {
  std::string slug;
  std::string fileName;
  int polyphony;
  bool loop;
  float gain;
  EnvConfig envConfig;
  // FilterConfig filterConfig;

  // Constructor enforces required fields and default value for polyphony.
  SamplePackItem(
    std::string slug,
    std::string fileName,
    int polyphony = 1,
    bool loop = false,
    float gain = 1.0
  ) : slug(slug), fileName(fileName), polyphony(polyphony), loop(loop), gain(gain) {}
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
