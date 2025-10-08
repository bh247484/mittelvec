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

  // auto [gainNodeId, gainNodePtr] = graph.addNode<MittelVec::Gain>(0.25);
  // auto [noiseGenNodeId, noiseGenNodePtr] = graph.addNode<MittelVec::NoiseGenerator>();
  auto [outputNodeId, outputNodePtr] = graph.addNode<MittelVec::Gain>(1.0);

  const char* wavPath = "/Users/bh/Documents/game-audio/middleware-exp/sounds/blip.wav";

  ma_decoder decoder;
  ma_decoder_config decoderConfig = ma_decoder_config_init(
    ma_format_f32,
    globalContext.numChannels,
    globalContext.sampleRate
  );
  MittelVec::AudioBuffer sample(globalContext);

  if (ma_decoder_init_file(wavPath, &decoderConfig, &decoder) != MA_SUCCESS) {
    printf("Failed to load WAV file.\n");
  } else {
    ma_uint64 totalFrames;
    ma_result result = ma_decoder_get_length_in_pcm_frames(&decoder, &totalFrames);

    if (result != MA_SUCCESS) {
      printf("Failed to get length of WAV file.\n");
      ma_decoder_uninit(&decoder);
    }

    sample.resize(static_cast<size_t>(totalFrames));

    ma_uint64 framesRead;
    result = ma_decoder_read_pcm_frames(&decoder, sample.data.data(), totalFrames, &framesRead);
    if (result != MA_SUCCESS) {
      printf("Failed to read WAV file.\n");
    }

    // Optionally set frame count
    // sample.frames = static_cast<int>(framesRead);

    ma_decoder_uninit(&decoder);
  }

  // Load wav with miniaudio and write into AudioBuffer sample.
  auto [polySamplerId, polySamplerPtr] = graph.addNode<MittelVec::PolyphonicSampler>(sample, 6);

  // graph.connect(noiseGenNodeId, gainNodeId);
  // graph.connect(gainNodeId, outputNodeId);
  graph.connect(polySamplerId, outputNodeId);

  while (keepRunning) {
    std::string input;
    if (std::cin >> input) {
      if (input == "t") {
        polySamplerPtr->noteOn();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        polySamplerPtr->noteOn();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        polySamplerPtr->noteOn();
      }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  return 0;
}
