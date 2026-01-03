#include "MainComponent.h"
#include "MIDI/MIDIExporter.h"
#include "MIDI/MIDIImporter.h"

MainComponent::MainComponent()
{
    setSize(1000, 700);

    // Initialize style engine
    styleEngine_ = std::make_unique<JazzArchitect::StyleEngine>();

    // Initialize audio
    initializeAudio();

    // Style selector
    styleSelector_.addItem("Bebop", 1);
    styleSelector_.addItem("Cool", 2);
    styleSelector_.addItem("Modal", 3);
    styleSelector_.addItem("Hard Bop", 4);
    styleSelector_.addItem("Post-Bop", 5);
    styleSelector_.addItem("Swing", 6);
    styleSelector_.addItem("Fusion", 7);
    styleSelector_.addItem("Contemporary", 8);
    styleSelector_.addItem("Blues", 9);
    styleSelector_.setSelectedId(1);
    styleSelector_.onChange = [this] {
        // Update style engine and sliders based on preset
        int styleId = styleSelector_.getSelectedId();
        styleEngine_->setStyleById(styleId);

        // Update sliders to reflect the preset values
        const auto& style = styleEngine_->getStyle();
        tritoneSubSlider_.setValue(style.tritoneSubProb, juce::dontSendNotification);
        iiVPrefSlider_.setValue(style.iiVPreference, juce::dontSendNotification);
        modalInterSlider_.setValue(style.modalInterchange, juce::dontSendNotification);
        extensionSlider_.setValue(style.extensionLevel, juce::dontSendNotification);
    };
    styleComboBox(styleSelector_);
    addAndMakeVisible(styleSelector_);
    styleLabel(styleLabel_);
    addAndMakeVisible(styleLabel_);

    // Key selector
    const char* keys[] = {"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"};
    for (int i = 0; i < 12; ++i) {
        keySelector_.addItem(keys[i], i + 1);
    }
    keySelector_.setSelectedId(1);
    styleComboBox(keySelector_);
    addAndMakeVisible(keySelector_);
    styleLabel(keyLabel_);
    addAndMakeVisible(keyLabel_);

    // Synth type selector
    synthSelector_.addItem("E.Piano", 1);
    synthSelector_.addItem("Organ", 2);
    synthSelector_.addItem("Pad", 3);
    synthSelector_.setSelectedId(1);
    synthSelector_.onChange = [this] {
        int synthId = synthSelector_.getSelectedId();
        switch (synthId) {
            case 1: chordSynth_.setType(JazzArchitect::SynthType::ELECTRIC_PIANO); break;
            case 2: chordSynth_.setType(JazzArchitect::SynthType::ORGAN); break;
            case 3: chordSynth_.setType(JazzArchitect::SynthType::PAD); break;
        }
    };
    styleComboBox(synthSelector_);
    addAndMakeVisible(synthSelector_);
    styleLabel(synthLabel_);
    addAndMakeVisible(synthLabel_);

    // Audio output device selector
    audioOutputSelector_.onChange = [this] { changeAudioOutputDevice(); };
    styleComboBox(audioOutputSelector_);
    addAndMakeVisible(audioOutputSelector_);
    styleLabel(audioOutputLabel_);
    addAndMakeVisible(audioOutputLabel_);
    updateAudioOutputDevices();

    // BPM slider
    bpmSlider_.setRange(40, 200, 1);
    bpmSlider_.setValue(120);
    bpmSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 45, 18);
    styleSlider(bpmSlider_);
    addAndMakeVisible(bpmSlider_);
    styleLabel(bpmLabel_);
    bpmLabel_.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(bpmLabel_);

    // Length slider
    lengthSlider_.setRange(4, 32, 1);
    lengthSlider_.setValue(8);
    lengthSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 45, 18);
    styleSlider(lengthSlider_);
    addAndMakeVisible(lengthSlider_);
    styleLabel(lengthLabel_);
    lengthLabel_.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(lengthLabel_);

    // Buttons with glow effect
    generateButton_.onClick = [this] { glowButtonIndex_ = 0; glowCounter_ = 6; generateProgression(); };
    playButton_.onClick = [this] { glowButtonIndex_ = 1; glowCounter_ = 6; startPlayback(); };
    stopButton_.onClick = [this] { glowButtonIndex_ = 2; glowCounter_ = 6; stopPlayback(); };
    exportButton_.onClick = [this] { glowButtonIndex_ = 3; glowCounter_ = 6; exportMIDI(); };
    styleButton(generateButton_, accent_);
    styleButton(playButton_, accent_);
    styleButton(stopButton_, textLight_);
    styleButton(exportButton_, textDim_);
    importMidiButton_.onClick = [this] { glowButtonIndex_ = 7; glowCounter_ = 6; importMIDI(); };
    styleButton(importMidiButton_, textDim_);
    addAndMakeVisible(generateButton_);
    addAndMakeVisible(playButton_);
    addAndMakeVisible(stopButton_);
    addAndMakeVisible(exportButton_);
    addAndMakeVisible(importMidiButton_);

    // Style parameter sliders (vertical)
    auto setupVerticalSlider = [this](juce::Slider& slider, juce::Label& label, float defaultVal) {
        slider.setRange(0.0, 1.0, 0.01);
        slider.setValue(defaultVal);
        slider.setSliderStyle(juce::Slider::LinearVertical);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 36, 16);
        styleSlider(slider);
        addAndMakeVisible(slider);
        styleLabel(label);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
    };

    setupVerticalSlider(tritoneSubSlider_, tritoneSubLabel_, 0.3f);
    setupVerticalSlider(iiVPrefSlider_, iiVPrefLabel_, 0.9f);
    setupVerticalSlider(modalInterSlider_, modalInterLabel_, 0.2f);
    setupVerticalSlider(extensionSlider_, extensionLabel_, 0.5f);

    // Connect sliders to style engine for real-time updates
    tritoneSubSlider_.onValueChange = [this] {
        styleEngine_->setTritoneSubProb(static_cast<float>(tritoneSubSlider_.getValue()));
    };
    iiVPrefSlider_.onValueChange = [this] {
        styleEngine_->setIiVPreference(static_cast<float>(iiVPrefSlider_.getValue()));
    };
    modalInterSlider_.onValueChange = [this] {
        styleEngine_->setModalInterchange(static_cast<float>(modalInterSlider_.getValue()));
    };
    extensionSlider_.onValueChange = [this] {
        styleEngine_->setExtensionLevel(static_cast<float>(extensionSlider_.getValue()));
    };

    // Status label
    statusLabel_.setJustificationType(juce::Justification::centred);
    statusLabel_.setFont(juce::FontOptions(16.0f));
    statusLabel_.setColour(juce::Label::textColourId, textLight_);
    statusLabel_.setText("Ready", juce::dontSendNotification);
    addAndMakeVisible(statusLabel_);

    // Display toggles with glow effect
    showChordsToggle_.setToggleState(true, juce::dontSendNotification);
    showChordsToggle_.setColour(juce::ToggleButton::textColourId, textDim_);
    showChordsToggle_.setColour(juce::ToggleButton::tickColourId, accent_);
    showChordsToggle_.setColour(juce::ToggleButton::tickDisabledColourId, textDim_);
    showChordsToggle_.onClick = [this] { glowButtonIndex_ = 4; glowCounter_ = 6; repaint(); };
    addAndMakeVisible(showChordsToggle_);

    showTrebleToggle_.setToggleState(true, juce::dontSendNotification);
    showTrebleToggle_.setColour(juce::ToggleButton::textColourId, textDim_);
    showTrebleToggle_.setColour(juce::ToggleButton::tickColourId, accent_);
    showTrebleToggle_.setColour(juce::ToggleButton::tickDisabledColourId, textDim_);
    showTrebleToggle_.onClick = [this] { glowButtonIndex_ = 5; glowCounter_ = 6; repaint(); };
    addAndMakeVisible(showTrebleToggle_);

    showBassToggle_.setToggleState(true, juce::dontSendNotification);
    showBassToggle_.setColour(juce::ToggleButton::textColourId, textDim_);
    showBassToggle_.setColour(juce::ToggleButton::tickColourId, accent_);
    showBassToggle_.setColour(juce::ToggleButton::tickDisabledColourId, textDim_);
    showBassToggle_.onClick = [this] { glowButtonIndex_ = 6; glowCounter_ = 6; repaint(); };
    addAndMakeVisible(showBassToggle_);

    // Chord editor (hidden by default)
    chordEditor_.setColour(juce::TextEditor::backgroundColourId, bgDark_);
    chordEditor_.setColour(juce::TextEditor::textColourId, textLight_);
    chordEditor_.setColour(juce::TextEditor::outlineColourId, accent_);
    chordEditor_.setColour(juce::TextEditor::focusedOutlineColourId, accent_);
    chordEditor_.setFont(juce::FontOptions(14.0f));
    chordEditor_.setJustification(juce::Justification::centred);
    chordEditor_.setVisible(false);
    chordEditor_.onReturnKey = [this] {
        parseChordInput(chordEditor_.getText());
        chordEditor_.setVisible(false);
        editingChordIndex_ = -1;
        repaint();
    };
    chordEditor_.onFocusLost = [this] {
        chordEditor_.setVisible(false);
        editingChordIndex_ = -1;
        repaint();
    };
    addChildComponent(chordEditor_);

    // Staff notation is now drawn directly in paint()

    // Initial progression
    generateProgression();

    // Start timer for UI updates (30Hz like Techno Machine)
    startTimerHz(30);
}

MainComponent::~MainComponent()
{
    stopTimer();
    deviceManager_.removeAudioCallback(this);
}

void MainComponent::styleButton(juce::TextButton& btn, juce::Colour textCol)
{
    btn.setColour(juce::TextButton::buttonColourId, bgMid_);
    btn.setColour(juce::TextButton::buttonOnColourId, bgMid_.brighter(0.1f));
    btn.setColour(juce::TextButton::textColourOffId, textCol);
    btn.setColour(juce::TextButton::textColourOnId, textCol.brighter(0.2f));
}

void MainComponent::styleSlider(juce::Slider& slider)
{
    slider.setColour(juce::Slider::backgroundColourId, bgMid_);
    slider.setColour(juce::Slider::trackColourId, accentDim_);
    slider.setColour(juce::Slider::thumbColourId, accent_);
    slider.setColour(juce::Slider::textBoxTextColourId, textLight_);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, bgDark_);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

void MainComponent::styleLabel(juce::Label& label)
{
    label.setColour(juce::Label::textColourId, textDim_);
    label.setFont(juce::FontOptions(14.0f));
}

void MainComponent::styleComboBox(juce::ComboBox& box)
{
    box.setColour(juce::ComboBox::backgroundColourId, bgMid_);
    box.setColour(juce::ComboBox::textColourId, textLight_);
    box.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    box.setColour(juce::ComboBox::arrowColourId, accentDim_);
}

void MainComponent::initializeAudio()
{
    auto result = deviceManager_.initialiseWithDefaultDevices(0, 2);
    if (result.isNotEmpty()) {
        statusLabel_.setText("Audio Error: " + result, juce::dontSendNotification);
    }
    deviceManager_.addAudioCallback(this);
}

void MainComponent::updateAudioOutputDevices()
{
    audioOutputSelector_.clear();

    auto* currentDeviceType = deviceManager_.getCurrentDeviceTypeObject();
    if (currentDeviceType == nullptr)
        return;

    auto deviceNames = currentDeviceType->getDeviceNames(false);  // false = output devices

    for (int i = 0; i < deviceNames.size(); ++i) {
        audioOutputSelector_.addItem(deviceNames[i], i + 1);
    }

    // Select current device
    auto* currentDevice = deviceManager_.getCurrentAudioDevice();
    if (currentDevice != nullptr) {
        auto currentName = currentDevice->getName();
        int index = deviceNames.indexOf(currentName);
        if (index >= 0) {
            audioOutputSelector_.setSelectedId(index + 1, juce::dontSendNotification);
        }
    }
}

void MainComponent::changeAudioOutputDevice()
{
    int selectedIndex = audioOutputSelector_.getSelectedId() - 1;
    if (selectedIndex < 0)
        return;

    auto* currentDeviceType = deviceManager_.getCurrentDeviceTypeObject();
    if (currentDeviceType == nullptr)
        return;

    auto deviceNames = currentDeviceType->getDeviceNames(false);
    if (selectedIndex >= deviceNames.size())
        return;

    auto selectedDeviceName = deviceNames[selectedIndex];

    // Setup for the new device
    juce::AudioDeviceManager::AudioDeviceSetup setup;
    deviceManager_.getAudioDeviceSetup(setup);
    setup.outputDeviceName = selectedDeviceName;

    // Apply the setup
    auto result = deviceManager_.setAudioDeviceSetup(setup, true);

    if (result.isNotEmpty()) {
        statusLabel_.setText("Device Error: " + result, juce::dontSendNotification);
    } else {
        statusLabel_.setText("Audio Output: " + selectedDeviceName, juce::dontSendNotification);
    }
}

void MainComponent::exportMIDI()
{
    if (currentProgression_.empty()) {
        statusLabel_.setText("No progression to export", juce::dontSendNotification);
        return;
    }

    // Create file chooser
    auto chooser = std::make_unique<juce::FileChooser>(
        "Export MIDI File",
        juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
            .getChildFile("JazzArchitect.mid"),
        "*.mid");

    auto chooserFlags = juce::FileBrowserComponent::saveMode |
                        juce::FileBrowserComponent::canSelectFiles |
                        juce::FileBrowserComponent::warnAboutOverwriting;

    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file == juce::File{}) {
            return;  // User cancelled
        }

        double bpm = bpmSlider_.getValue();
        bool success = JazzArchitect::MIDIExporter::exportToFile(
            currentProgression_,
            file,
            bpm,
            beatsPerChord_,
            3  // base octave
        );

        if (success) {
            statusLabel_.setText("Exported: " + file.getFileName(), juce::dontSendNotification);
        } else {
            statusLabel_.setText("Export failed!", juce::dontSendNotification);
        }
    });

    // Keep chooser alive
    fileChooser_ = std::move(chooser);
}

void MainComponent::importMIDI()
{
    // Create file chooser
    auto chooser = std::make_unique<juce::FileChooser>(
        "Import MIDI File",
        juce::File::getSpecialLocation(juce::File::userDesktopDirectory),
        "*.mid;*.midi");

    auto chooserFlags = juce::FileBrowserComponent::openMode |
                        juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file == juce::File{}) {
            return;  // User cancelled
        }

        auto result = JazzArchitect::MIDIImporter::importFromFile(file);

        if (result.success) {
            currentProgression_ = result.chords;
            customVoicings_.clear();
            currentChordIndex_ = 0;
            playbackPosition_ = 0.0;

            // Update BPM slider if tempo was found
            if (result.bpm > 30.0 && result.bpm < 250.0) {
                bpmSlider_.setValue(result.bpm, juce::dontSendNotification);
            }

            statusLabel_.setText("Imported " + juce::String(static_cast<int>(result.chords.size())) +
                                " chords from " + file.getFileName(),
                                juce::dontSendNotification);
            repaint();
        } else {
            statusLabel_.setText("Import failed: " + juce::String(result.errorMessage),
                                juce::dontSendNotification);
        }
    });

    // Keep chooser alive
    fileChooser_ = std::move(chooser);
}

int MainComponent::midiToStaffY(int midiNote, int trebleY, int bassY, int spacing)
{
    // Note positions within octave: C=0, D=1, E=2, F=3, G=4, A=5, B=6
    static const int notePositions[] = {0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6};

    int octave = midiNote / 12 - 1;
    int pitchClass = midiNote % 12;
    int notePos = notePositions[pitchClass];

    // B4 (MIDI 71) is on the middle line of treble staff
    int b4Position = trebleY + 2 * spacing;
    int stepsFromB4 = (octave - 4) * 7 + notePos - 6;
    int y = b4Position - stepsFromB4 * spacing / 2;

    return y;
}

void MainComponent::generateProgression()
{
    currentProgression_.clear();
    customVoicings_.clear();  // Clear custom voicings when generating new progression

    int length = static_cast<int>(lengthSlider_.getValue());
    int keyIdx = keySelector_.getSelectedId() - 1;

    // Generate using StyleEngine (style-aware PCFG)
    currentProgression_ = styleEngine_->generate(length, keyIdx);

    currentChordIndex_ = 0;

    // Get style name for status
    int styleId = styleSelector_.getSelectedId();
    juce::String styleName = juce::String(JazzArchitect::StylePresets::getStyleName(styleId));

    statusLabel_.setText("Generated " + juce::String(static_cast<int>(currentProgression_.size())) +
                         " chords (" + styleName + ")",
                         juce::dontSendNotification);

    repaint();
}

void MainComponent::startPlayback()
{
    isPlaying_ = true;
    currentChordIndex_ = 0;
    playbackPosition_ = 0.0;
    lastTriggeredChord_ = -1;  // Force re-trigger
    playButton_.setEnabled(false);
    stopButton_.setEnabled(true);
    repaint();  // Highlight first chord immediately
}

void MainComponent::stopPlayback()
{
    isPlaying_ = false;
    lastTriggeredChord_ = -1;
    chordSynth_.stopAll();
    playButton_.setEnabled(true);
    stopButton_.setEnabled(false);
    repaint();
}

void MainComponent::updateUI()
{
    if (isPlaying_ && !currentProgression_.empty()) {
        double bpm = bpmSlider_.getValue();
        double beatsPerSecond = bpm / 60.0;
        double secondsPerUpdate = 1.0 / 30.0;

        playbackPosition_ += beatsPerSecond * secondsPerUpdate;

        // Calculate total duration and find current chord using individual durations
        double totalBeats = 0.0;
        for (const auto& chord : currentProgression_) {
            totalBeats += chord.getDuration();
        }

        // Loop playback
        if (playbackPosition_ >= totalBeats) {
            playbackPosition_ = 0.0;
        }

        // Find current chord index based on accumulated durations
        double accumulatedBeats = 0.0;
        int newChordIndex = 0;
        for (size_t i = 0; i < currentProgression_.size(); ++i) {
            double chordDuration = currentProgression_[i].getDuration();
            if (playbackPosition_ < accumulatedBeats + chordDuration) {
                newChordIndex = static_cast<int>(i);
                break;
            }
            accumulatedBeats += chordDuration;
        }

        if (newChordIndex != currentChordIndex_) {
            currentChordIndex_ = newChordIndex;
            repaint();
        }

        int bar = static_cast<int>(playbackPosition_ / 4.0) + 1;
        int beat = static_cast<int>(playbackPosition_) % 4 + 1;
        statusLabel_.setText(juce::String::formatted("Bar %d | Beat %d | %.0f BPM", bar, beat, bpm),
                             juce::dontSendNotification);
    }
}

void MainComponent::timerCallback()
{
    updateUI();

    // Auto-stop preview sound
    if (previewStopCounter_ > 0) {
        previewStopCounter_--;
        if (previewStopCounter_ == 0) {
            chordSynth_.stopAll();
        }
    }

    // Button glow fade
    if (glowCounter_ > 0) {
        glowCounter_--;
        if (glowCounter_ == 0) {
            glowButtonIndex_ = -1;
        }
    }

    // Repaint for hover help updates
    repaint();
}

void MainComponent::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    sampleRate_ = static_cast<float>(device->getCurrentSampleRate());
    chordSynth_.setSampleRate(sampleRate_);
    chordSynth_.setType(JazzArchitect::SynthType::ELECTRIC_PIANO);
}

void MainComponent::audioDeviceStopped()
{
}

void MainComponent::audioDeviceIOCallbackWithContext(
    const float* const*,
    int,
    float* const* outputChannelData,
    int numOutputChannels,
    int numSamples,
    const juce::AudioIODeviceCallbackContext&)
{
    // Trigger chord if needed
    if (isPlaying_ && !currentProgression_.empty()) {
        if (currentChordIndex_ != lastTriggeredChord_) {
            lastTriggeredChord_ = currentChordIndex_;
            chordSynth_.playChord(currentProgression_[currentChordIndex_], 3);
        }
    } else if (!isPlaying_ && lastTriggeredChord_ != -1) {
        chordSynth_.stopAll();
        lastTriggeredChord_ = -1;
    }

    // Process audio
    for (int i = 0; i < numSamples; ++i) {
        auto output = chordSynth_.process();

        if (numOutputChannels >= 1 && outputChannelData[0] != nullptr) {
            outputChannelData[0][i] = output.left;
        }
        if (numOutputChannels >= 2 && outputChannelData[1] != nullptr) {
            outputChannelData[1][i] = output.right;
        }
    }
}

void MainComponent::paint(juce::Graphics& g)
{
    // Clear chord box cache
    chordBoxCache_.clear();
    chordBoxCache_.resize(currentProgression_.size());

    // Dark warm background
    g.fillAll(bgDark_);

    // Title - vibrant pink, 32pt
    g.setColour(accent_);
    g.setFont(juce::FontOptions(32.0f).withStyle("Bold"));
    g.drawText("JAZZ ARCHITECT", getLocalBounds().removeFromTop(50), juce::Justification::centred);

    // Thin separator line - 往下移動以給控制項更多空間
    g.setColour(juce::Colour(0xff302828));
    g.drawLine(20, 165, static_cast<float>(getWidth() - 20), 165, 0.5f);

    // Draw chord progression with staff notation
    if (!currentProgression_.empty()) {
        // Get toggle states
        bool showChords = showChordsToggle_.getToggleState();
        bool showTreble = showTrebleToggle_.getToggleState();
        bool showBass = showBassToggle_.getToggleState();

        // Count visible sections for spacing calculation
        int visibleSections = (showChords ? 1 : 0) + (showTreble ? 1 : 0) + (showBass ? 1 : 0);
        if (visibleSections == 0) visibleSections = 1;  // Prevent division by zero

        // Base sizes
        int boxHeight = 55;
        int staffLineSpacing = 10;
        int singleStaffHeight = 4 * staffLineSpacing;
        int sectionGap = 25;  // Gap between sections (+10px)

        // Calculate row height based on visible sections
        int rowHeight = sectionGap;  // Start with gap
        if (showChords) rowHeight += boxHeight + sectionGap;
        if (showTreble) rowHeight += singleStaffHeight + sectionGap;
        if (showBass) rowHeight += singleStaffHeight + sectionGap;

        // Extra spacing when fewer sections are visible
        float spacingMultiplier = 1.0f + (3 - visibleSections) * 0.15f;
        rowHeight = static_cast<int>(rowHeight * spacingMultiplier);

        int baseY = 200;
        int startX = 25;

        // Calculate available width for chords
        int sliderWidth = 55;
        int sliderSpacing = 8;
        int slidersStartX = getWidth() - 20 - (4 * sliderWidth + 3 * sliderSpacing);
        int availableWidth = slidersStartX - startX - 30;

        // Dynamically calculate chords per row
        int minBoxWidth = 80;
        int minSpacing = 8;
        int chordsPerRow = availableWidth / (minBoxWidth + minSpacing);
        if (chordsPerRow < 1) chordsPerRow = 1;
        if (chordsPerRow > 8) chordsPerRow = 8;

        // Distribute space evenly with 5% extra spacing
        int totalSpacing = static_cast<int>((availableWidth - chordsPerRow * minBoxWidth) * 1.05f);
        int spacing = chordsPerRow > 1 ? totalSpacing / (chordsPerRow - 1) : 0;
        int boxWidth = minBoxWidth;

        // Recalculate to use full width
        int totalUsed = chordsPerRow * boxWidth + (chordsPerRow - 1) * spacing;
        int extraPadding = (availableWidth - totalUsed) / 2;
        startX += extraPadding;

        int maxChordX = slidersStartX - 20;

        g.setColour(textDim_);
        g.setFont(juce::FontOptions(14.0f));
        g.drawText("CHORD PROGRESSION", 20, 180, 200, 16, juce::Justification::left);

        int numRows = (static_cast<int>(currentProgression_.size()) + chordsPerRow - 1) / chordsPerRow;

        for (int row = 0; row < numRows; ++row) {
            int currentY = baseY + row * rowHeight;

            // Track Y positions for each section
            int chordBoxY = currentY;
            int trebleY = currentY;
            int bassY = currentY;

            // Calculate Y positions based on what's visible
            if (showChords) {
                chordBoxY = currentY;
                currentY += boxHeight + sectionGap;
            }
            if (showTreble) {
                trebleY = currentY;
                currentY += singleStaffHeight + sectionGap;
            }
            if (showBass) {
                bassY = currentY;
            }

            // Draw treble staff
            if (showTreble) {
                g.setColour(textDim_.withAlpha(0.6f));
                for (int line = 0; line < 5; ++line) {
                    g.drawLine(20.0f, static_cast<float>(trebleY + line * staffLineSpacing),
                              static_cast<float>(maxChordX), static_cast<float>(trebleY + line * staffLineSpacing), 0.5f);
                }
                g.setColour(textDim_);
                g.setFont(juce::FontOptions(20.0f));
                g.drawText("G", 5, trebleY - 2, 15, 22, juce::Justification::centred);
            }

            // Draw bass staff
            if (showBass) {
                g.setColour(textDim_.withAlpha(0.6f));
                for (int line = 0; line < 5; ++line) {
                    g.drawLine(20.0f, static_cast<float>(bassY + line * staffLineSpacing),
                              static_cast<float>(maxChordX), static_cast<float>(bassY + line * staffLineSpacing), 0.5f);
                }
                g.setColour(textDim_);
                g.setFont(juce::FontOptions(20.0f));
                g.drawText("F", 5, bassY, 15, 22, juce::Justification::centred);
            }

            // Calculate how many chords in this row
            int chordsInThisRow = std::min(chordsPerRow, static_cast<int>(currentProgression_.size()) - row * chordsPerRow);

            // Calculate total duration in this row for proportional widths
            double totalRowDuration = 0.0;
            for (int col = 0; col < chordsInThisRow; ++col) {
                size_t idx = static_cast<size_t>(row * chordsPerRow + col);
                if (idx < currentProgression_.size()) {
                    totalRowDuration += currentProgression_[idx].getDuration();
                }
            }

            // Draw chords and notes for this row
            int currentX = startX;
            for (int col = 0; col < chordsInThisRow; ++col) {
                size_t idx = static_cast<size_t>(row * chordsPerRow + col);
                if (idx >= currentProgression_.size()) break;

                // Calculate width proportionally based on duration
                double chordDuration = currentProgression_[idx].getDuration();
                int totalSpaceForChords = availableWidth - (chordsInThisRow - 1) * spacing;
                int thisBoxWidth = static_cast<int>((chordDuration / totalRowDuration) * totalSpaceForChords);
                thisBoxWidth = std::max(60, std::min(200, thisBoxWidth));  // Clamp width

                int x = currentX;

                // Update chord box cache
                if (idx < chordBoxCache_.size()) {
                    chordBoxCache_[idx] = { x, chordBoxY, thisBoxWidth, boxHeight, trebleY, bassY, staffLineSpacing };
                }

                // Draw chord box
                if (showChords) {
                    // Check if this chord box is being clicked
                    auto mousePos = getMouseXYRelative();
                    bool isClicking = juce::ModifierKeys::currentModifiers.isLeftButtonDown() &&
                                      mousePos.x >= x && mousePos.x <= x + thisBoxWidth &&
                                      mousePos.y >= chordBoxY && mousePos.y <= chordBoxY + boxHeight;

                    // Draw click glow effect
                    if (isClicking) {
                        g.setColour(accent_.withAlpha(0.4f));
                        g.drawRoundedRectangle(static_cast<float>(x - 4), static_cast<float>(chordBoxY - 4),
                                               static_cast<float>(thisBoxWidth + 8), static_cast<float>(boxHeight + 8), 8.0f, 3.0f);
                        g.setColour(accent_.withAlpha(0.2f));
                        g.drawRoundedRectangle(static_cast<float>(x - 6), static_cast<float>(chordBoxY - 6),
                                               static_cast<float>(thisBoxWidth + 12), static_cast<float>(boxHeight + 12), 10.0f, 2.0f);
                    }

                    if (static_cast<int>(idx) == currentChordIndex_ && isPlaying_) {
                        g.setColour(accent_.withAlpha(0.3f));
                    } else {
                        g.setColour(bgMid_);
                    }
                    g.fillRoundedRectangle(static_cast<float>(x), static_cast<float>(chordBoxY),
                                           static_cast<float>(thisBoxWidth), static_cast<float>(boxHeight), 4.0f);

                    if (static_cast<int>(idx) == currentChordIndex_ && isPlaying_) {
                        g.setColour(accent_);
                        g.drawRoundedRectangle(static_cast<float>(x), static_cast<float>(chordBoxY),
                                               static_cast<float>(thisBoxWidth), static_cast<float>(boxHeight), 4.0f, 2.0f);
                    }

                    g.setColour(textLight_);
                    g.setFont(juce::FontOptions(16.0f).withStyle("Bold"));
                    g.drawText(currentProgression_[idx].toString(), x, chordBoxY + 8, thisBoxWidth, 20, juce::Justification::centred);

                    g.setFont(juce::FontOptions(11.0f));
                    g.setColour(accentDim_);
                    JazzArchitect::PitchClass key(keySelector_.getSelectedId() - 1);
                    g.drawText(currentProgression_[idx].asRomanNumeral(key), x, chordBoxY + 32, thisBoxWidth, 16, juce::Justification::centred);

                    // Draw duration indicator (bottom right corner)
                    g.setFont(juce::FontOptions(10.0f));
                    g.setColour(textDim_.withAlpha(0.7f));
                    juce::String durationStr = juce::String(chordDuration, 1);
                    if (durationStr.endsWith(".0")) durationStr = juce::String(static_cast<int>(chordDuration));
                    g.drawText(durationStr, x + thisBoxWidth - 22, chordBoxY + boxHeight - 14, 18, 12, juce::Justification::right);
                }

                // Draw notes on staff
                if (showTreble || showBass) {
                    // Use dragged notes, custom voicing, or original notes
                    std::vector<int> midiNotes;
                    if (draggingChordIndex_ == static_cast<int>(idx) && !draggedNotes_.empty()) {
                        midiNotes = draggedNotes_;
                    } else if (customVoicings_.count(idx) > 0) {
                        midiNotes = customVoicings_.at(idx);
                    } else {
                        midiNotes = currentProgression_[idx].getMIDINotes(4);
                    }

                    // Find the lowest note (bass note)
                    int lowestNote = midiNotes.empty() ? 0 : *std::min_element(midiNotes.begin(), midiNotes.end());

                    juce::Colour noteColor = (static_cast<int>(idx) == currentChordIndex_ && isPlaying_) ? accent_ : accentDim_;

                    int noteCenterX = x + thisBoxWidth / 2;
                    for (size_t noteIdx = 0; noteIdx < midiNotes.size(); ++noteIdx) {
                        int midi = midiNotes[noteIdx];
                        int noteY = midiToStaffY(midi, trebleY, bassY, staffLineSpacing);
                        bool isTrebleNote = (midi >= 60);  // Middle C and above

                        // Only draw if the corresponding staff is visible
                        if ((isTrebleNote && showTreble) || (!isTrebleNote && showBass)) {
                            // Highlight dragging note or lowest note (bass)
                            if (draggingChordIndex_ == static_cast<int>(idx) &&
                                draggingNoteIndex_ == static_cast<int>(noteIdx)) {
                                g.setColour(accent_);
                            } else if (midi == lowestNote) {
                                g.setColour(textLight_);  // Bright white for bass note
                            } else {
                                g.setColour(noteColor);
                            }

                            // Draw accidental if needed (black keys: 1,3,6,8,10)
                            int pitchClass = midi % 12;
                            bool isSharp = (pitchClass == 1 || pitchClass == 3 || pitchClass == 6 ||
                                           pitchClass == 8 || pitchClass == 10);
                            if (isSharp) {
                                // Draw sharp symbol (simplified #)
                                g.setFont(juce::FontOptions(12.0f));
                                g.drawText("#", noteCenterX - 14, noteY - 6, 10, 12, juce::Justification::centred);
                            }

                            // Note head: 9x8 ratio (slightly horizontal, more like standard notation)
                            g.drawEllipse(static_cast<float>(noteCenterX - 5), static_cast<float>(noteY - 4), 9.0f, 8.0f, 1.5f);

                            if (midi == 60 && showTreble) {
                                g.setColour(textDim_.withAlpha(0.6f));
                                g.drawLine(static_cast<float>(noteCenterX - 8), static_cast<float>(noteY),
                                          static_cast<float>(noteCenterX + 8), static_cast<float>(noteY), 0.5f);
                            }
                        }
                    }
                }

                // Advance X position for next chord
                currentX += thisBoxWidth + spacing;
            }
        }
    }

    // Style parameters label - 增加與分隔線間距
    g.setColour(textDim_);
    g.setFont(juce::FontOptions(12.0f));
    g.drawText("STYLE PARAMETERS", getWidth() - 290, 180, 280, 16, juce::Justification::left);  // 165 -> 180 增加間距

    // Circle of Fifths - below the sliders
    int sliderWidth = 55;
    int sliderSpacing = 8;
    int slidersStartX = getWidth() - 20 - (4 * sliderWidth + 3 * sliderSpacing);
    int circleRadius = 70;
    int circleCenterX = slidersStartX + (4 * sliderWidth + 3 * sliderSpacing) / 2;
    int circleCenterY = 430;  // Below sliders (205 + 22 + 130 + padding)
    drawCircleOfFifths(g, circleCenterX, circleCenterY, circleRadius);

    // Hover help text for all controls (below Circle of Fifths)
    int helpY = circleCenterY + circleRadius + 15;
    int helpWidth = 4 * sliderWidth + 3 * sliderSpacing;
    int helpX = slidersStartX;

    juce::String helpTextEN, helpTextJA, helpTextZH;

    // Style parameter sliders
    if (tritoneSubSlider_.isMouseOver()) {
        helpTextEN = "Tritone Sub: Replace V7 with bII7. Both share the same tritone interval, enabling smooth chromatic voice leading to tonic.";
        helpTextJA = juce::String::fromUTF8(u8"トライトーン代理: V7をbII7に置換。同じ増4度を共有し、滑らかな半音進行を生む。");
        helpTextZH = juce::String::fromUTF8(u8"三全音替代: 將 V7 替換為 bII7。兩者共享相同的增四度，創造流暢的半音聲部導進。");
    } else if (iiVPrefSlider_.isMouseOver()) {
        helpTextEN = "ii-V Pref: The backbone of jazz harmony. Higher values favor ii7-V7-I cadences over direct V7-I resolutions.";
        helpTextJA = juce::String::fromUTF8(u8"ii-V傾向: ジャズ和声の基礎。高い値はV7-I直接解決よりii7-V7-Iカデンツを優先。");
        helpTextZH = juce::String::fromUTF8(u8"ii-V 偏好: 爵士和聲的骨幹。較高的值會優先使用 ii7-V7-I 終止式，而非直接 V7-I。");
    } else if (modalInterSlider_.isMouseOver()) {
        helpTextEN = "Modal: Borrow chords from parallel modes. Examples: bVII (Mixolydian), iv (Dorian), bVI (Aeolian).";
        helpTextJA = juce::String::fromUTF8(u8"モーダル: 平行調から和音を借用。例: bVII(Mixo), iv(Dorian), bVI(Aeolian)。");
        helpTextZH = juce::String::fromUTF8(u8"調式交換: 從平行調式借用和弦。例: bVII(Mixo), iv(Dorian), bVI(Aeolian)。");
    } else if (extensionSlider_.isMouseOver()) {
        helpTextEN = "Extension: Control chord complexity. Low: triads/7ths. Medium: add 9ths. High: 11ths & 13ths for rich colors.";
        helpTextJA = juce::String::fromUTF8(u8"テンション: 和音の複雑さを制御。低:3和音/7th。中:9th追加。高:11th,13thで豊かな響き。");
        helpTextZH = juce::String::fromUTF8(u8"延伸音: 控制和弦複雜度。低:三和弦/七和弦。中:加入9度。高:11、13度創造豐富色彩。");
    }
    // Buttons
    else if (generateButton_.isMouseOver()) {
        helpTextEN = "Generate: Create a new chord progression based on the selected style and parameters.";
        helpTextJA = juce::String::fromUTF8(u8"生成: 選択したスタイルとパラメータに基づいて新しいコード進行を作成。");
        helpTextZH = juce::String::fromUTF8(u8"生成: 根據選定的風格和參數創建新的和弦進行。");
    } else if (playButton_.isMouseOver()) {
        helpTextEN = "Play: Start playback of the current chord progression with the built-in synthesizer.";
        helpTextJA = juce::String::fromUTF8(u8"再生: 内蔵シンセサイザーで現在のコード進行を再生開始。");
        helpTextZH = juce::String::fromUTF8(u8"播放: 使用內建合成器開始播放當前和弦進行。");
    } else if (stopButton_.isMouseOver()) {
        helpTextEN = "Stop: Stop the current playback immediately.";
        helpTextJA = juce::String::fromUTF8(u8"停止: 現在の再生をすぐに停止。");
        helpTextZH = juce::String::fromUTF8(u8"停止: 立即停止當前播放。");
    } else if (exportButton_.isMouseOver()) {
        helpTextEN = "Export MIDI: Save the current chord progression as a Standard MIDI File (.mid).";
        helpTextJA = juce::String::fromUTF8(u8"MIDI出力: 現在のコード進行をMIDIファイル(.mid)として保存。");
        helpTextZH = juce::String::fromUTF8(u8"匯出 MIDI: 將當前和弦進行儲存為標準 MIDI 檔案 (.mid)。");
    }
    // Selectors
    else if (styleSelector_.isMouseOver()) {
        helpTextEN = "Style: Choose a jazz style preset. Each style has different harmonic preferences and substitution tendencies.";
        helpTextJA = juce::String::fromUTF8(u8"スタイル: ジャズスタイルを選択。各スタイルは異なる和声傾向を持つ。");
        helpTextZH = juce::String::fromUTF8(u8"風格: 選擇爵士風格預設。每種風格有不同的和聲偏好與替代傾向。");
    } else if (keySelector_.isMouseOver()) {
        helpTextEN = "Key: Set the tonic key for the chord progression. All chords will be generated relative to this key.";
        helpTextJA = juce::String::fromUTF8(u8"キー: コード進行の調を設定。全てのコードはこの調に基づいて生成。");
        helpTextZH = juce::String::fromUTF8(u8"調性: 設定和弦進行的主調。所有和弦將根據此調生成。");
    } else if (synthSelector_.isMouseOver()) {
        helpTextEN = "Sound: Select the synthesizer timbre. E.Piano (Rhodes-like), Organ, or Pad for different textures.";
        helpTextJA = juce::String::fromUTF8(u8"音色: シンセサイザーの音色を選択。E.Piano、Organ、Padから選べる。");
        helpTextZH = juce::String::fromUTF8(u8"音色: 選擇合成器音色。E.Piano (類 Rhodes)、Organ 或 Pad。");
    } else if (audioOutputSelector_.isMouseOver()) {
        helpTextEN = "Audio Out: Select the audio output device for playback.";
        helpTextJA = juce::String::fromUTF8(u8"オーディオ出力: 再生用のオーディオ出力デバイスを選択。");
        helpTextZH = juce::String::fromUTF8(u8"音訊輸出: 選擇播放用的音訊輸出裝置。");
    }
    // Other sliders
    else if (bpmSlider_.isMouseOver()) {
        helpTextEN = "BPM: Set the tempo (beats per minute) for playback. Range: 40-200 BPM.";
        helpTextJA = juce::String::fromUTF8(u8"BPM: 再生テンポを設定。範囲: 40-200 BPM。");
        helpTextZH = juce::String::fromUTF8(u8"BPM: 設定播放速度 (每分鐘拍數)。範圍: 40-200 BPM。");
    } else if (lengthSlider_.isMouseOver()) {
        helpTextEN = "Length: Set the number of chords to generate. Range: 4-32 chords.";
        helpTextJA = juce::String::fromUTF8(u8"長さ: 生成するコード数を設定。範囲: 4-32コード。");
        helpTextZH = juce::String::fromUTF8(u8"長度: 設定生成的和弦數量。範圍: 4-32 個和弦。");
    }
    // Display toggles
    else if (showChordsToggle_.isMouseOver()) {
        helpTextEN = "Chords: Toggle display of chord symbol boxes (chord name and Roman numeral).";
        helpTextJA = juce::String::fromUTF8(u8"コード: コードシンボル表示の切り替え（コード名とローマ数字）。");
        helpTextZH = juce::String::fromUTF8(u8"和弦: 切換和弦符號方塊顯示 (和弦名稱與羅馬數字)。");
    } else if (showTrebleToggle_.isMouseOver()) {
        helpTextEN = "Treble: Toggle display of treble clef staff showing upper voicing notes.";
        helpTextJA = juce::String::fromUTF8(u8"高音部: 高音部譜表の表示切り替え（上声部の音符）。");
        helpTextZH = juce::String::fromUTF8(u8"高音譜: 切換高音譜表顯示 (上聲部音符)。");
    } else if (showBassToggle_.isMouseOver()) {
        helpTextEN = "Bass: Toggle display of bass clef staff showing bass notes.";
        helpTextJA = juce::String::fromUTF8(u8"低音部: 低音部譜表の表示切り替え（ベース音）。");
        helpTextZH = juce::String::fromUTF8(u8"低音譜: 切換低音譜表顯示 (低音音符)。");
    }
    // Chord boxes - check if mouse is over any chord box
    else {
        auto mousePos = getMouseXYRelative();
        int hoveredChord = findChordAtPosition(mousePos.x, mousePos.y);
        if (hoveredChord >= 0) {
            helpTextEN = "Chord Box: Click upper half to edit chord name. Click lower half to preview sound. Drag notes on staff to adjust voicing.";
            helpTextJA = juce::String::fromUTF8(u8"コードボックス: 上半分クリックでコード名編集。下半分でプレビュー。譜面上の音符をドラッグでボイシング調整。");
            helpTextZH = juce::String::fromUTF8(u8"和弦方塊: 點擊上半部編輯和弦名稱。點擊下半部預聽。拖曳譜表上的音符調整配置。");
        }
    }

    if (helpTextEN.isNotEmpty()) {
        // Draw background for help text area - use original width, wrap text vertically
        int textAreaX = slidersStartX;
        int textAreaWidth = helpWidth;
        int lineHeight = 42;  // Two lines per language at 16pt + 2px spacing

        g.setColour(bgMid_.withAlpha(0.95f));
        int totalHeight = lineHeight * 3 + 4 + 16;  // 3 blocks + 2px*2 spacing + padding
        g.fillRoundedRectangle(static_cast<float>(textAreaX - 5), static_cast<float>(helpY - 8),
                               static_cast<float>(textAreaWidth + 10), static_cast<float>(totalHeight), 6.0f);

        g.setColour(textLight_);
        g.setFont(juce::FontOptions(16.0f));
        g.drawFittedText(helpTextEN, textAreaX, helpY, textAreaWidth, lineHeight, juce::Justification::centred, 2);
        g.drawFittedText(helpTextJA, textAreaX, helpY + lineHeight + 2, textAreaWidth, lineHeight, juce::Justification::centred, 2);
        g.drawFittedText(helpTextZH, textAreaX, helpY + (lineHeight + 2) * 2, textAreaWidth, lineHeight, juce::Justification::centred, 2);
    }
}

void MainComponent::paintOverChildren(juce::Graphics& g)
{
    // Draw button press glow effect (drawn over child components)
    if (glowButtonIndex_ >= 0 && glowCounter_ > 0) {
        float alpha = static_cast<float>(glowCounter_) / 6.0f;  // Fade from 1.0 to 0.0
        juce::Rectangle<float> bounds;

        switch (glowButtonIndex_) {
            case 0: bounds = generateButton_.getBounds().toFloat(); break;
            case 1: bounds = playButton_.getBounds().toFloat(); break;
            case 2: bounds = stopButton_.getBounds().toFloat(); break;
            case 3: bounds = exportButton_.getBounds().toFloat(); break;
            case 4: bounds = showChordsToggle_.getBounds().toFloat(); break;
            case 5: bounds = showTrebleToggle_.getBounds().toFloat(); break;
            case 6: bounds = showBassToggle_.getBounds().toFloat(); break;
            case 7: bounds = importMidiButton_.getBounds().toFloat(); break;
            default: break;
        }

        if (!bounds.isEmpty()) {
            // Inner glow
            g.setColour(accent_.withAlpha(0.6f * alpha));
            g.drawRoundedRectangle(bounds.expanded(4.0f), 6.0f, 3.0f);
            // Outer glow
            g.setColour(accent_.withAlpha(0.35f * alpha));
            g.drawRoundedRectangle(bounds.expanded(8.0f), 10.0f, 2.0f);
        }
    }

    // Draw chord box glow effect
    for (size_t i = 0; i < chordBoxCache_.size(); ++i) {
        if (static_cast<int>(i) == glowButtonIndex_ - 100 && glowCounter_ > 0) {
            const auto& box = chordBoxCache_[i];
            float alpha = static_cast<float>(glowCounter_) / 6.0f;
            auto bounds = juce::Rectangle<float>(static_cast<float>(box.x), static_cast<float>(box.y),
                                                  static_cast<float>(box.width), static_cast<float>(box.height));
            // Inner glow
            g.setColour(accent_.withAlpha(0.6f * alpha));
            g.drawRoundedRectangle(bounds.expanded(4.0f), 6.0f, 3.0f);
            // Outer glow
            g.setColour(accent_.withAlpha(0.35f * alpha));
            g.drawRoundedRectangle(bounds.expanded(8.0f), 10.0f, 2.0f);
        }
    }
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(20);
    area.removeFromTop(50); // Title space

    // Top controls row 1 - 增加標籤寬度，統一間距為 12px
    auto row1 = area.removeFromTop(30);

    styleLabel_.setBounds(row1.removeFromLeft(50));  // 40 -> 50
    styleSelector_.setBounds(row1.removeFromLeft(110));  // 100 -> 110
    row1.removeFromLeft(12);  // 統一間距

    keyLabel_.setBounds(row1.removeFromLeft(35));  // 30 -> 35
    keySelector_.setBounds(row1.removeFromLeft(60));
    row1.removeFromLeft(12);

    bpmLabel_.setBounds(row1.removeFromLeft(45));  // 35 -> 45
    bpmSlider_.setBounds(row1.removeFromLeft(135));  // 140 -> 135 (稍微縮小以平衡空間)
    row1.removeFromLeft(12);

    lengthLabel_.setBounds(row1.removeFromLeft(55));  // 50 -> 55
    lengthSlider_.setBounds(row1.removeFromLeft(100));
    row1.removeFromLeft(12);

    synthLabel_.setBounds(row1.removeFromLeft(55));  // 45 -> 55
    synthSelector_.setBounds(row1.removeFromLeft(85));  // 80 -> 85
    row1.removeFromLeft(12);

    audioOutputLabel_.setBounds(row1.removeFromLeft(70));
    audioOutputSelector_.setBounds(row1.removeFromLeft(160));

    area.removeFromTop(15);  // 8 -> 15 增加列間距

    // Top controls row 2 (buttons) - 更緊湊的間距
    auto row2 = area.removeFromTop(30);
    generateButton_.setBounds(row2.removeFromLeft(80));
    row2.removeFromLeft(6);
    playButton_.setBounds(row2.removeFromLeft(50));
    row2.removeFromLeft(6);
    stopButton_.setBounds(row2.removeFromLeft(50));
    row2.removeFromLeft(6);
    exportButton_.setBounds(row2.removeFromLeft(85));
    row2.removeFromLeft(6);
    importMidiButton_.setBounds(row2.removeFromLeft(85));

    // Display toggles (next to "CHORD PROGRESSION" label)
    int toggleY = 178;
    int toggleWidth = 70;
    showChordsToggle_.setBounds(180, toggleY, toggleWidth, 20);
    showTrebleToggle_.setBounds(250, toggleY, toggleWidth, 20);
    showBassToggle_.setBounds(320, toggleY, toggleWidth, 20);

    // Style parameter sliders (right side, vertical) - 增加與分隔線間距
    int sliderWidth = 55;
    int sliderHeight = 130;
    int sliderSpacing = 8;
    int slidersStartX = getWidth() - 20 - (4 * sliderWidth + 3 * sliderSpacing);
    int slidersY = 205;  // 185 -> 205 增加間距

    // Status at bottom (staff notation is drawn in paint())
    statusLabel_.setBounds(20, getHeight() - 35, slidersStartX - 40, 24);

    // Labels above sliders - 增加高度以避免截斷
    tritoneSubLabel_.setBounds(slidersStartX, slidersY, sliderWidth, 20);  // 18 -> 20
    iiVPrefLabel_.setBounds(slidersStartX + sliderWidth + sliderSpacing, slidersY, sliderWidth, 20);
    modalInterLabel_.setBounds(slidersStartX + 2 * (sliderWidth + sliderSpacing), slidersY, sliderWidth, 20);
    extensionLabel_.setBounds(slidersStartX + 3 * (sliderWidth + sliderSpacing), slidersY, sliderWidth, 20);

    // Sliders below labels
    int sliderY = slidersY + 22;  // 20 -> 22 配合標籤高度增加
    tritoneSubSlider_.setBounds(slidersStartX, sliderY, sliderWidth, sliderHeight);
    iiVPrefSlider_.setBounds(slidersStartX + sliderWidth + sliderSpacing, sliderY, sliderWidth, sliderHeight);
    modalInterSlider_.setBounds(slidersStartX + 2 * (sliderWidth + sliderSpacing), sliderY, sliderWidth, sliderHeight);
    extensionSlider_.setBounds(slidersStartX + 3 * (sliderWidth + sliderSpacing), sliderY, sliderWidth, sliderHeight);
}

void MainComponent::mouseDown(const juce::MouseEvent& event)
{
    if (currentProgression_.empty()) return;
    if (chordBoxCache_.empty()) return;

    int x = event.x;
    int y = event.y;

    // First check if clicking on chord right edge for duration resize
    for (size_t i = 0; i < chordBoxCache_.size(); ++i) {
        const auto& box = chordBoxCache_[i];
        int rightEdge = box.x + box.width;

        // Check if mouse is near the right edge (8px range) and within box height
        if (x >= rightEdge - 8 && x <= rightEdge + 4 &&
            y >= box.y && y <= box.y + box.height) {
            resizingChordIndex_ = static_cast<int>(i);
            resizeStartX_ = x;
            resizeStartDuration_ = currentProgression_[i].getDuration();
            glowButtonIndex_ = 100 + static_cast<int>(i);
            glowCounter_ = 6;
            statusLabel_.setText("Resizing duration...", juce::dontSendNotification);
            return;
        }
    }

    // Then check if clicking on a note (for dragging) - priority over chord box
    for (size_t i = 0; i < chordBoxCache_.size(); ++i) {
        int noteIdx = findNoteAtPosition(static_cast<int>(i), x, y);
        if (noteIdx >= 0) {
            draggingChordIndex_ = static_cast<int>(i);
            draggingNoteIndex_ = noteIdx;
            // Trigger glow effect for chord box
            glowButtonIndex_ = 100 + static_cast<int>(i);
            glowCounter_ = 6;
            // Use custom voicing if available, otherwise use original
            if (customVoicings_.count(i) > 0) {
                draggedNotes_ = customVoicings_.at(i);
            } else {
                draggedNotes_ = currentProgression_[i].getMIDINotes(4);
            }
            // Play chord when starting to drag (short duration: 0.4s = 12 ticks at 30Hz)
            chordSynth_.playNotes(draggedNotes_);
            previewStopCounter_ = 12;
            statusLabel_.setText("Dragging note...", juce::dontSendNotification);
            return;
        }
    }

    // Then check if clicking on a chord box (only if chords are visible)
    if (showChordsToggle_.getToggleState()) {
        int chordIndex = findChordAtPosition(x, y);
        if (chordIndex >= 0) {
            const auto& box = chordBoxCache_[static_cast<size_t>(chordIndex)];
            int boxMidY = box.y + box.height / 2;

            // Trigger glow effect for chord box
            glowButtonIndex_ = 100 + chordIndex;
            glowCounter_ = 6;

            // Click on lower half: play chord sound
            if (y > boxMidY) {
                // Get notes (custom voicing or original)
                std::vector<int> notes;
                if (customVoicings_.count(static_cast<size_t>(chordIndex)) > 0) {
                    notes = customVoicings_.at(static_cast<size_t>(chordIndex));
                } else {
                    notes = currentProgression_[static_cast<size_t>(chordIndex)].getMIDINotes(4);
                }
                chordSynth_.playNotes(notes);
                previewStopCounter_ = 30;  // 1 second at 30Hz
                statusLabel_.setText("Playing: " + juce::String(currentProgression_[static_cast<size_t>(chordIndex)].toString()), juce::dontSendNotification);
                return;
            }

            // Click on upper half: edit chord
            editingChordIndex_ = chordIndex;
            chordEditor_.setBounds(box.x, box.y, box.width, box.height);
            chordEditor_.setText(currentProgression_[static_cast<size_t>(chordIndex)].toString(), false);
            chordEditor_.setVisible(true);
            chordEditor_.grabKeyboardFocus();
            chordEditor_.selectAll();
            return;
        }
    }
}

void MainComponent::mouseDrag(const juce::MouseEvent& event)
{
    // Handle chord duration resizing
    if (resizingChordIndex_ >= 0 && static_cast<size_t>(resizingChordIndex_) < currentProgression_.size()) {
        int deltaX = event.x - resizeStartX_;
        // Convert pixel delta to duration delta (40 pixels = 1 beat)
        double deltaDuration = deltaX / 40.0;
        double newDuration = resizeStartDuration_ + deltaDuration;
        // Clamp to valid range (0.5 to 8 beats)
        newDuration = std::max(0.5, std::min(8.0, newDuration));
        // Snap to 0.5 beat increments
        newDuration = std::round(newDuration * 2.0) / 2.0;

        currentProgression_[static_cast<size_t>(resizingChordIndex_)].setDuration(newDuration);

        // Update status
        juce::String durationStr = juce::String(newDuration, 1);
        if (durationStr.endsWith(".0")) durationStr = juce::String(static_cast<int>(newDuration));
        statusLabel_.setText("Duration: " + durationStr + " beats", juce::dontSendNotification);
        repaint();
        return;
    }

    // Handle note dragging
    if (draggingChordIndex_ < 0 || draggingNoteIndex_ < 0) return;
    if (static_cast<size_t>(draggingChordIndex_) >= chordBoxCache_.size()) return;

    const auto& box = chordBoxCache_[static_cast<size_t>(draggingChordIndex_)];
    int newMidi = staffYToMidi(event.y, box.trebleY, box.bassY, box.staffSpacing);

    // Clamp to valid MIDI range
    newMidi = std::max(36, std::min(84, newMidi));

    if (static_cast<size_t>(draggingNoteIndex_) < draggedNotes_.size()) {
        int oldMidi = draggedNotes_[static_cast<size_t>(draggingNoteIndex_)];
        if (newMidi != oldMidi) {
            draggedNotes_[static_cast<size_t>(draggingNoteIndex_)] = newMidi;
            // Play updated chord when note position changes (short: 0.4s)
            chordSynth_.playNotes(draggedNotes_);
            previewStopCounter_ = 12;
            repaint();
        }
    }
}

void MainComponent::mouseUp(const juce::MouseEvent& /*event*/)
{
    // Handle chord duration resize completion
    if (resizingChordIndex_ >= 0) {
        statusLabel_.setText("Duration updated", juce::dontSendNotification);
        resizingChordIndex_ = -1;
        repaint();
        return;
    }

    if (draggingChordIndex_ >= 0 && draggingNoteIndex_ >= 0 && !draggedNotes_.empty()) {
        // Save the custom voicing
        customVoicings_[static_cast<size_t>(draggingChordIndex_)] = draggedNotes_;

        // Recognize and update chord symbol based on new notes
        auto newChord = JazzArchitect::MIDIImporter::recognizeChordFromNotes(draggedNotes_);
        // Preserve duration from original chord
        double originalDuration = currentProgression_[static_cast<size_t>(draggingChordIndex_)].getDuration();
        newChord.setDuration(originalDuration);
        currentProgression_[static_cast<size_t>(draggingChordIndex_)] = newChord;

        statusLabel_.setText("Chord updated: " + juce::String(newChord.toString()), juce::dontSendNotification);
    }
    draggingChordIndex_ = -1;
    draggingNoteIndex_ = -1;
    draggedNotes_.clear();
    repaint();
}

void MainComponent::mouseMove(const juce::MouseEvent& event)
{
    // Check if mouse is near any chord box right edge for resize cursor
    int x = event.x;
    int y = event.y;

    for (size_t i = 0; i < chordBoxCache_.size(); ++i) {
        const auto& box = chordBoxCache_[i];
        int rightEdge = box.x + box.width;

        // Check if mouse is near the right edge (8px range) and within box height
        if (x >= rightEdge - 8 && x <= rightEdge + 4 &&
            y >= box.y && y <= box.y + box.height) {
            setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
            return;
        }
    }

    setMouseCursor(juce::MouseCursor::NormalCursor);
}

int MainComponent::findChordAtPosition(int x, int y)
{
    for (size_t i = 0; i < chordBoxCache_.size(); ++i) {
        const auto& box = chordBoxCache_[i];
        if (x >= box.x && x <= box.x + box.width &&
            y >= box.y && y <= box.y + box.height) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int MainComponent::findNoteAtPosition(int chordIndex, int x, int y)
{
    if (chordIndex < 0 || static_cast<size_t>(chordIndex) >= chordBoxCache_.size()) return -1;
    if (static_cast<size_t>(chordIndex) >= currentProgression_.size()) return -1;

    const auto& box = chordBoxCache_[static_cast<size_t>(chordIndex)];
    size_t idx = static_cast<size_t>(chordIndex);

    // Use custom voicing if available
    std::vector<int> notes;
    if (customVoicings_.count(idx) > 0) {
        notes = customVoicings_.at(idx);
    } else {
        notes = currentProgression_[idx].getMIDINotes(4);
    }

    int noteCenterX = box.x + box.width / 2;

    // Check if x is near the note column (wider range for easier clicking)
    if (std::abs(x - noteCenterX) > 25) return -1;

    // Find the closest note within range
    int closestNote = -1;
    int closestDist = 999;

    for (size_t i = 0; i < notes.size(); ++i) {
        // Offset by 4 to match the drawing position (noteY - 4 is the top of ellipse)
        int noteY = midiToStaffY(notes[i], box.trebleY, box.bassY, box.staffSpacing);
        int dist = std::abs(y - noteY);
        if (dist <= 12 && dist < closestDist) {
            closestDist = dist;
            closestNote = static_cast<int>(i);
        }
    }
    return closestNote;
}

int MainComponent::staffYToMidi(int y, int trebleY, int bassY, int spacing)
{
    // Reverse of midiToStaffY
    // B4 is at trebleY + 2 * spacing
    int b4Position = trebleY + 2 * spacing;
    int stepsFromB4 = (b4Position - y) * 2 / spacing;

    // Convert steps to MIDI note
    static const int stepToNote[] = {11, 0, 2, 4, 5, 7, 9};  // B, C, D, E, F, G, A
    int octaveOffset = stepsFromB4 / 7;
    int stepInOctave = ((stepsFromB4 % 7) + 7) % 7;

    int midiNote = 59 + octaveOffset * 12 + stepToNote[stepInOctave];  // B4 = 71
    midiNote = 71 + stepsFromB4 + (stepsFromB4 < 0 ? -1 : 0);

    // Simplified calculation: approximate from B4 (MIDI 71)
    midiNote = 71 + stepsFromB4;

    // Clamp to reasonable range
    return std::max(36, std::min(84, midiNote));
}

void MainComponent::parseChordInput(const juce::String& input)
{
    if (editingChordIndex_ < 0 || static_cast<size_t>(editingChordIndex_) >= currentProgression_.size()) {
        return;
    }

    juce::String trimmed = input.trim();
    if (trimmed.isEmpty()) return;

    try {
        auto newChord = JazzArchitect::ChordSymbol::fromString(trimmed.toStdString());
        currentProgression_[static_cast<size_t>(editingChordIndex_)] = newChord;
        statusLabel_.setText("Chord updated: " + trimmed, juce::dontSendNotification);
    } catch (...) {
        statusLabel_.setText("Invalid chord: " + trimmed, juce::dontSendNotification);
    }
}

void MainComponent::drawCircleOfFifths(juce::Graphics& g, int centerX, int centerY, int radius)
{
    // Circle of fifths order (starting from C at top, going clockwise)
    static const char* noteNames[] = {"C", "G", "D", "A", "E", "B", "Gb", "Db", "Ab", "Eb", "Bb", "F"};
    static const int pitchClasses[] = {0, 7, 2, 9, 4, 11, 6, 1, 8, 3, 10, 5};  // Corresponding pitch classes

    // Draw outer circle
    g.setColour(bgMid_);
    g.fillEllipse(static_cast<float>(centerX - radius),
                  static_cast<float>(centerY - radius),
                  static_cast<float>(radius * 2),
                  static_cast<float>(radius * 2));

    g.setColour(textDim_.withAlpha(0.5f));
    g.drawEllipse(static_cast<float>(centerX - radius),
                  static_cast<float>(centerY - radius),
                  static_cast<float>(radius * 2),
                  static_cast<float>(radius * 2), 1.0f);

    // Get current chord root (if playing)
    int currentRoot = -1;
    if (!currentProgression_.empty() && isPlaying_ && currentChordIndex_ >= 0 &&
        static_cast<size_t>(currentChordIndex_) < currentProgression_.size()) {
        currentRoot = currentProgression_[static_cast<size_t>(currentChordIndex_)].root().value();
    }

    // Find position in circle for current root
    int currentPos = -1;
    for (int i = 0; i < 12; ++i) {
        if (pitchClasses[i] == currentRoot) currentPos = i;
    }

    // Draw note labels
    g.setFont(juce::FontOptions(14.0f));
    float labelRadius = static_cast<float>(radius) * 0.78f;

    for (int i = 0; i < 12; ++i) {
        float angle = static_cast<float>(i) * juce::MathConstants<float>::twoPi / 12.0f - juce::MathConstants<float>::halfPi;
        float x = static_cast<float>(centerX) + std::cos(angle) * labelRadius;
        float y = static_cast<float>(centerY) + std::sin(angle) * labelRadius;

        // Highlight current chord root
        if (i == currentPos) {
            g.setColour(accent_);
            g.fillEllipse(x - 12, y - 8, 24, 16);
            g.setColour(bgDark_);
        } else {
            g.setColour(textDim_);
        }

        g.drawText(noteNames[i], static_cast<int>(x - 12), static_cast<int>(y - 8), 24, 16,
                   juce::Justification::centred);
    }

    // Draw center label
    g.setColour(textDim_.withAlpha(0.6f));
    g.setFont(juce::FontOptions(14.0f));
    g.drawText("Circle of", centerX - 35, centerY - 16, 70, 16, juce::Justification::centred);
    g.drawText("Fifths", centerX - 35, centerY + 2, 70, 16, juce::Justification::centred);
}
