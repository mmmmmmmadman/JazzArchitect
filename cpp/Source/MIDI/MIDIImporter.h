#pragma once

#include <JuceHeader.h>
#include "../Core/ChordSymbol.h"
#include "../Core/ChordQuality.h"
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>

namespace JazzArchitect {

/**
 * MIDI File Importer
 * Imports chord progressions from Standard MIDI Files
 */
class MIDIImporter {
public:
    struct ImportResult {
        std::vector<ChordSymbol> chords;
        double bpm = 120.0;
        bool success = false;
        std::string errorMessage;
    };

    /**
     * Import a chord progression from a MIDI file
     * @param file The source MIDI file
     * @return ImportResult containing the chord progression or error message
     */
    static ImportResult importFromFile(const juce::File& file)
    {
        ImportResult result;

        // Read MIDI file
        juce::FileInputStream inputStream(file);
        if (!inputStream.openedOk()) {
            result.errorMessage = "Could not open file";
            return result;
        }

        juce::MidiFile midiFile;
        if (!midiFile.readFrom(inputStream, true)) {
            result.errorMessage = "Invalid MIDI file format";
            return result;
        }

        // Get tempo from MIDI file
        result.bpm = extractTempo(midiFile);

        // Get ticks per quarter note
        int ticksPerQuarterNote = midiFile.getTimeFormat();
        if (ticksPerQuarterNote <= 0) ticksPerQuarterNote = 480;

        // Collect all note events from all tracks
        std::vector<NoteEvent> allNotes;
        for (int trackIdx = 0; trackIdx < midiFile.getNumTracks(); ++trackIdx) {
            const auto* track = midiFile.getTrack(trackIdx);
            if (track == nullptr) continue;

            extractNotesFromTrack(track, allNotes);
        }

        if (allNotes.empty()) {
            result.errorMessage = "No notes found in MIDI file";
            return result;
        }

        // Sort notes by start time
        std::sort(allNotes.begin(), allNotes.end(),
                  [](const NoteEvent& a, const NoteEvent& b) {
                      return a.startTick < b.startTick;
                  });

        // Cluster notes into chords (notes starting within tolerance)
        double toleranceTicks = ticksPerQuarterNote * 0.1;  // 10% of a beat
        auto clusters = clusterNotes(allNotes, toleranceTicks);

        // Recognize chords from clusters
        for (const auto& cluster : clusters) {
            ChordSymbol chord = recognizeChord(cluster.notes);

            // Calculate duration in beats
            double durationTicks = cluster.endTick - cluster.startTick;
            double durationBeats = durationTicks / ticksPerQuarterNote;
            // Snap to nearest 0.5 beat
            durationBeats = std::round(durationBeats * 2.0) / 2.0;
            durationBeats = std::max(0.5, std::min(8.0, durationBeats));
            chord.setDuration(durationBeats);

            result.chords.push_back(chord);
        }

        if (result.chords.empty()) {
            result.errorMessage = "Could not recognize any chords";
            return result;
        }

        result.success = true;
        return result;
    }

    /**
     * Recognize a chord from MIDI notes
     * @param midiNotes Vector of MIDI note numbers
     * @return Recognized ChordSymbol
     */
    static ChordSymbol recognizeChordFromNotes(const std::vector<int>& midiNotes)
    {
        return recognizeChord(midiNotes);
    }

private:
    struct NoteEvent {
        int midiNote;
        double startTick;
        double endTick;
    };

    struct NoteCluster {
        std::vector<int> notes;
        double startTick;
        double endTick;
    };

    static double extractTempo(const juce::MidiFile& midiFile)
    {
        // Default tempo
        double bpm = 120.0;

        for (int trackIdx = 0; trackIdx < midiFile.getNumTracks(); ++trackIdx) {
            const auto* track = midiFile.getTrack(trackIdx);
            if (track == nullptr) continue;

            for (int eventIdx = 0; eventIdx < track->getNumEvents(); ++eventIdx) {
                const auto& event = track->getEventPointer(eventIdx)->message;
                if (event.isTempoMetaEvent()) {
                    double microsPerBeat = event.getTempoSecondsPerQuarterNote() * 1000000.0;
                    if (microsPerBeat > 0) {
                        bpm = 60000000.0 / microsPerBeat;
                    }
                    return bpm;  // Use first tempo event
                }
            }
        }

        return bpm;
    }

    static void extractNotesFromTrack(const juce::MidiMessageSequence* track,
                                      std::vector<NoteEvent>& notes)
    {
        for (int eventIdx = 0; eventIdx < track->getNumEvents(); ++eventIdx) {
            const auto* holder = track->getEventPointer(eventIdx);
            const auto& msg = holder->message;

            if (msg.isNoteOn() && msg.getVelocity() > 0) {
                NoteEvent event;
                event.midiNote = msg.getNoteNumber();
                event.startTick = msg.getTimeStamp();

                // Find matching note off
                double endTick = event.startTick + 480;  // Default 1 beat
                int matchIdx = track->getIndexOfMatchingKeyUp(eventIdx);
                if (matchIdx >= 0) {
                    endTick = track->getEventPointer(matchIdx)->message.getTimeStamp();
                }
                event.endTick = endTick;

                notes.push_back(event);
            }
        }
    }

    static std::vector<NoteCluster> clusterNotes(const std::vector<NoteEvent>& notes,
                                                  double toleranceTicks)
    {
        std::vector<NoteCluster> clusters;
        if (notes.empty()) return clusters;

        NoteCluster currentCluster;
        currentCluster.startTick = notes[0].startTick;
        currentCluster.endTick = notes[0].endTick;
        currentCluster.notes.push_back(notes[0].midiNote);

        for (size_t i = 1; i < notes.size(); ++i) {
            double timeDiff = notes[i].startTick - currentCluster.startTick;

            if (timeDiff <= toleranceTicks) {
                // Same chord
                currentCluster.notes.push_back(notes[i].midiNote);
                currentCluster.endTick = std::max(currentCluster.endTick, notes[i].endTick);
            } else {
                // New chord - save current and start new
                clusters.push_back(currentCluster);

                currentCluster.notes.clear();
                currentCluster.startTick = notes[i].startTick;
                currentCluster.endTick = notes[i].endTick;
                currentCluster.notes.push_back(notes[i].midiNote);
            }
        }

        // Add last cluster
        if (!currentCluster.notes.empty()) {
            clusters.push_back(currentCluster);
        }

        return clusters;
    }

    static ChordSymbol recognizeChord(const std::vector<int>& midiNotes)
    {
        if (midiNotes.empty()) {
            return ChordSymbol(PitchClass(0), ChordQuality::MAJ7);
        }

        // Convert to pitch class set (0-11)
        std::set<int> pitchClasses;
        for (int note : midiNotes) {
            pitchClasses.insert(note % 12);
        }

        // Find lowest note as potential bass
        int lowestNote = *std::min_element(midiNotes.begin(), midiNotes.end());
        int bassPC = lowestNote % 12;

        // Chord templates (intervals from root)
        struct ChordTemplate {
            ChordQuality quality;
            std::vector<int> intervals;
            int score;  // Higher = more common/preferred
        };

        std::vector<ChordTemplate> templates = {
            { ChordQuality::MAJ7,  {0, 4, 7, 11}, 10 },
            { ChordQuality::MIN7,  {0, 3, 7, 10}, 10 },
            { ChordQuality::DOM7,  {0, 4, 7, 10}, 12 },  // Most common in jazz
            { ChordQuality::HDIM7, {0, 3, 6, 10}, 8 },
            { ChordQuality::DIM7,  {0, 3, 6, 9}, 6 },
            { ChordQuality::MIN_MAJ7, {0, 3, 7, 11}, 5 },
            { ChordQuality::AUG,   {0, 4, 8}, 4 },
            { ChordQuality::MAJ6,  {0, 4, 7, 9}, 7 },
            { ChordQuality::MIN6,  {0, 3, 7, 9}, 7 },
            { ChordQuality::SUS4,  {0, 5, 7, 10}, 6 },
            { ChordQuality::SUS2,  {0, 2, 7, 10}, 6 },
        };

        int bestRoot = bassPC;  // Default to bass note as root
        ChordQuality bestQuality = ChordQuality::DOM7;
        int bestScore = -1;

        // Try each possible root
        for (int root = 0; root < 12; ++root) {
            // Calculate intervals from this root
            std::set<int> intervals;
            for (int pc : pitchClasses) {
                intervals.insert((pc - root + 12) % 12);
            }

            // Try each template
            for (const auto& tmpl : templates) {
                int matchCount = 0;
                for (int interval : tmpl.intervals) {
                    if (intervals.count(interval) > 0) {
                        matchCount++;
                    }
                }

                // Calculate score based on match percentage and template preference
                int score = matchCount * 10 + tmpl.score;

                // Bonus if root matches bass note
                if (root == bassPC) {
                    score += 5;
                }

                // Require at least 3 matching notes for 4-note chords
                if (tmpl.intervals.size() >= 4 && matchCount >= 3 && score > bestScore) {
                    bestScore = score;
                    bestRoot = root;
                    bestQuality = tmpl.quality;
                }
                // For triads, require all notes
                else if (tmpl.intervals.size() == 3 && matchCount == 3 && score > bestScore) {
                    bestScore = score;
                    bestRoot = root;
                    bestQuality = tmpl.quality;
                }
            }
        }

        return ChordSymbol(PitchClass(bestRoot), bestQuality);
    }
};

} // namespace JazzArchitect
