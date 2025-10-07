#include "../include/Engine.h"
#define MINIAUDIO_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#include "../include/miniaudio.h"
#pragma GCC diagnostic pop

namespace MittelVec {

// Callback for audio playback.
void miniaudio_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  float* out = (float*)pOutput;
  CallbackData* cbData = (CallbackData*)pDevice->pUserData;

  // Write graph data into graph output buffer.
  cbData->graph->processGraph(*cbData->graphOutput);
  
  // Copy graph output buffer into miniaudio output buffer.
  memcpy(out, (*cbData->graphOutput).data.data(), frameCount * cbData->globalContext->numChannels * sizeof(float));
  // std::copy((*cbData->graphOutput).data.begin(), (*cbData->graphOutput).data.end(), out);
  // for (ma_uint32 i = 0; i < frameCount * pDevice->playback.channels; i++) {
  //   out[i] = (*(cbData->graphOutput))[i];
  // }

  (void)pInput; // unused
}

Engine::Engine(AudioContext globalContext)
  : globalContext(globalContext),
    graph(globalContext),
    output(globalContext)
{
  initMiniaudio();
}

Engine::~Engine() {
  stop();
}

void Engine::initMiniaudio() {
  // Setup miniaudio
  config = ma_device_config_init(ma_device_type_playback);
  config.playback.format    = ma_format_f32;
  config.playback.channels  = globalContext.numChannels;
  config.sampleRate         = static_cast<int>(globalContext.sampleRate);
  config.periodSizeInFrames = globalContext.bufferSize;
  config.dataCallback       = miniaudio_callback;
  config.pUserData          = &cbData;

  if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
    assert(false);
  }

  // Miniaudio buffer size not guaranteed here.
  // We need to cover our bases in the unlikely event the request buffer sized can't be applied.
  auto miniaudioBufferSize = device.playback.internalPeriodSizeInFrames;

  if (globalContext.bufferSize != miniaudioBufferSize) {
    printf(
      "Miniaudio backend couldn't assign requested buffer size of %d. Buffer size %d chosen and assigned instead.\n",
      globalContext.bufferSize,
      miniaudioBufferSize
    );
    globalContext.bufferSize = miniaudioBufferSize;
    // Update graph and output buffer's audioContext
    graph.setAudioContext(globalContext);
    output.setAudioContext(globalContext);
  }
}

void Engine::start() {
  // Update pUserData pointers now that graph/graphOutput/globalContext have been initalized.
  cbData.graph = &graph;
  cbData.graphOutput = &output;
  cbData.globalContext = &globalContext;

  if (ma_device_start(&device) != MA_SUCCESS) {
    assert(false);
  }
}

void Engine::stop() {
  ma_device_uninit(&device);
}

}