#include "../include/MusicCueOrchestrator.h"

namespace MittelVec {

MusicCueOrchestrator::MusicCueOrchestrator(AudioGraph& graph, std::vector<MusicCueItem> cues, std::string samplesDir)
  : graph(graph) {
  
  auto [outputNodeId, outputNodePtr] = graph.addNode<Gain>(1.0f);

  for (const auto& item : cues) {
    auto [samplerNodeId, samplerNodePtr] = graph.addNode<Sampler>(
      samplesDir + item.fileName,
      1, // polyphony
      item.loop,
      item.gain
    );
    
    samplers[item.slug] = samplerNodePtr;
    graph.connect(samplerNodeId, outputNodeId);
  }
}

void MusicCueOrchestrator::playCue(const std::string& slug) {
  if (samplers.count(slug)) {
    samplers[slug]->noteOn();
  }
}

void MusicCueOrchestrator::stopCue(const std::string& slug) {
  if (samplers.count(slug)) {
    samplers[slug]->noteOff();
  }
}

} // namespace MittelVec