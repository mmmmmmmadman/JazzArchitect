#pragma once

#include "../Core/ChordSymbol.h"
#include <vector>
#include <random>

namespace JazzArchitect {

/**
 * Backdoor Dominant Substitution
 * Uses bVII7 to resolve to I instead of V7
 * Example: Bb7 -> Cmaj7 (instead of G7 -> Cmaj7)
 * The bVII7 is derived from the borrowed iv chord in melodic minor
 */
class BackdoorSubstitution {
public:
    /**
     * Create a backdoor dominant for a given tonic
     * @param tonicRoot The root of the target tonic chord
     * @return The bVII7 chord
     */
    static ChordSymbol createBackdoor(const PitchClass& tonicRoot) {
        // bVII is 10 semitones above tonic (or 2 below)
        PitchClass backdoorRoot = tonicRoot.transpose(10);
        return ChordSymbol(backdoorRoot, ChordQuality::DOM7);
    }

    /**
     * Check if a chord could be replaced with backdoor dominant
     * Only applies when V7 resolves to I
     */
    static bool canApplyBackdoor(
        const ChordSymbol& chord,
        const ChordSymbol& nextChord)
    {
        if (!isDominant(chord)) {
            return false;
        }

        // Check if current chord is V of next chord
        // V is 7 semitones above root
        int expectedVRoot = (nextChord.root().value() + 7) % 12;
        return chord.root().value() == expectedVRoot && isTonic(nextChord);
    }

    /**
     * Apply backdoor substitution to a progression
     */
    static std::vector<ChordSymbol> apply(
        const std::vector<ChordSymbol>& progression,
        float probability = 0.2f)
    {
        std::vector<ChordSymbol> result;
        result.reserve(progression.size());

        for (size_t i = 0; i < progression.size(); ++i) {
            const auto& chord = progression[i];

            if (i + 1 < progression.size()) {
                const auto& nextChord = progression[i + 1];

                if (canApplyBackdoor(chord, nextChord) && shouldSubstitute(probability)) {
                    result.push_back(createBackdoor(nextChord.root()));
                    continue;
                }
            }

            result.push_back(chord);
        }

        return result;
    }

private:
    static bool isDominant(const ChordSymbol& chord) {
        return chord.quality() == ChordQuality::DOM7;
    }

    static bool isTonic(const ChordSymbol& chord) {
        return chord.quality() == ChordQuality::MAJ7 ||
               chord.quality() == ChordQuality::MAJ6;
    }

    static bool shouldSubstitute(float probability) {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(rng) < probability;
    }
};

} // namespace JazzArchitect
