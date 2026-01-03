#pragma once

#include <JuceHeader.h>

namespace JazzArchitect {

class AudioEngine {
public:
    AudioEngine();
    void setSampleRate(float sr);
    void process(float* left, float* right, int numSamples);

private:
    float sampleRate_ = 48000.0f;
};

} // namespace JazzArchitect
