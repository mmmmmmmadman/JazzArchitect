#pragma once

#include <JuceHeader.h>
#include "../Core/ChordSymbol.h"
#include <vector>

namespace JazzArchitect {

/**
 * MIDI File Exporter
 * Exports chord progressions to Standard MIDI Files
 */
class MIDIExporter {
public:
    /**
     * Export a chord progression to a MIDI file
     * @param chords The chord progression to export
     * @param file The destination file
     * @param bpm Tempo in beats per minute
     * @param beatsPerChord Duration of each chord in beats
     * @param baseOctave Base octave for chord voicing
     * @return true if export was successful
     */
    static bool exportToFile(
        const std::vector<ChordSymbol>& chords,
        const juce::File& file,
        double bpm = 120.0,
        double beatsPerChord = 2.0,
        int baseOctave = 3)
    {
        juce::MidiFile midiFile;
        midiFile.setTicksPerQuarterNote(480);

        juce::MidiMessageSequence track;

        // Add tempo event
        double microsecondsPerBeat = 60000000.0 / bpm;
        juce::MidiMessage tempoEvent = juce::MidiMessage::tempoMetaEvent(
            static_cast<int>(microsecondsPerBeat));
        tempoEvent.setTimeStamp(0);
        track.addEvent(tempoEvent);

        // Add time signature (4/4)
        juce::MidiMessage timeSignature = juce::MidiMessage::timeSignatureMetaEvent(4, 2);
        timeSignature.setTimeStamp(0);
        track.addEvent(timeSignature);

        // Add track name
        juce::MidiMessage trackName = juce::MidiMessage::textMetaEvent(3, "Jazz Architect Chords");
        trackName.setTimeStamp(0);
        track.addEvent(trackName);

        // Convert beats to ticks
        int ticksPerBeat = 480;

        // Add chord notes
        int currentTick = 0;
        int channel = 1;  // MIDI channel 1

        for (const auto& chord : chords) {
            // Use individual chord duration (or fallback to beatsPerChord parameter)
            double chordDuration = chord.getDuration();
            int ticksPerChord = static_cast<int>(chordDuration * ticksPerBeat);

            auto notes = chord.getMIDINotes(baseOctave);

            // Note on events
            for (int note : notes) {
                juce::MidiMessage noteOn = juce::MidiMessage::noteOn(channel, note, 0.8f);
                noteOn.setTimeStamp(currentTick);
                track.addEvent(noteOn);
            }

            // Note off events (slightly before next chord)
            int noteOffTick = currentTick + ticksPerChord - 10;
            for (int note : notes) {
                juce::MidiMessage noteOff = juce::MidiMessage::noteOff(channel, note);
                noteOff.setTimeStamp(noteOffTick);
                track.addEvent(noteOff);
            }

            currentTick += ticksPerChord;
        }

        // Add end of track
        juce::MidiMessage endOfTrack = juce::MidiMessage::endOfTrack();
        endOfTrack.setTimeStamp(currentTick);
        track.addEvent(endOfTrack);

        midiFile.addTrack(track);

        // Write to file
        juce::FileOutputStream outputStream(file);
        if (!outputStream.openedOk()) {
            return false;
        }

        return midiFile.writeTo(outputStream);
    }

    /**
     * Create a MidiMessageSequence from a chord progression
     * (for real-time MIDI output)
     */
    static juce::MidiMessageSequence createSequence(
        const std::vector<ChordSymbol>& chords,
        double bpm = 120.0,
        double beatsPerChord = 2.0,
        int baseOctave = 3)
    {
        juce::MidiMessageSequence sequence;

        int ticksPerBeat = 480;
        int currentTick = 0;
        int channel = 1;

        for (const auto& chord : chords) {
            // Use individual chord duration
            double chordDuration = chord.getDuration();
            int ticksPerChord = static_cast<int>(chordDuration * ticksPerBeat);

            auto notes = chord.getMIDINotes(baseOctave);

            for (int note : notes) {
                juce::MidiMessage noteOn = juce::MidiMessage::noteOn(channel, note, 0.8f);
                noteOn.setTimeStamp(currentTick);
                sequence.addEvent(noteOn);

                juce::MidiMessage noteOff = juce::MidiMessage::noteOff(channel, note);
                noteOff.setTimeStamp(currentTick + ticksPerChord - 10);
                sequence.addEvent(noteOff);
            }

            currentTick += ticksPerChord;
        }

        return sequence;
    }

    /**
     * Get chord as text annotation (for markers)
     */
    static juce::String chordToMarker(const ChordSymbol& chord) {
        return juce::String(chord.toString());
    }
};

} // namespace JazzArchitect
