#pragma once
#include <stdio.h>
#include <csignal>
#include <thread>
#include <chrono>
#include <atomic>
#include <iostream>
#include "../include/Engine.h"
#include "../include/AudioBuffer.h"
#include "../include/AudioGraph.h"
#include "../include/Gain.h"
#include "../include/NoiseGenerator.h"
#include "../include/AudioContext.h"
#include "../include/Sampler.h"
#include "../include/SamplePack.h"

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

  // auto [gainNodeId, gainNodePtr] = graph.addNode<MittelVec::Gain>(0.05);
  // auto [noiseGenNodeId, noiseGenNodePtr] = graph.addNode<MittelVec::NoiseGenerator>();
  // auto [outputNodeId, outputNodePtr] = graph.addNode<MittelVec::Gain>(3.0);

  // graph.connect(noiseGenNodeId, gainNodeId);
  // graph.connect(gainNodeId, outputNodeId);

  // Declare SamplePackItems.
  std::vector<MittelVec::SamplePackItem> samplePackItems = {
    {
      "blip",
      "/Users/bh/Documents/game-audio/middleware-exp/sounds/blip.wav",
      6,
      false,
      5.0, // gain
      // {
      //   1.0,
      //   1.0,
      //   1.0,
      //   1.0,
      // }
    },
  };

  // Pass SamplePackItems to and instantiate SamplePack.
  MittelVec::SamplePack samplePack(graph, samplePackItems);
  samplePack.triggerSample("blip");

  while (keepRunning) {
    std::string input;
    if (std::cin >> input) {
      if (input == "t") {
        samplePack.triggerSample("blip");
      }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  return 0;
}
