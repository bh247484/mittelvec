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
#include "../include/MusicCueOrchestrator.h"

const int BUFFER_SIZE = 512;
const int NUM_CHANNELS = 1; // Keep mono for now, will need to refactor dsp to include channels in processing loops.
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
      "nbc-chimes", // slug
      "nbc-chimes.wav",
      6, // polyphony
      false, // loop
      5.0, // gain
      0, // pitchShift
    },
    {
      "nbc-chimes-high", // slug
      "nbc-chimes.wav",
      6, // polyphony
      false, // loop
      5.0, // gain
      7, // pitchShift
    },
    {
      "nbc-chimes-low", // slug
      "nbc-chimes.wav",
      6, // polyphony
      false, // loop
      5.0, // gain
      -7, // pitchShift
    },
    {
      "blip", // slug
      "blip.wav",
      6, // polyphony
      false, // loop
      5.0, // gain
      0, // pitchShift
    },
    {
      "blip-high", // slug
      "blip.wav",
      6, // polyphony
      false, // loop
      5.0, // gain
      8, // pitchShift
    },
    {
      "blip-low", // slug
      "blip.wav",
      6, // polyphony
      false, // loop
      5.0, // gain
      -5, // pitchShift
    },
    {
      "perc2", // slug
      "perc.wav",
      6, // polyphony
      false, // loop
      5.0, // gain
      2, // pitchShift
    },
    {
      "perc3", // slug
      "perc.wav",
      6, // polyphony
      false, // loop
      5.0, // gain
      -3, // pitchShift
    },
    {
      "perc",
      "perc.wav",
      6, // polyphony
      false, // loop
      5.0, // gain
      0, // pitchShift
    },
  };

  // Pass SamplePackItems to and instantiate SamplePack.
  std::string samplesDir = "/Users/bh/Documents/game-audio/middleware-exp/sounds/";
  MittelVec::SamplePack samplePack(graph, samplePackItems, samplesDir);

  std::vector<MittelVec::MusicCue> musicCues = {
    {
      "cue-1", // slug
      "cue-1.wav", // fileName
      true, // loop
    },
    {
      "cue-2", // slug
      "cue-2.wav", // fileName
      true, // loop
    },
  };

  std::string cuesDir = "/Users/bh/Documents/game-audio/middleware-exp/cues/";
  MittelVec::MusicCueOrchestrator cueOrchestrator(graph, musicCues, cuesDir);

  while (keepRunning) {
    std::string input;
    if (std::cin >> input) {
      if (input == "k") {
        samplePack.triggerSample("nbc-chimes");
      }

      if (input == "l") {
        samplePack.triggerSample("nbc-chimes-high");
      }

      if (input == "j") {
        samplePack.triggerSample("nbc-chimes-low");
      }

      if (input == "b") {
        samplePack.triggerSample("blip");
      }

      if (input == "n") {
        samplePack.triggerSample("blip-high");
      }

      if (input == "v") {
        samplePack.triggerSample("blip-low");
      }

      if (input == "y") {
        samplePack.triggerSample("perc2");
      }

      if (input == "r") {
        samplePack.triggerSample("perc3");
      }

      if (input == "t") {
        samplePack.triggerSample("perc");
      }

      if (input == "q") {
        cueOrchestrator.playCue("cue-1");
      }

      if (input == "w") {
        cueOrchestrator.playCue("cue-2");
      }

      if (input == "e") {
        cueOrchestrator.stopCue();
      }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  return 0;
}
