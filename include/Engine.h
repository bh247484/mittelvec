#pragma once
#include "AudioBuffer.h"
#include "AudioContext.h"
#include "AudioGraph.h"
#include "miniaudio.h"

namespace MittelVec {

struct CallbackData {
  AudioGraph* graph = nullptr;
  AudioBuffer* graphOutput = nullptr;
  AudioContext* globalContext = nullptr;
};

class Engine {
public:
  Engine(AudioContext globalContext);
  ~Engine();

  AudioContext globalContext;
  AudioGraph graph;
  AudioBuffer output;

  void start();
  void stop();

private:
  void initMiniaudio();

  // Miniaudio
  ma_result result;
  ma_device_config config;
  ma_device device;
  CallbackData cbData;
};

}