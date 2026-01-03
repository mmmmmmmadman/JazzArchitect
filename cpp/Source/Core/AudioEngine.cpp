#include "AudioEngine.h"

namespace JazzArchitect {

AudioEngine::AudioEngine() {}

void AudioEngine::setSampleRate(float sr) {
    sampleRate_ = sr;
}

void AudioEngine::process(float* left, float* right, int numSamples) {
    // TODO: Implement chord synthesis
    for (int i = 0; i < numSamples; ++i) {
        left[i] = 0.0f;
        right[i] = 0.0f;
    }
}

} // namespace JazzArchitect
