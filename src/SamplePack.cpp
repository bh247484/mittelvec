#include "../include/SamplePack.h"

namespace MittelVec {

SamplePack::SamplePack(AudioGraph& graph, std::vector<SamplePackItem> samplePackItems, float gain)
  : graph(graph) {
    auto [outputNodeId, outputNodePtr] = graph.addNode<Gain>(gain); // consider parameterizing gain

    for (auto& item : samplePackItems) {
      auto [samplerNodeId, samplerNodePtr] = graph.addNode<Sampler>(item.samplePath, item.polyphony, item.loop, item.gain);
      samplers[item.slug] = samplerNodePtr;
      graph.connect(samplerNodeId, outputNodeId);
    }
  }

void SamplePack::triggerSample(std::string slug) {
  samplers[slug]->noteOn();
}

} // namespace