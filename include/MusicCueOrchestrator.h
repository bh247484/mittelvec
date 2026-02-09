#pragma once
#include "AudioGraph.h"
#include "Sampler.h"
#include "Gain.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace MittelVec {

struct MusicCueItem {
  std::string slug;
  std::string fileName;
  bool loop;
  float gain;

  MusicCueItem(
    std::string slug,
    std::string fileName,
    bool loop = true,
    float gain = 1.0f
  ) : slug(slug), fileName(fileName), loop(loop), gain(gain) {}
};

class MusicCueOrchestrator {
public:
  MusicCueOrchestrator(AudioGraph& graph, std::vector<MusicCueItem> cues, std::string samplesDir);

  void playCue(const std::string& slug);
  void stopCue(const std::string& slug);

private:
  AudioGraph& graph;
  std::unordered_map<std::string, Sampler*> samplers;
};

} // namespace MittelVec