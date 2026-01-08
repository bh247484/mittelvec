// MittelVec - Single-Header Library
// Generated on 2026-01-07

#ifndef MITTELVEC_H
#define MITTELVEC_H

// System includes 
#include <algorithm>
#include <cassert>
#include <list>
#include <memory>
#include <optional>
#include <random>
#include <unordered_map>
#include <vector>

// NOTE! This lib depends on miniaudio (a single header file audio lib)
// Ensure miniaudio.h is in your include path
#include "miniaudio.h"

namespace MittelVec {



class AudioBuffer {
public:
  AudioBuffer(const AudioContext& context);
  ~AudioBuffer();

  int getNumChannels() const;
  int getNumFrames() const;
  float getSampleRate() const;
  int size() const;
  void setAudioContext(AudioContext newContext);
  
  void resize(int newBufferSize);
  void clear();
  AudioBuffer& operator+=(const AudioBuffer& other);
  AudioBuffer operator+(const AudioBuffer& other) const;

  float& operator[](int index);
  const float& operator[](int index) const;

  // Made public so the vector can be accessed directly.
  std::vector<float> data;

private:
  int channels;
  int frames;
  float sampleRate;
};


struct AudioContext {
  int bufferSize;
  int numChannels;
  float sampleRate;
};



class AudioGraph {
public:
    AudioGraph(const AudioContext& context);

    template <typename NodeType, typename... Args>
    std::pair<int, NodeType*> addNode(Args &&...args)
    {
      int id = nextNodeId++;
      auto node = std::make_unique<NodeType>(audioContext, std::forward<Args>(args)...);
      nodes[id] = std::move(node);
      isGraphDirty = true;
      return std::make_pair(id, static_cast<NodeType*>(nodes[id].get()));
    }

    void removeNode(int nodeId);
    void connect(int sourceNodeId, int destNodeId);
    void disconnect(int sourceNodeId, int destNodeId);
    void processGraph(AudioBuffer& graphOutputBuffer);
    void setAudioContext(AudioContext newContext);

    void updateProcessOrder();
    AudioContext audioContext;
    std::unordered_map<int, std::unique_ptr<AudioNode>> nodes;
    std::unordered_map<int, std::vector<int>> connections;
    std::unordered_map<int, std::vector<int>> reverseConnections;
    int nextNodeId;
    std::vector<int> processOrder;
    bool isGraphDirty;
};


class AudioNode {
public:
  AudioNode(const AudioContext& context) : outputBuffer(context) {}
  virtual ~AudioNode() = default;

  virtual void process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) = 0;
  
  AudioBuffer outputBuffer;
};


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


struct EnvConfig {
  float attack;
  float decay;
  float sustain;
  float release;
};

class Envelope : public AudioNode {
public:
  explicit Envelope(const AudioContext& context, const EnvConfig& config = { 0.01f, 0.1f, 0.8f, 0.2f }); // Revisit these defaults values.

  enum State { Idle, Attack, Decay, Sustain, Release };

  // Revisit when you need dynamic, run time param changes.
  // Will have to reconfigure to atomics anyway.
  // void setAttack(float attack);
  // void setDecay(float decay);
  // void setSustain(float sustain);
  // void setRelease(float release);

  float getNextLevel();
  
  void noteOn();
  void noteOff();
  void reset();
  bool const isActive();

  void virtual process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) override;

  void applyToBuffer(AudioBuffer& buffer);

private:
  State state;
  float attack, decay, sustain, release;
  float sampleRate;
  float currentLevel;
  bool skipSustain = true; // hard coding this for now, may eventually find use case for noteOff/sustains.
};


enum class FilterMode { Lowpass, Highpass, Bandpass, Notch };

struct FilterConfig {
  FilterMode mode = FilterMode::Lowpass;
  float cutoff = 1000.0f;
  float resonance = 0.707f; // Q
};

class Filter : public AudioNode {
public:
  Filter(const AudioContext& context, const FilterConfig& config);

  void setParams(float cutoff, float resonance);
  void setMode(FilterMode mode);
  
  void process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) override;
  void applyToBuffer(AudioBuffer& buffer);

private:
  void calculateCoefficients();

  FilterMode mode;
  float cutoff;
  float resonance;
  float sampleRate;

  // Coefficients
  double b0 = 0, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
  
  // State memory (Z-delay lines)
  double z1_x = 0, z2_x = 0, z1_y = 0, z2_y = 0;
};


class Gain : public AudioNode {
public:
    explicit Gain(const AudioContext& context, float gain);

    void setGain(float gain);
    void virtual process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) override;

private:
    float gain;
};

    
class NoiseGenerator : public AudioNode {
    public:
    NoiseGenerator(const AudioContext& context);
    
    void process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) override;
    
    private:
    std::random_device randomDevice;
    std::mt19937 randomGen;
    std::uniform_real_distribution<float> dist;
};
    


class PitchShift : public AudioNode {
public:
  explicit PitchShift(const AudioContext& context, int semitoneShift);

  void virtual process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) override;
  void applyToBuffer(AudioBuffer& buffer);
  void setPitch(int semitoneShift);
  void reset();

private:
  // double samplePosition = 0.0;
  double currentDelay = 0.0;
  double ratio;
  std::vector<float> ringBuffer;
  int ringWriteIdx;

  float lerp(float a, float b, float fraction);
  float cubicInterpolation(float sm1, float s0, float s1, float s2, float fraction);
  // float sincInterpolation(const float* inputData, size_t numInputSamples, double position, int numTaps);
  double convertSemitoneToRatio(int semitoneShift);
  float getLerpSample(double samplePosition);
  float getCubicSample(double samplePosition);
};


struct SamplePackItem {
  std::string slug;
  std::string fileName;
  int polyphony;
  bool loop;
  float gain;
  int pitchShift;
  std::optional<EnvConfig> envConfig;
  std::optional<FilterConfig> filterConfig;

  // Constructor enforces required fields and default value for polyphony.
  SamplePackItem(
    std::string slug,
    std::string fileName,
    int polyphony = 1,
    bool loop = false,
    float gain = 1.0,
    int pitchShift = 0,
    std::optional<EnvConfig> env = std::nullopt,
    std::optional<FilterConfig> filterConfig = std::nullopt
  ) : slug(slug), fileName(fileName), polyphony(polyphony), loop(loop),
      gain(gain), pitchShift(pitchShift), envConfig(env), filterConfig(filterConfig) {}
};

class SamplePack {
public:
  SamplePack(AudioGraph& graph, std::vector<SamplePackItem> samplePackItems, std::string samplesDir, float gain = 1.0);
  void triggerSample(std::string slug);

  std::unordered_map<std::string, Sampler*> samplers;
  std::unique_ptr<Gain> output;

private:
  AudioGraph& graph;
};


// Consider making SamplerVoice its own class..
struct SamplerVoice {
  int playheadIndex = 0;
  bool active = false;

  std::unique_ptr<Envelope> envelope;
  std::unique_ptr<PitchShift> pitchShifter;
  std::unique_ptr<Filter> filter;

  AudioBuffer voiceBuffer;

  SamplerVoice(const AudioContext& context)
    : voiceBuffer(context),
    envelope(std::make_unique<Envelope>(context)),
    pitchShifter(std::make_unique<PitchShift>(context, 0)),
    filter(std::make_unique<Filter>(context, FilterConfig { FilterMode::Lowpass, 500.0f, 0.5f }))
  {}

  void trigger() {
    playheadIndex = 0;
    active = true;
    envelope->noteOn();
    pitchShifter->reset();
  }

  void processVoice(
    const AudioBuffer& sample,
    AudioBuffer& outputBuffer,
    bool loop,
    float gain,
    int pitchShift,
    std::optional<EnvConfig> envConfig,
    std::optional<FilterConfig> filterConfig
  ) {
    if (!active) return;
    
    voiceBuffer.clear();
    for (int i = 0; i < voiceBuffer.size(); ++i) {
      // If voice has reached the end of the sample.  
      if (playheadIndex >= sample.size()) {
        if (loop) {
          playheadIndex = 0;
          if (envConfig.has_value()) envelope->noteOn();
        } else {
          if (envConfig.has_value()) envelope->reset();
          active = false;
          playheadIndex = 0;
          break;
        }
      }

      // Write sample data into voiceBuffer.
      voiceBuffer[i] = sample[playheadIndex++] * gain;
    }

    // Apply per-voice DSP
    if (pitchShift && pitchShift != 0) {
      // This are probably being reset redundantly across processVoice calls.
      // Should cache the values or something.
      pitchShifter->setPitch(pitchShift);
      pitchShifter->applyToBuffer(voiceBuffer);
    }
    if (envConfig.has_value()) envelope->applyToBuffer(voiceBuffer);

    if (filterConfig.has_value()) {
      filter->setMode(filterConfig->mode);
      filter->setParams(filterConfig->cutoff, filterConfig->resonance);
      filter->applyToBuffer(voiceBuffer);
    }

    // Sum into main output buffer
    outputBuffer += voiceBuffer;
  }
};
    
class Sampler : public AudioNode {
  public:
  Sampler(
    const AudioContext& context,
    std::string samplePath,
    int polyphony,
    bool loop = false,
    float gain = 1.0f,
    int pitchShift = 0,
    std::optional<EnvConfig> envConfig = std::nullopt,
    std::optional<FilterConfig> filterConfig = std::nullopt
  );

  void noteOn();
  void noteOff();
  SamplerVoice* allocateVoice();
  
  void process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) override;
  
  private:
  int polyphony;
  AudioBuffer sample;
  std::vector<SamplerVoice> voices;
  std::list<SamplerVoice*> activeVoices; // indicies per voice of `voices` vector above.
  bool loop;
  float gain;
  int pitchShift;
  std::optional<EnvConfig> envConfig;
  std::optional<FilterConfig> filterConfig;
};
    
} // namespace MittelVec

#endif // MITTELVEC_H

#ifdef MITTELVEC_IMPLEMENTATION

#ifndef MINIAUDIO_IMPLEMENTATION
    #define MINIAUDIO_IMPLEMENTATION
#endif
#include "miniaudio.h"

namespace MittelVec {





AudioBuffer::AudioBuffer(const AudioContext& context)
: channels(context.numChannels), frames(context.bufferSize), sampleRate(context.sampleRate)
{
  data.resize(static_cast<int>(channels) * frames, 0.0f);
}

AudioBuffer::~AudioBuffer() = default;
    
int AudioBuffer::getNumChannels() const { return channels; }
int AudioBuffer::getNumFrames() const { return frames; }
float AudioBuffer::getSampleRate() const { return sampleRate; }
int AudioBuffer::size() const { return data.size(); }

void AudioBuffer::setAudioContext(AudioContext newContext) {
  channels = newContext.numChannels;
  frames = newContext.bufferSize;
  sampleRate = newContext.sampleRate;
  data.resize(static_cast<int>(channels) * frames, 0.0f);
}

// Clear buffer
void AudioBuffer::clear() { std::fill(data.begin(), data.end(), 0.0f); }

void AudioBuffer::resize(int newBufferSize) {
  frames = static_cast<int>(newBufferSize);
  data.resize(frames * channels, 0.0f); // fill buffer with zeroes while resizing.
}

// Operator overloads
AudioBuffer& AudioBuffer::operator+=(const AudioBuffer& other) {
  assert(channels == other.channels && frames == other.frames);
  for (int i = 0; i < data.size(); ++i) {
    data[i] += other.data[i];
  }
  return *this;
}
    
AudioBuffer AudioBuffer::operator+(const AudioBuffer& other) const {
  AudioBuffer result(*this);
  result += other;
  return result;
}

float& AudioBuffer::operator[](int index) {
  return data[index];
}

const float& AudioBuffer::operator[](int index) const {
  return data[index];
}







AudioGraph::AudioGraph(const AudioContext& context)
  : audioContext(context), nextNodeId(0), isGraphDirty(true) {}

void AudioGraph::removeNode(int nodeId) {
  if (nodes.find(nodeId) == nodes.end()) return;

  nodes.erase(nodeId);
  connections.erase(nodeId);
  reverseConnections.erase(nodeId);

  for (auto& pair : connections) {
    auto& destinations = pair.second;
    destinations.erase(std::remove(destinations.begin(), destinations.end(), nodeId), destinations.end());
  }

  for (auto& pair : reverseConnections) {
    auto& sources = pair.second;
    sources.erase(std::remove(sources.begin(), sources.end(), nodeId), sources.end());
  }

  isGraphDirty = true;
}

void AudioGraph::connect(int sourceNodeId, int destNodeId) {
  if (nodes.find(sourceNodeId) == nodes.end() || nodes.find(destNodeId) == nodes.end()) {
    return;
  }

  connections[sourceNodeId].push_back(destNodeId);
  reverseConnections[destNodeId].push_back(sourceNodeId);
  isGraphDirty = true;
}

void AudioGraph::disconnect(int sourceNodeId, int destNodeId) {
  if (connections.count(sourceNodeId)) {
    auto& destinations = connections[sourceNodeId];
    destinations.erase(std::remove(destinations.begin(), destinations.end(), destNodeId), destinations.end());
  }

  if (reverseConnections.count(destNodeId)) {
    auto& sources = reverseConnections[destNodeId];
    sources.erase(std::remove(sources.begin(), sources.end(), sourceNodeId), sources.end());
  }

  isGraphDirty = true;
}

void AudioGraph::updateProcessOrder() {
  processOrder.clear();
  if (nodes.empty()) {
    isGraphDirty = false;
    return;
  }

  // Map to store in-degrees of each node
  std::unordered_map<int, int> inDegree;
  for (const auto& pair : nodes) {
    inDegree[pair.first] = 0;
  }

  // Calculate in-degrees
  for (const auto& pair : connections) {
    for (int destNodeId : pair.second) {
      inDegree[destNodeId]++;
    }
  }

  // Initialize queue with nodes having an in-degree of 0 (our source nodes)
  std::queue<int> queue;
  for (const auto& pair : inDegree) {
    if (pair.second == 0) {
      queue.push(pair.first);
    }
  }

  // Process nodes using Kahn's algorithm
  while (!queue.empty()) {
    int nodeId = queue.front();
    queue.pop();
    processOrder.push_back(nodeId);

    // Decrement inDegrees of neighboring nodes
    if (connections.count(nodeId)) {
      for (int neighbor : connections[nodeId]) {
        inDegree[neighbor]--;
        if (inDegree[neighbor] == 0) {
          queue.push(neighbor);
        }
      }
    }
  }

  // Check for cycles. If a cycle exists, the graph is invalid.
  if (processOrder.size() != nodes.size()) {
    std::cerr << "Error: Cycle detected in the audio graph. Audio processing will be stopped." << std::endl;
    processOrder.clear(); // Clear the invalid processing order
  }

  isGraphDirty = false;
}


void AudioGraph::processGraph(AudioBuffer& graphOutputBuffer) {
  // If the graph structure has changed, recalculate the processing order.
  if (isGraphDirty) {
    updateProcessOrder();
  }

  // If the process order is empty (and there are nodes), it means a cycle was detected.
  if (processOrder.empty() && !nodes.empty()) {
    graphOutputBuffer.clear();
    return; // Output silence if graph is invalid
  }

  // Process each node in the topologically sorted order
  for (int nodeId : processOrder) {
    auto& node = nodes[nodeId];

    // Find inputs for the current node from its predecessors' output buffers
    std::vector<const AudioBuffer*> inputs;
    if (reverseConnections.count(nodeId)) {
      for (int sourceId : reverseConnections[nodeId]) {
        if (nodes.count(sourceId)) {
          inputs.push_back(&(nodes[sourceId]->outputBuffer));
        }
      }
    }
    // Process the node
    node->process(inputs, node->outputBuffer);
  }

  // Sum the outputs of all "terminal" nodes (nodes with no outgoing connections)
  graphOutputBuffer.clear();
  for (auto const& [nodeId, node] : nodes) {
    if (connections.find(nodeId) == connections.end() || connections[nodeId].empty()) {
      for (int i = 0; i < graphOutputBuffer.size(); ++i) {
        graphOutputBuffer[i] += node->outputBuffer[i];
      }
    }
  }
}

void AudioGraph::setAudioContext(AudioContext newContext)
{
  audioContext = newContext;
}

#define MINIAUDIO_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"

#pragma GCC diagnostic pop


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




Envelope::Envelope(const AudioContext& context, const EnvConfig& config)
  : AudioNode(context), attack(config.attack), decay(config.decay), sustain(config.sustain), release(config.release), sampleRate(context.sampleRate) {}

float Envelope::getNextLevel() {
  switch (state) {
  case Idle:
    break;
  case Attack:
    currentLevel += 1.0 / (attack * sampleRate);

    // Clamp level and transition to decay.
    if (currentLevel >= 1.0) {
      currentLevel = 1.0;
      state = Decay;
    }
    break;
  case Decay:
    currentLevel -= (1.0 - sustain) / (decay * sampleRate);
    
    if (currentLevel <= sustain) {
      currentLevel = sustain;
      state = Sustain;
    }
    break;
  case Sustain:
    if (skipSustain) {
      state = Release;
    }
    break;
  case Release:
    currentLevel -= sustain / (release * sampleRate);
    
    if (currentLevel <= 0.0) {
      currentLevel = 0.0;
      state = Idle;
    }
    break;
  default:
    break;
  }

  return currentLevel;
}

void Envelope::noteOn() {
  state = Attack;
  currentLevel = 0; // confirm this...
}

void Envelope::noteOff() {
  state = Release;
}

void Envelope::reset() {
  currentLevel = 0;
  state = Idle;
}

bool const Envelope::isActive() {
  return state != Idle;
}

void Envelope::process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) {
  if (inputs.empty()) return;
  outputBuffer.clear();

  // Sum all input buffers into the output buffer
  for (const auto* inputBufferPtr : inputs) {
    for (int i = 0; i < outputBuffer.size(); ++i) {
      outputBuffer[i] += (*inputBufferPtr)[i];
    }
  }

  // Apply envelope to the summed signal
  for (int i = 0; i < outputBuffer.size(); ++i) {
    outputBuffer[i] *= getNextLevel();
  }
}

/**
 * Applies envelope directly to incoming buffer.
 * Allows for inline processing as opposed to more modular node/graph style `process` method approach.
 */
void Envelope::applyToBuffer(AudioBuffer& buffer) {
  for (int i = 0; i < buffer.size(); ++i) {
    buffer[i] *= getNextLevel();
  }
}



// C++ 17 doesn't have PI constant.
// We can use arccosine of -1.0 to get pi instead.
// When x == -1.0 on the unit circle, theta == pi.
const double PI = std::acos(-1.0);


Filter::Filter(const AudioContext& context, const FilterConfig& config)
  : AudioNode(context), 
    mode(config.mode), 
    cutoff(config.cutoff), 
    resonance(config.resonance), 
    sampleRate(static_cast<float>(context.sampleRate)) 
{
  calculateCoefficients();
}

void Filter::calculateCoefficients() {
  double w0 = 2.0 * PI * cutoff / sampleRate;
  double alpha = std::sin(w0) / (2.0 * resonance);
  double cosW0 = std::cos(w0);

  double a0 = 0;

  switch (mode) {
    case FilterMode::Lowpass:
      b0 = (1.0 - cosW0) / 2.0;
      b1 = 1.0 - cosW0;
      b2 = (1.0 - cosW0) / 2.0;
      a0 = 1.0 + alpha;
      a1 = -2.0 * cosW0;
      a2 = 1.0 - alpha;
      break;
    case FilterMode::Highpass:
      b0 = (1.0 + cosW0) / 2.0;
      b1 = -(1.0 + cosW0);
      b2 = (1.0 + cosW0) / 2.0;
      a0 = 1.0 + alpha;
      a1 = -2.0 * cosW0;
      a2 = 1.0 - alpha;
      break;
    case FilterMode::Bandpass:
      b0 = alpha;
      b1 = 0;
      b2 = -alpha;
      a0 = 1.0 + alpha;
      a1 = -2.0 * cosW0;
      a2 = 1.0 - alpha;
      break;
    case FilterMode::Notch:
      b0 = 1.0;
      b1 = -2.0 * cosW0;
      b2 = 1.0;
      a0 = 1.0 + alpha;
      a1 = -2.0 * cosW0;
      a2 = 1.0 - alpha;
      break;
  }

  // Normalize all coefficients by a0
  b0 /= a0; b1 /= a0; b2 /= a0;
  a1 /= a0; a2 /= a0;

  // Debug...
  // printf("Filter: F=%f, Q=%f, SR=%f | b0=%f, a1=%f\n", cutoff, resonance, sampleRate, b0, a1);
}

void Filter::setParams(float newCutoff, float newResonance) {
  cutoff = newCutoff;
  resonance = newResonance;
  calculateCoefficients();
}

void Filter::setMode(FilterMode newMode) {
  mode = newMode;
  calculateCoefficients();
}

void Filter::process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) {
  if (inputs.empty()) return;
  outputBuffer.clear();

  // Sum inputs
  for (const auto* inputBufferPtr : inputs) {
    for (int i = 0; i < outputBuffer.size(); ++i) {
      outputBuffer[i] += (*inputBufferPtr)[i];
    }
  }

  applyToBuffer(outputBuffer);
}

void Filter::applyToBuffer(AudioBuffer& buffer) {
  for (int i = 0; i < buffer.size(); ++i) {
    double x = buffer[i];
    
    // Difference Equation (Direct Form I)
    double y = (b0 * x) + (b1 * z1_x) + (b2 * z2_x) - (a1 * z1_y) - (a2 * z2_y);

    // Update state
    z2_x = z1_x;
    z1_x = x;
    z2_y = z1_y;
    z1_y = y;

    buffer[i] = static_cast<float>(y);
  }
}




Gain::Gain(const AudioContext& context, float gain)
  : AudioNode(context), gain(gain) {}

void Gain::setGain(float newGain) {
  this->gain = newGain;
}

void Gain::process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) {
  outputBuffer.clear();

  if (inputs.empty()) {
    return;
  }

  // Sum all input buffers into the output buffer
  // Revisit this... Seems wasteful if inputs.length < 1.
  for (const auto* inputBufferPtr : inputs) {
    for (int i = 0; i < outputBuffer.size(); ++i) {
      outputBuffer[i] += (*inputBufferPtr)[i];
    }
  }

  // Apply gain to the summed signal
  for (int i = 0; i < outputBuffer.size(); ++i) {
    outputBuffer[i] *= gain;
  }
}




NoiseGenerator::NoiseGenerator(const AudioContext& context)
  : AudioNode(context),
    randomGen(randomDevice()),
    dist(-1.0f, 1.0f)
{}

void NoiseGenerator::process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) {
  for (int i = 0; i < outputBuffer.size(); ++i) {
    outputBuffer[i] = dist(randomGen);
  }
}




PitchShift::PitchShift(const AudioContext& context, int semitoneShift)
  : AudioNode(context), currentDelay(0.0), ringWriteIdx(0), ratio(convertSemitoneToRatio(semitoneShift)) {
    // Preallocate ringBuffer.
    ringBuffer.resize(context.bufferSize * context.numChannels);

    // Use below if you want custom window size.
    // Set a window size of ~20ms (adjust to taste)
    // int windowSize = static_cast<int>(context.sampleRate * 0.020);
    // ringBuffer.resize(windowSize);
  }

  
void PitchShift::process(const std::vector<const AudioBuffer*>& inputs, AudioBuffer& outputBuffer) {
  if (inputs.empty()) {
    outputBuffer.clear();
    return;
  }

  const int numInputSamples = inputs[0]->getNumFrames();
  
  // Sum all input buffers into the output buffer
  // Revisit this... Seems wasteful if inputs.length < 1.
  for (const auto* inputBufferPtr : inputs) {
    for (int i = 0; i < outputBuffer.size(); ++i) {
      outputBuffer[i] += (*inputBufferPtr)[i];
    }
  }

  applyToBuffer(outputBuffer);
}

void PitchShift::applyToBuffer(AudioBuffer& buffer) {
  const int numInputSamples = buffer.getNumFrames();
  const double ringSize = static_cast<double>(ringBuffer.size());
  // Not sure this margin is strictly necessary but may mitigate some pops/clicks.
  const double safetyMargin = 20.0;

  for (int i = 0; i < numInputSamples; ++i) {
    // Write sample into ringBuffer
    ringBuffer[ringWriteIdx] = buffer[i];

    // Offset for Dual Tap delay.
    currentDelay += (1.0 - ratio);

    // Wrap currentDelay
    if (currentDelay >= ringSize) currentDelay -= ringSize;
    if (currentDelay < 0) currentDelay += ringSize;

    double tapAPos = static_cast<double>(ringWriteIdx) - (currentDelay + safetyMargin);
    if (tapAPos < 0) tapAPos += ringSize; // wrap

    double tapBPos = tapAPos + (ringSize * 0.5); // Offset by 180 deg
    if (tapBPos >= ringSize) tapBPos -= ringSize; // wrap

    // Triangle window for crossfading between taps.
    // Consider using Hamming window.
    float windowPhase = static_cast<float>(currentDelay / ringSize);
    float gainA = 1.0f - std::abs((windowPhase * 2.0f) - 1.0f);
    float gainB = 1.0f - gainA;

    float tapA = getCubicSample(tapAPos);
    float tapB = getCubicSample(tapBPos);
    buffer[i] = (tapA * gainA) + (tapB * gainB);

    // Increment write index.
    ringWriteIdx = (ringWriteIdx + 1) % static_cast<int>(ringSize);
  }
}

void PitchShift::setPitch(int semitoneShift) {
  ratio = convertSemitoneToRatio(semitoneShift);
}

void PitchShift::reset() {
  // samplePosition = 0.0;
  currentDelay = 0.0;
  ringWriteIdx = 0;
  std::fill(ringBuffer.begin(), ringBuffer.end(), 0.0f);
}

float PitchShift::getLerpSample(double samplePosition) {
  int index0 = static_cast<int>(std::floor(samplePosition));
  int index1 = (index0 + 1) % static_cast<int>(ringBuffer.size());
  float fraction = static_cast<float>(samplePosition - std::floor(samplePosition));

  return lerp(ringBuffer[index0], ringBuffer[index1], fraction);
}

float PitchShift::getCubicSample(double samplePosition) {
  int i = static_cast<int>(std::floor(samplePosition));
  float fraction = static_cast<float>(samplePosition - i);
  int size = static_cast<int>(ringBuffer.size());

  // Get 4 samples for cubic interpolation: i-1, i, i+1, i+2
  float sm1 = ringBuffer[(i - 1 + size) % size];
  float s0  = ringBuffer[i % size];
  float s1  = ringBuffer[(i + 1) % size];
  float s2  = ringBuffer[(i + 2) % size];

  return cubicInterpolation(sm1, s0, s1, s2, fraction);
}

float PitchShift::lerp(float a, float b, float fraction) {
  return a + (b - a) * fraction;
}

// sm1 = s-1
float PitchShift::cubicInterpolation(float sm1, float s0, float s1, float s2, float fraction) {
  float a = -0.5f * sm1 + 1.5f * s0 - 1.5f * s1 + 0.5f * s2;
  float b = sm1 - 2.5f * s0 + 2.0f * s1 - 0.5f * s2;
  float c = -0.5f * sm1 + 0.5f * s1;
  float d = s0;

  return d + fraction * (c + fraction * (b + fraction * a));
}

// void Gain::sincInterpolation(const float* inputData, size_t numInputSamples, double position, int numTaps) {
  
// }

double PitchShift::convertSemitoneToRatio(int semitoneShift) {
  const double exponent = static_cast<double>(semitoneShift) / 12.0;
    
  // The base is 2 (for an octave)
  return std::pow(2.0, exponent);
}




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






Sampler::Sampler(
  const AudioContext &context,
  std::string samplePath,
  int polyphony,
  bool loop,
  float gain,
  int pitchShift,
  std::optional<EnvConfig> envConfig,
  std::optional<FilterConfig> filterConfig
)
  : AudioNode(context), sample(context), polyphony(polyphony),
  loop(loop), gain(gain), pitchShift(pitchShift),
  envConfig(envConfig), filterConfig(filterConfig)
{
  // Load sample
  ma_decoder decoder;
  ma_decoder_config decoderConfig = ma_decoder_config_init(
    ma_format_f32,
    context.numChannels,
    context.sampleRate
  );

  if (ma_decoder_init_file(samplePath.c_str(), &decoderConfig, &decoder) != MA_SUCCESS) {
    printf("Failed to load WAV file at path %s\n", samplePath.c_str());
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
  }
  
  // Cleanup miniaudio decoder.
  ma_decoder_uninit(&decoder);

  // Setup voices.
  voices.reserve(polyphony);
  for (int i = 0; i < polyphony; ++i) {
    voices.emplace_back(context);
  }
}

SamplerVoice* Sampler::allocateVoice() {
  // Try first inactive voice.
  for (SamplerVoice& voice : voices) {
    if (!voice.active) {
      return &voice;
    }
  }

  // Steal oldest voice when all voices active.
  SamplerVoice* oldestVoice = activeVoices.front();
  activeVoices.pop_front();
  return oldestVoice;
}

void Sampler::noteOn() {
  SamplerVoice* freeVoice = allocateVoice();
  freeVoice->trigger();
  activeVoices.push_back(freeVoice);
}

void Sampler::process(const std::vector<const AudioBuffer *> &inputs, AudioBuffer &outputBuffer) {
  outputBuffer.clear();

  for (SamplerVoice& voice : voices) {
    if (!voice.active) continue;

    voice.processVoice(
      sample,
      outputBuffer,
      loop,
      gain,
      pitchShift,
      envConfig,
      filterConfig
    );

    if (!voice.active) {
      activeVoices.remove(&voice);
    }
  }

}

} // namespace MittelVec

#endif // MITTELVEC_IMPLEMENTATION
