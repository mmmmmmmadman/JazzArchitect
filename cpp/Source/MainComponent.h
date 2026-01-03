#pragma once

#include <JuceHeader.h>
#include <map>
#include "Core/ChordSymbol.h"
#include "Grammar/Generator.h"
#include "Style/StyleEngine.h"
#include "Synthesis/ChordSynth.h"

class MainComponent : public juce::Component,
                      public juce::AudioIODeviceCallback,
                      public juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    // AudioIODeviceCallback
    void audioDeviceIOCallbackWithContext(
        const float* const* inputChannelData,
        int numInputChannels,
        float* const* outputChannelData,
        int numOutputChannels,
        int numSamples,
        const juce::AudioIODeviceCallbackContext& context) override;

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

    void paint(juce::Graphics& g) override;
    void paintOverChildren(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;

private:
    // Colors (Techno Machine style)
    juce::Colour bgDark_      { 0xff0e0c0c };
    juce::Colour bgMid_       { 0xff201a1a };
    juce::Colour accent_      { 0xffff9eb0 };
    juce::Colour accentDim_   { 0xffc08090 };
    juce::Colour textLight_   { 0xffffffff };
    juce::Colour textDim_     { 0xffc8b8b8 };

    // Audio
    juce::AudioDeviceManager deviceManager_;
    float sampleRate_ = 48000.0f;

    // UI Controls
    juce::ComboBox styleSelector_;
    juce::ComboBox keySelector_;
    juce::Slider bpmSlider_;
    juce::Slider lengthSlider_;

    juce::TextButton generateButton_{"Generate"};
    juce::TextButton playButton_{"Play"};
    juce::TextButton stopButton_{"Stop"};
    juce::TextButton exportButton_{"Export MIDI"};
    juce::TextButton importMidiButton_{"Import MIDI"};
    juce::TextButton importImageButton_{"Import Image"};

    juce::Label bpmLabel_{"", "BPM"};
    juce::Label lengthLabel_{"", "Length"};
    juce::Label styleLabel_{"", "Style"};
    juce::Label keyLabel_{"", "Key"};
    juce::Label statusLabel_;

    // Style parameter sliders (vertical)
    juce::Slider tritoneSubSlider_;
    juce::Slider iiVPrefSlider_;
    juce::Slider modalInterSlider_;
    juce::Slider extensionSlider_;

    juce::Label tritoneSubLabel_{"", "Tritone"};
    juce::Label iiVPrefLabel_{"", "ii-V"};
    juce::Label modalInterLabel_{"", "Modal"};
    juce::Label extensionLabel_{"", "Extension"};

    // Synth type selector
    juce::ComboBox synthSelector_;
    juce::Label synthLabel_{"", "Sound"};

    // Audio output device selector
    juce::ComboBox audioOutputSelector_;
    juce::Label audioOutputLabel_{"", "Audio Out"};

    // Display toggles for chord progression
    juce::ToggleButton showChordsToggle_{"Chords"};
    juce::ToggleButton showTrebleToggle_{"Treble"};
    juce::ToggleButton showBassToggle_{"Bass"};

    // Style engine (integrates grammar + style)
    std::unique_ptr<JazzArchitect::StyleEngine> styleEngine_;

    // Chord progression
    std::vector<JazzArchitect::ChordSymbol> currentProgression_;
    int currentChordIndex_ = 0;
    bool isPlaying_ = false;

    // Playback timing
    double playbackPosition_ = 0.0;
    double beatsPerChord_ = 2.0;

    // Synthesizer
    JazzArchitect::ChordSynth chordSynth_;
    int lastTriggeredChord_ = -1;

    // File chooser (for async export)
    std::unique_ptr<juce::FileChooser> fileChooser_;

    // Chord editing
    int editingChordIndex_ = -1;
    juce::TextEditor chordEditor_;

    // Note dragging
    int draggingChordIndex_ = -1;
    int draggingNoteIndex_ = -1;
    std::vector<int> draggedNotes_;

    // Chord duration resizing
    int resizingChordIndex_ = -1;    // Which chord is being resized (-1 = none)
    int resizeStartX_ = 0;           // Drag start X position
    double resizeStartDuration_ = 0; // Original duration before resize

    // Custom voicings (chord index -> custom MIDI notes)
    std::map<size_t, std::vector<int>> customVoicings_;

    // Preview sound auto-stop
    int previewStopCounter_ = 0;  // Countdown to stop preview sound

    // Button press glow effect
    int glowButtonIndex_ = -1;  // Which button is pressed (-1 = none)
    int glowCounter_ = 0;       // Countdown for glow fade

    // Hover help for sliders (0=none, 1=tritone, 2=iiV, 3=modal, 4=extension)
    int hoveredSlider_ = 0;

    // Chord box layout cache (updated in paint)
    struct ChordBoxInfo {
        int x, y, width, height;
        int trebleY, bassY, staffSpacing;
    };
    std::vector<ChordBoxInfo> chordBoxCache_;

    // Helper methods
    void styleButton(juce::TextButton& btn, juce::Colour textCol);
    void styleSlider(juce::Slider& slider);
    void styleLabel(juce::Label& label);
    void styleComboBox(juce::ComboBox& box);

    // Methods
    void generateProgression();
    void startPlayback();
    void stopPlayback();
    void updateUI();
    void initializeAudio();
    void exportMIDI();
    void importMIDI();
    void importImage();
    void updateAudioOutputDevices();
    void changeAudioOutputDevice();
    int midiToStaffY(int midiNote, int trebleY, int bassY, int spacing);
    int staffYToMidi(int y, int trebleY, int bassY, int spacing);
    void parseChordInput(const juce::String& input);
    int findChordAtPosition(int x, int y);
    int findNoteAtPosition(int chordIndex, int x, int y);
    void drawCircleOfFifths(juce::Graphics& g, int centerX, int centerY, int radius);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
