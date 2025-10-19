#include <string>
#include "../include/PolyphonicSampler.h"
#include "../include/miniaudio.h"

namespace MittelVec {

PolyphonicSampler::PolyphonicSampler(const AudioContext &context, std::string samplePath, int polyphony)
  : AudioNode(context), sample(context), polyphony(polyphony)
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

    // Optionally set frame count
    // sample.frames = static_cast<int>(framesRead);
  }
  
  // Cleanup miniaudio decoder.
  ma_decoder_uninit(&decoder);


  // Setup voices.
  voices.reserve(polyphony);
  for (int i = 0; i < polyphony; ++i) {
    voices.emplace_back(context);
  }
}

SamplerVoice* PolyphonicSampler::allocateVoice() {
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

void PolyphonicSampler::noteOn() {
  SamplerVoice* freeVoice = allocateVoice();
  freeVoice->trigger();
  activeVoices.push_back(freeVoice);
}

void PolyphonicSampler::process(const std::vector<const AudioBuffer *> &inputs, AudioBuffer &outputBuffer) {
  outputBuffer.clear();

  for (SamplerVoice& voice : voices) {
    if (!voice.active) continue;

    voice.processVoice(sample, outputBuffer);

    if (!voice.active) {
      activeVoices.remove(&voice);
    }
  }

}

}
