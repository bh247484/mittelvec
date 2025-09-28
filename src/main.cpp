#define MINIAUDIO_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#include "miniaudio.h"
#pragma GCC diagnostic pop

#include <stdio.h>
#include "../include/AudioBuffer.h"
#include "../include/AudioGraph.h"
#include "../include/Gain.h"
#include "../include/NoiseGenerator.h"
#include "../include/AudioContext.h"

const int BUFFER_SIZE = 512;
const int NUM_CHANNELS = 2;
const float SAMPLE_RATE = 44100.0f;

struct CallbackData {
  Middleman::AudioGraph* graph = nullptr;
  Middleman::AudioBuffer* graphOutput = nullptr;
};

// Callback for audio playback.
void miniaudio_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  float* out = (float*)pOutput;
  CallbackData* cbData = (CallbackData*)pDevice->pUserData;

  // Write graph data into graph output buffer.
  cbData->graph->processGraph(*cbData->graphOutput);
  
  // Copy graph output buffer into miniaudio output buffer.
  memcpy(out, (*cbData->graphOutput).data.data(), frameCount * NUM_CHANNELS * sizeof(float));
  // std::copy((*cbData->graphOutput).data.begin(), (*cbData->graphOutput).data.end(), out);
  // for (ma_uint32 i = 0; i < frameCount * pDevice->playback.channels; i++) {
  //   out[i] = (*(cbData->graphOutput))[i];
  // }

  (void)pInput; // unused
}

int main() {
  
  // Setup miniaudio
  ma_result result;
  ma_device_config config;
  ma_device device;
  CallbackData cbData;

  config = ma_device_config_init(ma_device_type_playback);
  config.playback.format    = ma_format_f32;
  config.playback.channels  = NUM_CHANNELS;
  config.sampleRate         = int(SAMPLE_RATE);
  config.periodSizeInFrames = BUFFER_SIZE;
  config.dataCallback       = miniaudio_callback;
  config.pUserData          = &cbData;

  ma_device device;
  if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
    assert(false);
  }

  // Miniaudio buffer size not guaranteed here so we need to query it after initialization.
  auto miniaudioBufferSize = device.playback.internalPeriodSizeInFrames;

  // Setup middleware graph
  Middleman::AudioContext context = { miniaudioBufferSize, NUM_CHANNELS, SAMPLE_RATE };
  Middleman::AudioGraph graph(context);

  auto [gainNodeId, gainNodePtr] = graph.addNode<Middleman::Gain>(0.5);
  auto [noiseGenNodeId, noiseGenNodePtr] = graph.addNode<Middleman::NoiseGenerator>();
  auto [outputNodeId, outputNodePtr] = graph.addNode<Middleman::Gain>(1.0);

  graph.connect(noiseGenNodeId, gainNodeId);
  graph.connect(gainNodeId, outputNodeId);

  Middleman::AudioBuffer graphOutput(context);

  // Update pUserData pointers now that graph/graphOutput have been initalized.
  cbData.graph = &graph;
  cbData.graphOutput = &graphOutput;

  return 0;
}
