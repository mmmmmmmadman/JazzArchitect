#include "MainComponent.h"
#include "MIDI/MIDIExporter.h"

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
    bpmSlider_.setRange(60, 200, 1);
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

    // Buttons
    generateButton_.onClick = [this] { generateProgression(); };
    playButton_.onClick = [this] { startPlayback(); };
    stopButton_.onClick = [this] { stopPlayback(); };
    exportButton_.onClick = [this] { exportMIDI(); };
    styleButton(generateButton_, accent_);
    styleButton(playButton_, accent_);
    styleButton(stopButton_, textLight_);
    styleButton(exportButton_, textDim_);
    addAndMakeVisible(generateButton_);
    addAndMakeVisible(playButton_);
    addAndMakeVisible(stopButton_);
    addAndMakeVisible(exportButton_);

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

    // Display toggles
    auto setupToggle = [this](juce::ToggleButton& toggle) {
        toggle.setToggleState(true, juce::dontSendNotification);
        toggle.setColour(juce::ToggleButton::textColourId, textDim_);
        toggle.setColour(juce::ToggleButton::tickColourId, accent_);
        toggle.setColour(juce::ToggleButton::tickDisabledColourId, textDim_);
        toggle.onClick = [this] { repaint(); };
        addAndMakeVisible(toggle);
    };
    setupToggle(showChordsToggle_);
    setupToggle(showTrebleToggle_);
    setupToggle(showBassToggle_);

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

        int newChordIndex = static_cast<int>(playbackPosition_ / beatsPerChord_);
        if (newChordIndex >= static_cast<int>(currentProgression_.size())) {
            newChordIndex = 0;
            playbackPosition_ = 0.0;
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
        int sectionGap = 15;  // Gap between sections (+10px from before)

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

            // Draw chords and notes for this row
            for (int col = 0; col < chordsInThisRow; ++col) {
                size_t idx = static_cast<size_t>(row * chordsPerRow + col);
                if (idx >= currentProgression_.size()) break;

                int x = startX + col * (boxWidth + spacing);

                // Update chord box cache
                if (idx < chordBoxCache_.size()) {
                    chordBoxCache_[idx] = { x, chordBoxY, boxWidth, boxHeight, trebleY, bassY, staffLineSpacing };
                }

                // Draw chord box
                if (showChords) {
                    if (static_cast<int>(idx) == currentChordIndex_ && isPlaying_) {
                        g.setColour(accent_.withAlpha(0.3f));
                    } else {
                        g.setColour(bgMid_);
                    }
                    g.fillRoundedRectangle(static_cast<float>(x), static_cast<float>(chordBoxY),
                                           static_cast<float>(boxWidth), static_cast<float>(boxHeight), 4.0f);

                    if (static_cast<int>(idx) == currentChordIndex_ && isPlaying_) {
                        g.setColour(accent_);
                        g.drawRoundedRectangle(static_cast<float>(x), static_cast<float>(chordBoxY),
                                               static_cast<float>(boxWidth), static_cast<float>(boxHeight), 4.0f, 2.0f);
                    }

                    g.setColour(textLight_);
                    g.setFont(juce::FontOptions(16.0f).withStyle("Bold"));
                    g.drawText(currentProgression_[idx].toString(), x, chordBoxY + 8, boxWidth, 20, juce::Justification::centred);

                    g.setFont(juce::FontOptions(11.0f));
                    g.setColour(accentDim_);
                    JazzArchitect::PitchClass key(keySelector_.getSelectedId() - 1);
                    g.drawText(currentProgression_[idx].asRomanNumeral(key), x, chordBoxY + 32, boxWidth, 16, juce::Justification::centred);
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

                    juce::Colour noteColor = (static_cast<int>(idx) == currentChordIndex_ && isPlaying_) ? accent_ : accentDim_;

                    int noteCenterX = x + boxWidth / 2;
                    for (size_t noteIdx = 0; noteIdx < midiNotes.size(); ++noteIdx) {
                        int midi = midiNotes[noteIdx];
                        int noteY = midiToStaffY(midi, trebleY, bassY, staffLineSpacing);
                        bool isTrebleNote = (midi >= 60);  // Middle C and above

                        // Only draw if the corresponding staff is visible
                        if ((isTrebleNote && showTreble) || (!isTrebleNote && showBass)) {
                            // Highlight dragging note
                            if (draggingChordIndex_ == static_cast<int>(idx) &&
                                draggingNoteIndex_ == static_cast<int>(noteIdx)) {
                                g.setColour(accent_);
                            } else {
                                g.setColour(noteColor);
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

    // Hover help text for sliders (below Circle of Fifths)
    int helpY = circleCenterY + circleRadius + 15;
    int helpWidth = 4 * sliderWidth + 3 * sliderSpacing;
    int helpX = slidersStartX;

    juce::String helpTextEN, helpTextJA, helpTextZH;

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

    // Top controls row 2 (buttons) - 統一間距
    auto row2 = area.removeFromTop(30);
    generateButton_.setBounds(row2.removeFromLeft(95));  // 90 -> 95
    row2.removeFromLeft(10);  // 8 -> 10
    playButton_.setBounds(row2.removeFromLeft(75));  // 70 -> 75
    row2.removeFromLeft(10);
    stopButton_.setBounds(row2.removeFromLeft(75));  // 70 -> 75
    row2.removeFromLeft(10);
    exportButton_.setBounds(row2.removeFromLeft(105));  // 100 -> 105

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

    // First check if clicking on a note (for dragging) - priority over chord box
    for (size_t i = 0; i < chordBoxCache_.size(); ++i) {
        int noteIdx = findNoteAtPosition(static_cast<int>(i), x, y);
        if (noteIdx >= 0) {
            draggingChordIndex_ = static_cast<int>(i);
            draggingNoteIndex_ = noteIdx;
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
    if (draggingChordIndex_ >= 0 && draggingNoteIndex_ >= 0 && !draggedNotes_.empty()) {
        // Save the custom voicing
        customVoicings_[static_cast<size_t>(draggingChordIndex_)] = draggedNotes_;
        statusLabel_.setText("Note position saved", juce::dontSendNotification);
    }
    draggingChordIndex_ = -1;
    draggingNoteIndex_ = -1;
    draggedNotes_.clear();
    repaint();
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
