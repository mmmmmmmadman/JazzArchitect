# JazzArchitect: A Jazz Harmony Generation System Based on Recursive Probabilistic Grammar

**Abstract**

This paper presents JazzArchitect, a jazz chord progression generation system based on the recursive Probabilistic Context-Free Grammar (PCFG) framework proposed by Rohrmeier (2020). Implemented in C++ with the JUCE audio framework, the system provides real-time audio synthesis and MIDI output capabilities. Through a 15-parameter style vector, the system can generate chord progressions in 9 different jazz styles, including Bebop, Cool Jazz, and Modal Jazz. The system implements three major substitution rules (tritone substitution, backdoor dominant, and Coltrane changes) along with Smither (2019) guide-tone voice leading optimization. This paper details the system architecture, algorithm design, and implementation specifics.

**Keywords**: Jazz harmony, Probabilistic context-free grammar, Chord progression generation, Voice leading, Music information retrieval

---

## 1. Introduction

### 1.1 Motivation

Jazz harmony is renowned for its complex chord progressions and rich substitution rules. Traditional jazz improvisers master these rules through years of study, but for computer music generation systems, simulating this musical knowledge remains a significant challenge.

Existing music generation methods fall into two main categories:

1. **Data-driven methods**: Using deep learning (such as LSTM, Transformer) to learn harmonic patterns from corpora
2. **Rule-based methods**: Generating based on music theory rules

While data-driven methods can produce fluent results, they lack interpretability and controllability. Rule-based methods offer precise control over the generation process, but traditional implementations tend to be overly rigid.

### 1.2 Research Objectives

This research aims to combine the advantages of both approaches:

1. Adopt the recursive grammar framework proposed by Rohrmeier (2020) for theoretical foundation
2. Use probabilistic weights to control rule selection, increasing generation diversity
3. Implement a style parameter system allowing cross-era jazz style generation
4. Provide complete audio synthesis and MIDI output functionality

### 1.3 Contributions

The main contributions of this research are:

- Complete implementation of the Rohrmeier (2020) PCFG framework in C++
- Design of a 15-parameter style vector system supporting 9 jazz style presets
- Integration of three major substitution rules and guide-tone voice leading optimization
- Development of a standalone application with graphical user interface

---

## 2. Related Work

### 2.1 Formal Grammar for Jazz Harmony

Steedman (1984) first proposed analyzing jazz chord sequences using formal grammar, treating jazz harmony as a linguistic structure. This study defined basic generation rules but did not address probabilistic weighting.

Rohrmeier (2020) extended this framework, proposing a recursive grammar containing the following core concepts:

- **Prolongation**: Repetition or elaboration of functionally equivalent chords
- **Preparation**: Dominant function chords preparing target chords
- **Substitution**: Replacement of functionally equivalent chords

### 2.2 Probabilistic Context-Free Grammar

Harasim et al. (2018, 2020) applied PCFG to the Jazz Harmony Treebank (JHT), extracting rule weights from 150 jazz standards. This method combines the structural nature of formal grammar with the flexibility of statistical methods.

### 2.3 Voice Leading Theory

Smither (2019) proposed guide-tone voice leading theory for jazz harmony. Core concepts include:

- **Guide tones**: The third and seventh of each chord
- **Smooth voice leading**: Guide tones of adjacent chords should move by minimal intervals
- **Target cost**: Ideal voice leading cost should be 2 semitones or less

### 2.4 Substitution Rules

Common substitution rules in jazz harmony include:

| Substitution Type | Original Chord | Substitute Chord | Theoretical Basis |
|-------------------|----------------|------------------|-------------------|
| Tritone substitution | V7 | bII7 | Shared tritone interval |
| Backdoor dominant | V7 | bVII7 | Minor key borrowing |
| Coltrane changes | I | Major third cycle | Coltrane style |

---

## 3. System Architecture

### 3.1 Overall Architecture

JazzArchitect employs a layered architecture design:

```
User Interface Layer
├── JUCE GUI
└── Parameter Controls

Generation Layer
├── PCFG Engine
│   ├── Generator
│   └── Rule Library
└── Style Engine
    ├── Substitution Processing
    ├── Voice Leading Optimization
    └── Style Parameters

Core Layer
├── Chord Symbol
├── Pitch Class
└── Interval Calculation

Output Layer
├── Chord Synthesizer (8-voice polyphony)
└── MIDI Exporter
```

### 3.2 Module Description

#### Core Module
- **PitchClass**: Pitch class representation (0-11, C=0)
- **ChordQuality**: Chord quality enumeration (maj7, min7, dom7, hdim7, dim7, aug)
- **ChordSymbol**: Complete chord symbol including root, quality, extensions, and alterations

#### Grammar Module
- **NonTerminal**: Non-terminal symbols (S, T, D, SD, PREP, PROL)
- **GrammarRule**: Production rule definitions
- **PCFG**: Probabilistic context-free grammar engine
- **Generator**: Top-down derivation engine

#### Style Module
- **StyleVector**: 15-parameter style vector
- **StylePresets**: 9 style presets
- **StyleEngine**: Integrates generation and post-processing

#### Substitution Module
- **TritoneSubstitution**: V7 to bII7
- **BackdoorSubstitution**: V7 to bVII7
- **ColtraneSubstitution**: Major third cycle substitution
- **SubstitutionEngine**: Unified substitution processor

#### Voice Leading Module
- **GuideTone**: Third and seventh analysis
- **VoiceLeadingOptimizer**: Minimizes voice leading cost

#### Synthesis Module
- **ChordSynth**: 8-voice polyphonic chord synthesizer
- **ADSREnvelope**: Attack-Decay-Sustain-Release envelope
- **SynthVoice**: Single synthesis voice

#### MIDI Module
- **MIDIExporter**: Standard MIDI File export

---

## 4. Grammar Engine

### 4.1 Non-Terminal Symbols

The system defines the following non-terminal symbols:

| Symbol | Name | Function |
|--------|------|----------|
| S | Start | Start symbol (entire piece) |
| T | Tonic | Tonic region |
| D | Dominant | Dominant region |
| SD | Subdominant | Subdominant region |
| PREP | Preparation | Preparation region |
| PROL | Prolongation | Prolongation region |

### 4.2 Core Production Rules

#### Prolongation Rules
Prolongation rules repeat or elaborate functionally equivalent chords:

- Tonic prolongation: T produces T T
- Dominant prolongation: D produces D D
- Right prolongation: T produces T PROL
- Left prolongation: T produces PROL T

#### Preparation Rules
Preparation rules insert dominant function chords:

- Authentic cadence: T produces D T
- Plagal cadence: T produces SD T
- ii-V pattern: D produces PREP D
- Secondary dominant preparation: PREP produces ii
- Double dominant preparation: PREP produces V/V

#### Substitution Rules
Substitution rules are unary productions:

- Tritone substitution: V7 produces bII7
- Backdoor dominant: V7 produces bVII7
- Coltrane changes: I produces major third cycle

### 4.3 Rule Probabilities

Rule probabilities are controlled by the StyleVector. For the ii-V pattern:

- Probability of choosing preparation = StyleVector.iiVPreference
- Probability of choosing prolongation = 1 - iiVPreference

### 4.4 Derivation Algorithm

The system employs top-down recursive derivation:

1. Initialize derivation tree with start symbol S
2. While tree contains non-terminals:
   - Select leftmost non-terminal N
   - Get applicable rules for N
   - Weight rules according to style parameters
   - Sample rule r according to probabilities
   - Apply r, replacing N with RHS symbols
3. Map terminals to chord symbols in the key
4. Return chord progression

---

## 5. Style System

### 5.1 StyleVector Parameters

StyleVector contains 15 parameters controlling generation behavior:

| Parameter | Range | Description |
|-----------|-------|-------------|
| tritoneSubProb | 0.0-1.0 | Tritone substitution probability |
| iiVPreference | 0.0-1.0 | ii-V progression preference |
| modalInterchange | 0.0-1.0 | Modal interchange frequency |
| chromaticApproach | 0.0-1.0 | Chromatic approach frequency |
| dominantChainDepth | 1-5 | Maximum dominant chain depth |
| rhythmDensity | 0.0-1.0 | Chord change density |
| extensionLevel | 0.0-1.0 | Extension usage level |
| backdoorProb | 0.0-1.0 | Backdoor dominant probability |
| coltraneProb | 0.0-1.0 | Coltrane changes probability |
| minorBorrowing | 0.0-1.0 | Minor key borrowing frequency |
| diminishedUsage | 0.0-1.0 | Diminished chord usage frequency |
| deceptiveResolution | 0.0-1.0 | Deceptive cadence probability |
| plagalCadence | 0.0-1.0 | Plagal cadence probability |
| suspensionUsage | 0.0-1.0 | Suspension usage frequency |
| pedalPointProb | 0.0-1.0 | Pedal point probability |

### 5.2 Style Presets

The system provides 9 style presets:

| Style | Tritone | ii-V | Modal | Extension | Characteristics |
|-------|---------|------|-------|-----------|-----------------|
| Bebop | 0.30 | 0.90 | 0.20 | 0.70 | Fast ii-V, rich extensions |
| Cool | 0.20 | 0.70 | 0.30 | 0.50 | Slower tempo, spacious |
| Modal | 0.10 | 0.30 | 0.70 | 0.30 | Few functional progressions, modal color |
| Hard Bop | 0.35 | 0.85 | 0.25 | 0.60 | Blues influence, strong rhythm |
| Post-Bop | 0.40 | 0.60 | 0.60 | 0.80 | Modern harmony, complex structure |
| Swing | 0.15 | 0.80 | 0.10 | 0.40 | Traditional progressions, concise |
| Fusion | 0.30 | 0.50 | 0.50 | 0.90 | Rich extensions, modal mixture |
| Contemporary | 0.35 | 0.40 | 0.70 | 0.85 | Modern techniques, free structure |
| Blues | 0.20 | 0.60 | 0.40 | 0.50 | Blues progressions, parallel chords |

### 5.3 Style Engine

StyleEngine integrates the generation workflow:

1. Generate base progression using PCFG
2. Apply substitution engine to process substitution rules
3. Optimize using voice leading optimizer
4. Return final progression

---

## 6. Substitution Rules

### 6.1 Tritone Substitution

Tritone substitution is based on V7 and bII7 sharing the same tritone interval:

- G7 tritone: B-F (3rd and 7th)
- Db7 tritone: F-Cb (3rd and 7th)

Both share B/Cb and F, making them interchangeable.

In implementation, the system replaces V7 with bII7 according to tritoneSubProb. The new root is calculated as the original root plus 6 semitones (tritone interval).

### 6.2 Backdoor Dominant

Backdoor dominant comes from minor key borrowing, replacing V7 with bVII7:

- Traditional: G7 resolves to Cmaj7
- Backdoor: Bb7 resolves to Cmaj7

Implementation is controlled by backdoorProb. The new root is the original root plus 3 semitones (ascending minor third).

### 6.3 Coltrane Changes

Coltrane changes originate from John Coltrane's "Giant Steps," replacing simple progressions with major third cycles:

- Traditional: I chord
- Coltrane: III7 - bVImaj7 - bII7 - V7 - I

Implementation expands a single I chord into a sequence of 5 chords.

---

## 7. Voice Leading Optimization

### 7.1 Guide-Tone Analysis

According to Smither (2019), guide tones are the third and seventh of each chord:

| Chord Type | Third | Seventh |
|------------|-------|---------|
| Cmaj7 | E (4) | B (11) |
| Dm7 | F (5) | C (0) |
| G7 | B (11) | F (5) |

### 7.2 Voice Leading Cost

Voice leading cost between two chords is defined as the minimum movement of guide tones:

Cost = |third1 - third2| + |seventh1 - seventh2|

Intervals are calculated as minimum values (considering octave equivalence).

### 7.3 Optimization Algorithm

VoiceLeadingOptimizer checks each pair of adjacent chords, attempting substitution if cost is too high:

1. For each pair of adjacent chords
2. Calculate voice leading cost
3. If cost > 2:
   - Try tritone substitution on the first chord
   - Calculate new cost
   - If new cost is lower, apply substitution
4. Return optimized progression

---

## 8. Audio Synthesis

### 8.1 Synthesizer Architecture

ChordSynth is an 8-voice polyphonic chord synthesizer supporting three timbres:

| Timbre | Synthesis Method | Characteristics |
|--------|------------------|-----------------|
| Electric Piano | FM Synthesis | Rhodes style, decaying modulation |
| Organ | Additive Synthesis | Hammond style, harmonic stacking |
| Pad | Subtractive Synthesis | Sawtooth wave + low-pass filter |

### 8.2 Envelope Settings

Each SynthVoice contains an independent ADSR envelope:

| Timbre | Attack | Decay | Sustain | Release |
|--------|--------|-------|---------|---------|
| Electric Piano | 5ms | 300ms | 0.3 | 400ms |
| Organ | 10ms | 50ms | 0.9 | 100ms |
| Pad | 200ms | 500ms | 0.7 | 800ms |

### 8.3 Chord Voicing

ChordSymbol.getMIDINotes() generates MIDI notes:

1. Calculate root MIDI value = baseOctave * 12 + root
2. Add root note
3. For each interval in chord quality, add corresponding note
4. For each extension, add corresponding note
5. Return note list

---

## 9. Implementation Details

### 9.1 Development Environment

- **Language**: C++17
- **Framework**: JUCE 7.x
- **Build System**: CMake 3.22+
- **Platform**: macOS (ARM64/x86_64)

### 9.2 File Structure

```
cpp/
├── Source/
│   ├── Core/           # Core classes
│   ├── Grammar/        # Grammar engine
│   ├── Style/          # Style system
│   ├── Substitution/   # Substitution rules
│   ├── VoiceLeading/   # Voice leading
│   ├── Synthesis/      # Audio synthesis
│   ├── MIDI/           # MIDI output
│   ├── Main.cpp
│   ├── MainComponent.h
│   └── MainComponent.cpp
├── JUCE/               # JUCE submodule
├── CMakeLists.txt
└── build/
```

### 9.3 Build Instructions

```bash
cd cpp
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### 9.4 Application Specifications

- **File Size**: Approximately 11 MB
- **Minimum Requirements**: macOS 10.13+
- **Audio Engine**: CoreAudio, 48 kHz
- **CPU Usage**: Very low (< 1%)

---

## 10. User Interface

### 10.1 Interface Layout

```
┌─────────────────────────────────────────────────────────────┐
│ Jazz Architect                                              │
├─────────────────────────────────────────────────────────────┤
│ Style: [Bebop   v]  Key: [C      v]  Sound: [E.Piano  v]   │
│ BPM: [===120===]    Length: [===8===]                      │
│                                                             │
│ [Generate]  [Play]  [Stop]  [Export MIDI]                  │
├─────────────────────────────────────────────────────────────┤
│                    CHORD PROGRESSION                        │
│  ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐     │
│  │Dm7   │ │G7    │ │Cmaj7 │ │A7    │ │Dm7   │ │G7    │     │
│  └──────┘ └──────┘ └──────┘ └──────┘ └──────┘ └──────┘     │
├─────────────────────────────────────────────────────────────┤
│ Tritone [═══○═══]  ii-V [═══════○]  Modal [═○═══════]      │
│ Ext     [═════○═]                                          │
└─────────────────────────────────────────────────────────────┘
```

### 10.2 Operation Flow

1. Select style preset (or adjust parameter sliders)
2. Select key and length
3. Click Generate to create new progression
4. Click Play to playback (select timbre)
5. Click Export MIDI to export file

---

## 11. Conclusion and Future Work

### 11.1 Summary of Results

This research successfully implemented the JazzArchitect system, achieving the following objectives:

1. **Theoretical Integration**: Complete implementation of Rohrmeier (2020) PCFG framework
2. **Style Control**: 15-parameter style vector and 9 presets
3. **Substitution Rules**: Tritone, backdoor, and Coltrane substitutions
4. **Voice Optimization**: Smither (2019) guide-tone voice leading
5. **Practical Tool**: GUI, audio synthesis, MIDI output

### 11.2 Future Work

1. **Corpus Training**: Extract rule probabilities from Jazz Harmony Treebank
2. **Evaluation Integration**: Add MusDr evaluation metrics
3. **Listening Experiments**: Conduct A/B comparison tests
4. **Score Display**: Add staff notation visualization
5. **Real-time MIDI**: Connect to external synthesizers
6. **Cross-platform**: Windows/Linux versions

---

## References

1. Rohrmeier, M. (2020). Towards a Syntax of Jazz Harmony. *Music Theory and Analysis*, 7(1), 1-62.

2. Steedman, M. J. (1984). A generative grammar for jazz chord sequences. *Music Perception*, 2(1), 52-77.

3. Smither, S. (2019). Guide-tone voice leading in jazz. *Music Theory Online*, 25(2).

4. Harasim, D., Finkensiep, C., Ericson, P., O'Donnell, T. J., & Rohrmeier, M. (2020). The jazz harmony treebank. *Proceedings of ISMIR 2020*.

5. Tymoczko, D. (2006). The geometry of musical chords. *Science*, 313(5783), 72-74.

6. Wu, S. L., & Yang, Y. H. (2020). The jazz transformer on the front line: Exploring the shortcomings of AI-composed music through quantitative measures. *Proceedings of ISMIR 2020*.

7. Granroth-Wilding, M., & Steedman, M. (2014). A robust parser-interpreter for jazz chord sequences. *Journal of New Music Research*, 43(4), 355-374.

---

## Appendix A: Chord Quality Interval Table

| Chord Quality | Intervals (semitones) |
|---------------|----------------------|
| MAJ7 | 0, 4, 7, 11 |
| MIN7 | 0, 3, 7, 10 |
| DOM7 | 0, 4, 7, 10 |
| HDIM7 | 0, 3, 6, 10 |
| DIM7 | 0, 3, 6, 9 |
| AUG | 0, 4, 8 |
| MAJ6 | 0, 4, 7, 9 |
| MIN6 | 0, 3, 7, 9 |

## Appendix B: Non-Terminal to Terminal Mapping

| Non-Terminal | Function | Common Terminals |
|--------------|----------|------------------|
| T | Tonic | Imaj7, vi7, iii7 |
| D | Dominant | V7, vii-7 |
| SD | Subdominant | IV, ii7 |
| PREP | Preparation | ii7, IV |

---

*JazzArchitect v1.0 - January 2, 2026*
