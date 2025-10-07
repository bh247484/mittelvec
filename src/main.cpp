#include <stdio.h>
#include <csignal>
#include <thread>
#include <chrono>
#include <atomic>
#include "../include/Engine.h"
#include "../include/AudioBuffer.h"
#include "../include/AudioGraph.h"
#include "../include/Gain.h"
#include "../include/NoiseGenerator.h"
#include "../include/AudioContext.h"
#include "../include/PolyphonicSampler.h"

const int BUFFER_SIZE = 512;
const int NUM_CHANNELS = 2;
const float SAMPLE_RATE = 44100.0f;

std::atomic<bool> keepRunning(true);

int main() {
  std::signal(SIGINT, [](int) { keepRunning = false; });

  MittelVec::AudioContext globalContext = { BUFFER_SIZE, NUM_CHANNELS, SAMPLE_RATE };
  MittelVec::Engine engine(globalContext);
  engine.start();

  MittelVec::AudioGraph& graph = engine.graph;

  auto [gainNodeId, gainNodePtr] = graph.addNode<MittelVec::Gain>(0.25);
  auto [noiseGenNodeId, noiseGenNodePtr] = graph.addNode<MittelVec::NoiseGenerator>();
  auto [outputNodeId, outputNodePtr] = graph.addNode<MittelVec::Gain>(1.0);

  // MittelVec::AudioBuffer sample(globalContext);
  // // Load wav with miniaudio and write into AudioBuffer sample.
  // // ...
  // auto [polySamplerId, polySamplerPtr] = graph.addNode<MittelVec::PolyphonicSampler>(,6);

  graph.connect(noiseGenNodeId, gainNodeId);
  graph.connect(gainNodeId, outputNodeId);

  while (keepRunning) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return 0;
}
