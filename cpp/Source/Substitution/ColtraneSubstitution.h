#pragma once

#include "../Core/ChordSymbol.h"
#include <vector>
#include <random>

namespace JazzArchitect {

/**
 * Coltrane Changes (Giant Steps pattern)
 * Divides the octave into three equal parts (major thirds)
 * Creates a cycle: I -> III7 -> bVI -> bII7 -> I
 *
 * The three tonal centers are related by major thirds:
 * C -> E -> Ab -> C (0 -> 4 -> 8 -> 12)
 *
 * Each tonal center is approached by its V7
 */
class ColtraneSubstitution {
public:
    /**
     * Generate Coltrane changes from a tonic
     * Returns: V7/III -> III -> V7/bVI -> bVI -> V7/I -> I
     */
    static std::vector<ChordSymbol> generateColtrane(const PitchClass& tonicRoot) {
        std::vector<ChordSymbol> result;

        // Three tonal centers: I, III (up M3), bVI (up another M3)
        PitchClass center1 = tonicRoot;                    // I
        PitchClass center2 = tonicRoot.transpose(4);       // III (E from C)
        PitchClass center3 = tonicRoot.transpose(8);       // bVI (Ab from C)

        // V7 of III (B7 resolves to E)
        PitchClass v7_of_iii = center2.transpose(7);
        result.push_back(ChordSymbol(v7_of_iii, ChordQuality::DOM7));

        // III major
        result.push_back(ChordSymbol(center2, ChordQuality::MAJ7));

        // V7 of bVI (Eb7 resolves to Ab)
        PitchClass v7_of_bvi = center3.transpose(7);
        result.push_back(ChordSymbol(v7_of_bvi, ChordQuality::DOM7));

        // bVI major
        result.push_back(ChordSymbol(center3, ChordQuality::MAJ7));

        // V7 of I (G7 resolves to C)
        PitchClass v7_of_i = center1.transpose(7);
        result.push_back(ChordSymbol(v7_of_i, ChordQuality::DOM7));

        // I major
        result.push_back(ChordSymbol(center1, ChordQuality::MAJ7));

        return result;
    }

    /**
     * Substitute a ii-V-I with Coltrane changes
     * ii-V-I becomes: ii -> V7/III -> III -> V7/bVI -> bVI -> V7 -> I
     */
    static std::vector<ChordSymbol> substituteIiVI(
        const ChordSymbol& ii,
        const ChordSymbol& v,
        const ChordSymbol& i)
    {
        std::vector<ChordSymbol> result;

        // Keep the ii
        result.push_back(ii);

        // Get the tonic root
        PitchClass tonicRoot = i.root();

        // Insert shortened Coltrane cycle
        PitchClass center2 = tonicRoot.transpose(4);  // III
        PitchClass center3 = tonicRoot.transpose(8);  // bVI

        // V7/III -> III
        result.push_back(ChordSymbol(center2.transpose(7), ChordQuality::DOM7));
        result.push_back(ChordSymbol(center2, ChordQuality::MAJ7));

        // V7/bVI -> bVI
        result.push_back(ChordSymbol(center3.transpose(7), ChordQuality::DOM7));
        result.push_back(ChordSymbol(center3, ChordQuality::MAJ7));

        // Original V -> I
        result.push_back(v);
        result.push_back(i);

        return result;
    }

    /**
     * Apply Coltrane substitution to major chords in a progression
     */
    static std::vector<ChordSymbol> apply(
        const std::vector<ChordSymbol>& progression,
        float probability = 0.15f)
    {
        std::vector<ChordSymbol> result;

        for (size_t i = 0; i < progression.size(); ++i) {
            const auto& chord = progression[i];

            // Look for ii-V-I pattern
            if (i + 2 < progression.size() && shouldSubstitute(probability)) {
                const auto& next1 = progression[i + 1];
                const auto& next2 = progression[i + 2];

                if (isIiVI(chord, next1, next2)) {
                    auto coltrane = substituteIiVI(chord, next1, next2);
                    result.insert(result.end(), coltrane.begin(), coltrane.end());
                    i += 2; // Skip the original V and I
                    continue;
                }
            }

            result.push_back(chord);
        }

        return result;
    }

    /**
     * Check if three chords form a ii-V-I pattern
     */
    static bool isIiVI(
        const ChordSymbol& c1,
        const ChordSymbol& c2,
        const ChordSymbol& c3)
    {
        // Check ii (min7, root is 2 semitones above tonic)
        if (c1.quality() != ChordQuality::MIN7) return false;

        // Check V (dom7, root is 7 semitones above tonic)
        if (c2.quality() != ChordQuality::DOM7) return false;

        // Check I (maj7)
        if (c3.quality() != ChordQuality::MAJ7 && c3.quality() != ChordQuality::MAJ6) return false;

        // Verify relationships
        int tonicRoot = c3.root().value();
        int expectedIi = (tonicRoot + 2) % 12;
        int expectedV = (tonicRoot + 7) % 12;

        return c1.root().value() == expectedIi && c2.root().value() == expectedV;
    }

private:
    static bool shouldSubstitute(float probability) {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(rng) < probability;
    }
};

} // namespace JazzArchitect
