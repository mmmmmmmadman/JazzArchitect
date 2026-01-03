#pragma once

#include "GuideTone.h"
#include "../Substitution/TritoneSubstitution.h"
#include <vector>
#include <optional>

namespace JazzArchitect {

/**
 * Voice Leading Optimizer
 * Uses local search to find substitutions that improve voice leading.
 */
class VoiceLeadingOptimizer {
public:
    /**
     * Optimize a progression for better voice leading.
     * Returns the optimized progression.
     */
    static std::vector<ChordSymbol> optimize(
        const std::vector<ChordSymbol>& chords,
        int maxIterations = 100)
    {
        std::vector<ChordSymbol> current = chords;
        float currentCost = progressionVoiceLeadingCost(current);

        for (int iter = 0; iter < maxIterations; ++iter) {
            bool improved = false;

            for (size_t i = 0; i < current.size(); ++i) {
                const auto& chord = current[i];

                // Try tritone substitution for dominant chords
                if (TritoneSubstitution::isDominant(chord)) {
                    std::vector<ChordSymbol> test = current;
                    test[i] = TritoneSubstitution::substitute(chord);

                    float newCost = progressionVoiceLeadingCost(test);
                    if (newCost < currentCost) {
                        current = test;
                        currentCost = newCost;
                        improved = true;
                        break;
                    }
                }
            }

            if (!improved) {
                break;
            }
        }

        return current;
    }

    /**
     * Suggest a connecting chord to improve voice leading between two chords.
     * Returns nullopt if no improvement is possible.
     */
    static std::optional<ChordSymbol> suggestConnectingChord(
        const ChordSymbol& chord1,
        const ChordSymbol& chord2,
        const PitchClass& key)
    {
        float directCost = voiceLeadingCost(chord1, chord2);

        if (directCost <= 2.0f) {
            return std::nullopt;  // Already smooth
        }

        std::vector<std::pair<ChordSymbol, float>> candidates;

        // Try passing diminished
        PitchClass dimRoot = chord1.root().transpose(1);
        ChordSymbol dim(dimRoot, ChordQuality::DIM7);
        float costViaDim = voiceLeadingCost(chord1, dim) + voiceLeadingCost(dim, chord2);
        if (costViaDim < directCost) {
            candidates.push_back({dim, costViaDim});
        }

        // Try secondary dominant
        PitchClass vRoot = chord2.root().transpose(7);
        ChordSymbol secDom(vRoot, ChordQuality::DOM7);
        float costViaV = voiceLeadingCost(chord1, secDom) + voiceLeadingCost(secDom, chord2);
        if (costViaV < directCost) {
            candidates.push_back({secDom, costViaV});
        }

        // Try ii of target
        PitchClass iiRoot = chord2.root().transpose(2);
        ChordSymbol ii(iiRoot, ChordQuality::MIN7);
        float costViaII = voiceLeadingCost(chord1, ii) + voiceLeadingCost(ii, chord2);
        if (costViaII < directCost) {
            candidates.push_back({ii, costViaII});
        }

        if (candidates.empty()) {
            return std::nullopt;
        }

        // Return the best candidate
        auto best = std::min_element(candidates.begin(), candidates.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });

        return best->first;
    }

    /**
     * Insert passing chords to improve rough transitions
     */
    static std::vector<ChordSymbol> insertPassingChords(
        const std::vector<ChordSymbol>& chords,
        const PitchClass& key)
    {
        if (chords.size() < 2) {
            return chords;
        }

        std::vector<ChordSymbol> result;
        result.push_back(chords[0]);

        for (size_t i = 1; i < chords.size(); ++i) {
            const auto& prev = chords[i - 1];
            const auto& curr = chords[i];

            auto passing = suggestConnectingChord(prev, curr, key);
            if (passing.has_value()) {
                result.push_back(passing.value());
            }

            result.push_back(curr);
        }

        return result;
    }
};

} // namespace JazzArchitect
