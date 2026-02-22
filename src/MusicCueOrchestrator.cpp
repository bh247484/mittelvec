#include "../include/MusicCueOrchestrator.h"
#include <stdexcept>

namespace MittelVec {

MusicCueOrchestrator::MusicCueOrchestrator(AudioGraph& graph, std::vector<MusicCue> cues, std::string samplesDir)
  : graph(graph) {
  
  auto [outputNodeId, outputNodePtr] = graph.addNode<Gain>(1.0f);

  for (const auto& item : cues) {
    if (samplers.find(item.slug) != samplers.end()) {
      throw std::runtime_error("Duplicate music cue slug: " + item.slug);
    }

    if (item.slug.empty()) {
      throw std::runtime_error("Cue slug name cannot be empty string.");
    }

    auto [samplerNodeId, samplerNodePtr] = graph.addNode<Sampler>(
      samplesDir + item.fileName,
      1, // polyphony
      item.loop,
      item.gain,
      0, // pitchShift
      EnvConfig { 0.1f, 0.1f, 1.0f, 0.5f } // attack, decay, sustain, release
    );
    
    samplers[item.slug] = samplerNodePtr;
    graph.connect(samplerNodeId, outputNodeId);
  }
}

void MusicCueOrchestrator::playCue(const std::string& slug) {
  if (slug == currentCueSlug) {
    return;
  }

  if (samplers.count(slug)) {
    if (!currentCueSlug.empty()) {
      stopCue();
    }
    
    samplers[slug]->noteOn();
    currentCueSlug = slug;
  }
}

void MusicCueOrchestrator::stopCue() {
  if (!currentCueSlug.empty() && samplers.count(currentCueSlug)) {
    samplers[currentCueSlug]->noteOff();
    currentCueSlug = "";
  }
}

} // namespace MittelVec