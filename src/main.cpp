#include <stdio.h>
#include "../include/AudioGraph.h"
#include "../include/Gain.h"
#include "../include/NoiseGenerator.h"
#include "../include/AudioContext.h"

const int BUFFER_SIZE = 512;
const int NUM_CHANNELS = 2;
const float SAMPLE_RATE = 44100.0f;

int main() {
  printf("hello");
  AudioContext context = { BUFFER_SIZE, NUM_CHANNELS, SAMPLE_RATE };

  AudioGraph graph(context);

  auto [gainNodeId, gainNodePtr] = graph.addNode<Gain>(0.5);
  auto [noiseGenNodeId, noiseGenNodePtr] = graph.addNode<NoiseGenerator>();
  auto [outputNodeId, outputNodePtr] = graph.addNode<Gain>(1.0);

  graph.connect(noiseGenNodeId, gainNodeId);
  graph.connect(gainNodeId, outputNodeId);

  return 0;
}
