#pragma once

#include "../Core/ChordSymbol.h"
#include <vector>
#include <algorithm>
#include <cmath>

namespace JazzArchitect {

/**
 * Voice Leading Motion Types (based on Smither 2019)
 */
enum class VoiceLeadingType {
    COMMON_TONE,    // Note stays the same (0 semitones)
    STEP,           // Half or whole step (1-2 semitones)
    SKIP,           // Third (3-4 semitones)
    LEAP            // Fourth or larger (5+ semitones)
};

/**
 * A guide tone (3rd or 7th of a chord)
 */
struct GuideTone {
    PitchClass pitch;
    bool isThird;       // true = 3rd, false = 7th
    ChordSymbol chord;

    GuideTone(PitchClass p, bool third, const ChordSymbol& c)
        : pitch(p), isThird(third), chord(c) {}
};

/**
 * Connection between two guide tones
 */
struct VoiceLeadingConnection {
    GuideTone fromTone;
    GuideTone toTone;
    int interval;           // Semitones (0-6, minimum distance)
    VoiceLeadingType motion;
    float cost;

    VoiceLeadingConnection(const GuideTone& from, const GuideTone& to, int iv, VoiceLeadingType m, float c)
        : fromTone(from), toTone(to), interval(iv), motion(m), cost(c) {}
};

/**
 * Complete voice leading analysis for a progression
 */
struct VoiceLeadingAnalysis {
    std::vector<ChordSymbol> chords;
    std::vector<std::vector<VoiceLeadingConnection>> connections;
    float totalCost;
    float averageCost;
    int smoothTransitions;  // cost <= 2
    int roughTransitions;   // cost > 4
};

/**
 * Calculate minimum interval between two pitch classes (0-6 semitones)
 */
inline int minInterval(const PitchClass& pc1, const PitchClass& pc2) {
    int forward = (pc2.value() - pc1.value() + 12) % 12;
    int backward = (pc1.value() - pc2.value() + 12) % 12;
    return std::min(forward, backward);
}

/**
 * Classify the type of motion based on interval
 */
inline VoiceLeadingType classifyMotion(int interval) {
    if (interval == 0) return VoiceLeadingType::COMMON_TONE;
    if (interval <= 2) return VoiceLeadingType::STEP;
    if (interval <= 4) return VoiceLeadingType::SKIP;
    return VoiceLeadingType::LEAP;
}

/**
 * Calculate voice leading cost between two chords based on guide tones.
 *
 * Lower cost = smoother voice leading.
 * Optimal progressions have cost <= 2.
 *
 * Based on Smither (2019) guide-tone voice leading theory.
 */
inline float voiceLeadingCost(const ChordSymbol& chord1, const ChordSymbol& chord2) {
    PitchClass third1 = chord1.third();
    PitchClass seventh1 = chord1.seventh();
    PitchClass third2 = chord2.third();
    PitchClass seventh2 = chord2.seventh();

    // Calculate all possible voice leading paths
    // Path 1: 3rd -> 3rd, 7th -> 7th
    float cost1 = static_cast<float>(minInterval(third1, third2) + minInterval(seventh1, seventh2));

    // Path 2: 3rd -> 7th, 7th -> 3rd (voice exchange)
    float cost2 = static_cast<float>(minInterval(third1, seventh2) + minInterval(seventh1, third2));

    return std::min(cost1, cost2);
}

/**
 * Analyze voice leading connections between two chords
 */
inline std::vector<VoiceLeadingConnection> analyzeVoiceLeading(
    const ChordSymbol& chord1,
    const ChordSymbol& chord2)
{
    std::vector<VoiceLeadingConnection> connections;

    GuideTone gt1_3(chord1.third(), true, chord1);
    GuideTone gt1_7(chord1.seventh(), false, chord1);
    GuideTone gt2_3(chord2.third(), true, chord2);
    GuideTone gt2_7(chord2.seventh(), false, chord2);

    std::vector<GuideTone> fromTones = {gt1_3, gt1_7};
    std::vector<GuideTone> toTones = {gt2_3, gt2_7};

    for (const auto& from : fromTones) {
        for (const auto& to : toTones) {
            int interval = minInterval(from.pitch, to.pitch);
            VoiceLeadingType motion = classifyMotion(interval);

            // Cost calculation: smooth motion is cheaper
            float cost = static_cast<float>(interval);
            if (motion == VoiceLeadingType::SKIP || motion == VoiceLeadingType::LEAP) {
                cost *= 1.5f;
            }

            connections.emplace_back(from, to, interval, motion, cost);
        }
    }

    return connections;
}

/**
 * Calculate total voice leading cost for a progression
 */
inline float progressionVoiceLeadingCost(const std::vector<ChordSymbol>& chords) {
    if (chords.size() < 2) return 0.0f;

    float total = 0.0f;
    for (size_t i = 0; i < chords.size() - 1; ++i) {
        total += voiceLeadingCost(chords[i], chords[i + 1]);
    }

    return total;
}

/**
 * Calculate average voice leading cost per transition
 */
inline float averageVoiceLeadingCost(const std::vector<ChordSymbol>& chords) {
    if (chords.size() < 2) return 0.0f;
    return progressionVoiceLeadingCost(chords) / static_cast<float>(chords.size() - 1);
}

/**
 * Perform complete voice leading analysis on a progression
 */
inline VoiceLeadingAnalysis analyzeProgression(const std::vector<ChordSymbol>& chords) {
    VoiceLeadingAnalysis analysis;
    analysis.chords = chords;
    analysis.smoothTransitions = 0;
    analysis.roughTransitions = 0;

    for (size_t i = 0; i < chords.size() - 1; ++i) {
        auto connections = analyzeVoiceLeading(chords[i], chords[i + 1]);
        analysis.connections.push_back(connections);

        float cost = voiceLeadingCost(chords[i], chords[i + 1]);
        if (cost <= 2.0f) {
            analysis.smoothTransitions++;
        } else if (cost > 4.0f) {
            analysis.roughTransitions++;
        }
    }

    analysis.totalCost = progressionVoiceLeadingCost(chords);
    analysis.averageCost = averageVoiceLeadingCost(chords);

    return analysis;
}

/**
 * Check if a two-chord progression has smooth voice leading
 */
inline bool isSmoothProgression(const ChordSymbol& chord1, const ChordSymbol& chord2) {
    return voiceLeadingCost(chord1, chord2) <= 2.0f;
}

/**
 * Find the smoothest guide tone voicing given previous guide tones.
 * Returns pair of (3rd, 7th) or (7th, 3rd) depending on what minimizes motion.
 */
inline std::pair<PitchClass, PitchClass> findSmoothestVoicing(
    const ChordSymbol& chord,
    const std::pair<PitchClass, PitchClass>& prevGuideTones)
{
    PitchClass third = chord.third();
    PitchClass seventh = chord.seventh();

    PitchClass prev3 = prevGuideTones.first;
    PitchClass prev7 = prevGuideTones.second;

    // Option 1: 3rd -> 3rd, 7th -> 7th
    int cost1 = minInterval(prev3, third) + minInterval(prev7, seventh);

    // Option 2: Exchange (3rd -> 7th, 7th -> 3rd)
    int cost2 = minInterval(prev3, seventh) + minInterval(prev7, third);

    if (cost1 <= cost2) {
        return {third, seventh};
    } else {
        return {seventh, third};
    }
}

/**
 * Calculate voice leading quality score (0-100)
 * Higher is better
 */
inline float voiceLeadingQuality(const std::vector<ChordSymbol>& chords) {
    if (chords.size() < 2) return 100.0f;

    float avgCost = averageVoiceLeadingCost(chords);

    // Map cost to quality: 0 cost = 100 quality, 6+ cost = 0 quality
    float quality = std::max(0.0f, 100.0f - (avgCost * 16.67f));
    return quality;
}

} // namespace JazzArchitect
