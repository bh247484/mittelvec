#include "../include/SamplePack.h"

namespace MittelVec {

SamplePack::SamplePack(AudioGraph& graph, std::vector<SamplePackItem> samplePackItems, std::string samplesDir, float gain)
  : graph(graph) {
    auto [outputNodeId, outputNodePtr] = graph.addNode<Gain>(gain); // consider parameterizing gain

    for (auto& item : samplePackItems) {
      auto [samplerNodeId, samplerNodePtr] = graph.addNode<Sampler>(
        samplesDir + item.fileName,
        item.polyphony,
        item.loop,
        item.gain,
        item.pitchShift,
        item.envConfig,
        item.filterConfig
      );
      samplers[item.slug] = samplerNodePtr;
      graph.connect(samplerNodeId, outputNodeId);
    }
  }

void SamplePack::triggerSample(std::string slug) {
  samplers[slug]->noteOn();
}

} // namespace