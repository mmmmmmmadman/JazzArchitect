#pragma once

#include "../Core/ChordSymbol.h"
#include <cmath>
#include <array>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace JazzArchitect {

/**
 * Synth voice types
 */
enum class SynthType {
    ELECTRIC_PIANO,  // FM-style Rhodes
    ORGAN,           // Additive Hammond-style
    PAD              // Soft pad sound
};

/**
 * ADSR Envelope
 */
class ADSREnvelope {
public:
    void setSampleRate(float sr) { sampleRate_ = sr; }

    void setAttack(float ms) { attack_ = std::max(1.0f, ms); }
    void setDecay(float ms) { decay_ = std::max(1.0f, ms); }
    void setSustain(float level) { sustain_ = std::clamp(level, 0.0f, 1.0f); }
    void setRelease(float ms) { release_ = std::max(1.0f, ms); }

    void trigger() {
        stage_ = Stage::Attack;
        value_ = 0.0f;
    }

    void release() {
        if (stage_ != Stage::Idle) {
            stage_ = Stage::Release;
            releaseStart_ = value_;
        }
    }

    float process() {
        float delta;

        switch (stage_) {
            case Stage::Attack:
                delta = 1.0f / ((attack_ / 1000.0f) * sampleRate_);
                value_ += delta;
                if (value_ >= 1.0f) {
                    value_ = 1.0f;
                    stage_ = Stage::Decay;
                }
                break;

            case Stage::Decay:
                delta = (1.0f - sustain_) / ((decay_ / 1000.0f) * sampleRate_);
                value_ -= delta;
                if (value_ <= sustain_) {
                    value_ = sustain_;
                    stage_ = Stage::Sustain;
                }
                break;

            case Stage::Sustain:
                value_ = sustain_;
                break;

            case Stage::Release:
                delta = releaseStart_ / ((release_ / 1000.0f) * sampleRate_);
                value_ -= delta;
                if (value_ <= 0.0f) {
                    value_ = 0.0f;
                    stage_ = Stage::Idle;
                }
                break;

            case Stage::Idle:
                value_ = 0.0f;
                break;
        }

        return value_;
    }

    bool isActive() const { return stage_ != Stage::Idle; }

private:
    enum class Stage { Idle, Attack, Decay, Sustain, Release };

    float sampleRate_ = 48000.0f;
    float attack_ = 10.0f;
    float decay_ = 100.0f;
    float sustain_ = 0.7f;
    float release_ = 200.0f;

    Stage stage_ = Stage::Idle;
    float value_ = 0.0f;
    float releaseStart_ = 0.0f;
};

/**
 * Single synth voice for polyphony
 */
class SynthVoice {
public:
    void setSampleRate(float sr) {
        sampleRate_ = sr;
        envelope_.setSampleRate(sr);
    }

    void setType(SynthType type) { type_ = type; }

    void noteOn(int midiNote, float velocity = 1.0f) {
        midiNote_ = midiNote;
        frequency_ = 440.0f * std::pow(2.0f, (midiNote - 69) / 12.0f);
        velocity_ = velocity;
        phase_ = 0.0f;
        envelope_.trigger();

        // Set envelope based on synth type
        switch (type_) {
            case SynthType::ELECTRIC_PIANO:
                envelope_.setAttack(5.0f);
                envelope_.setDecay(300.0f);
                envelope_.setSustain(0.3f);
                envelope_.setRelease(400.0f);
                break;
            case SynthType::ORGAN:
                envelope_.setAttack(10.0f);
                envelope_.setDecay(50.0f);
                envelope_.setSustain(0.9f);
                envelope_.setRelease(100.0f);
                break;
            case SynthType::PAD:
                envelope_.setAttack(200.0f);
                envelope_.setDecay(500.0f);
                envelope_.setSustain(0.7f);
                envelope_.setRelease(800.0f);
                break;
        }
    }

    void noteOff() {
        envelope_.release();
    }

    float process() {
        if (!envelope_.isActive()) return 0.0f;

        float output = 0.0f;
        float env = envelope_.process();

        switch (type_) {
            case SynthType::ELECTRIC_PIANO:
                output = processElectricPiano();
                break;
            case SynthType::ORGAN:
                output = processOrgan();
                break;
            case SynthType::PAD:
                output = processPad();
                break;
        }

        // Advance phase
        phase_ += frequency_ / sampleRate_;
        if (phase_ >= 1.0f) phase_ -= 1.0f;

        return output * env * velocity_ * 0.3f;
    }

    bool isActive() const { return envelope_.isActive(); }
    int getMidiNote() const { return midiNote_; }

private:
    float processElectricPiano() {
        // Simple FM synthesis (2-operator)
        float modIndex = 2.0f * std::exp(-phase_ * 3.0f);  // Decaying modulation
        float modulator = std::sin(2.0f * static_cast<float>(M_PI) * phase_ * 2.0f);
        float carrier = std::sin(2.0f * static_cast<float>(M_PI) * phase_ + modulator * modIndex);
        return carrier;
    }

    float processOrgan() {
        // Additive synthesis (Hammond drawbars style)
        float p = 2.0f * static_cast<float>(M_PI) * phase_;
        float output = 0.0f;
        output += std::sin(p) * 0.8f;           // Fundamental (8')
        output += std::sin(p * 2.0f) * 0.5f;    // 4'
        output += std::sin(p * 3.0f) * 0.3f;    // 2 2/3'
        output += std::sin(p * 4.0f) * 0.2f;    // 2'
        output += std::sin(p * 0.5f) * 0.4f;    // 16' (sub)
        return output / 2.2f;
    }

    float processPad() {
        // Saw wave with soft filter
        float saw = 2.0f * phase_ - 1.0f;

        // Simple low-pass approximation
        float filtered = saw * 0.3f + lastOutput_ * 0.7f;
        lastOutput_ = filtered;

        return filtered;
    }

    float sampleRate_ = 48000.0f;
    float frequency_ = 440.0f;
    float phase_ = 0.0f;
    float velocity_ = 1.0f;
    int midiNote_ = 60;
    float lastOutput_ = 0.0f;

    SynthType type_ = SynthType::ELECTRIC_PIANO;
    ADSREnvelope envelope_;
};

/**
 * Polyphonic Chord Synthesizer
 * Plays chords with configurable voice count
 */
class ChordSynth {
public:
    static constexpr int MAX_VOICES = 8;

    void setSampleRate(float sr) {
        sampleRate_ = sr;
        for (auto& voice : voices_) {
            voice.setSampleRate(sr);
        }
    }

    void setType(SynthType type) {
        type_ = type;
        for (auto& voice : voices_) {
            voice.setType(type);
        }
    }

    /**
     * Play a chord
     */
    void playChord(const ChordSymbol& chord, int baseOctave = 3) {
        // Stop any playing notes first
        stopAll();

        // Get MIDI notes for the chord
        auto notes = chord.getMIDINotes(baseOctave);

        // Limit to available voices
        size_t numNotes = std::min(notes.size(), static_cast<size_t>(MAX_VOICES));

        for (size_t i = 0; i < numNotes; ++i) {
            voices_[i].setType(type_);
            voices_[i].noteOn(notes[i], 0.8f);
        }
    }

    /**
     * Play specific MIDI notes
     */
    void playNotes(const std::vector<int>& midiNotes, float velocity = 0.8f) {
        stopAll();

        size_t numNotes = std::min(midiNotes.size(), static_cast<size_t>(MAX_VOICES));
        for (size_t i = 0; i < numNotes; ++i) {
            voices_[i].setType(type_);
            voices_[i].noteOn(midiNotes[i], velocity);
        }
    }

    /**
     * Release all notes
     */
    void stopAll() {
        for (auto& voice : voices_) {
            voice.noteOff();
        }
    }

    /**
     * Process one sample (stereo output)
     */
    struct StereoOutput {
        float left;
        float right;
    };

    StereoOutput process() {
        float mix = 0.0f;

        for (auto& voice : voices_) {
            mix += voice.process();
        }

        // Simple stereo widening
        float mono = mix;

        return {
            std::tanh(mono * 0.8f),
            std::tanh(mono * 0.8f)
        };
    }

    /**
     * Check if any voice is still sounding
     */
    bool isPlaying() const {
        for (const auto& voice : voices_) {
            if (voice.isActive()) return true;
        }
        return false;
    }

private:
    std::array<SynthVoice, MAX_VOICES> voices_;
    float sampleRate_ = 48000.0f;
    SynthType type_ = SynthType::ELECTRIC_PIANO;
};

} // namespace JazzArchitect
