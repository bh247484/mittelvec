#include <string>
#include "../include/Sampler.h"
#include "../miniaudio.h"

namespace MittelVec {

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

}
