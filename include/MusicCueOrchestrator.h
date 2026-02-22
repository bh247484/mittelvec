#pragma once
#include "AudioGraph.h"
#include "Sampler.h"
#include "Gain.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace MittelVec {

struct MusicCue {
  std::string slug;
  std::string fileName;
  bool loop;
  float gain;

  MusicCue(
    std::string slug,
    std::string fileName,
    bool loop = true,
    float gain = 1.0f
  ) : slug(slug), fileName(fileName), loop(loop), gain(gain) {}
};

class MusicCueOrchestrator {
public:
  MusicCueOrchestrator(AudioGraph& graph, std::vector<MusicCue> cues, std::string samplesDir);

  void playCue(const std::string& slug);
  void stopCue();

private:
  AudioGraph& graph;
  std::unordered_map<std::string, Sampler*> samplers;
  std::string currentCueSlug;
};

} // namespace MittelVec