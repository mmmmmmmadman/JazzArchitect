#pragma once

#include "../Core/ChordSymbol.h"
#include <vector>
#include <random>

namespace JazzArchitect {

/**
 * Tritone Substitution
 * Replaces a dominant chord with one whose root is a tritone away.
 * G7 -> Db7 (both share the same tritone: B-F / Cb-F)
 */
class TritoneSubstitution {
public:
    /**
     * Apply tritone substitution to a single chord
     * Only applies to dominant 7th chords
     */
    static ChordSymbol substitute(const ChordSymbol& chord) {
        if (!isDominant(chord)) {
            return chord;
        }

        // Transpose root by tritone (6 semitones)
        PitchClass newRoot = chord.root().transpose(6);
        return ChordSymbol(newRoot, chord.quality());
    }

    /**
     * Apply tritone substitution to a progression with probability
     */
    static std::vector<ChordSymbol> apply(
        const std::vector<ChordSymbol>& progression,
        float probability = 0.3f)
    {
        std::vector<ChordSymbol> result;
        result.reserve(progression.size());

        for (size_t i = 0; i < progression.size(); ++i) {
            const auto& chord = progression[i];

            if (isDominant(chord) && shouldSubstitute(probability)) {
                // Check if next chord is the expected resolution
                bool resolvesDown = false;
                if (i + 1 < progression.size()) {
                    int nextRoot = progression[i + 1].root().value();
                    int curRoot = chord.root().value();
                    int interval = (nextRoot - curRoot + 12) % 12;
                    resolvesDown = (interval == 5 || interval == 7); // Perfect 4th or 5th
                }

                if (resolvesDown) {
                    result.push_back(substitute(chord));
                } else {
                    result.push_back(chord);
                }
            } else {
                result.push_back(chord);
            }
        }

        return result;
    }

    /**
     * Check if chord is a dominant type
     */
    static bool isDominant(const ChordSymbol& chord) {
        return chord.quality() == ChordQuality::DOM7;
    }

private:
    static bool shouldSubstitute(float probability) {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(rng) < probability;
    }
};

} // namespace JazzArchitect
