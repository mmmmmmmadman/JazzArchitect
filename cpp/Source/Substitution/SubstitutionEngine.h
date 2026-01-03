#pragma once

#include "TritoneSubstitution.h"
#include "BackdoorSubstitution.h"
#include "ColtraneSubstitution.h"
#include "../Style/StyleVector.h"
#include <vector>

namespace JazzArchitect {

/**
 * Unified substitution engine that applies multiple substitution
 * rules based on style parameters.
 */
class SubstitutionEngine {
public:
    /**
     * Apply all substitutions to a progression based on style
     */
    static std::vector<ChordSymbol> apply(
        const std::vector<ChordSymbol>& progression,
        const StyleVector& style)
    {
        std::vector<ChordSymbol> result = progression;

        // Apply substitutions in order of complexity
        // (simpler substitutions first)

        // 1. Backdoor dominant (based on modal interchange)
        if (style.modalInterchange > 0.2f) {
            float prob = style.modalInterchange * 0.3f;
            result = BackdoorSubstitution::apply(result, prob);
        }

        // 2. Tritone substitution (chromatic approach)
        if (style.tritoneSubProb > 0.1f) {
            result = TritoneSubstitution::apply(result, style.tritoneSubProb);
        }

        // 3. Coltrane changes (high complexity, rare)
        // Only apply in styles with high complexity and deep dominant chains
        if (style.dominantChainDepth >= 4 && style.chromaticApproach > 0.4f) {
            float prob = style.chromaticApproach * 0.15f;
            result = ColtraneSubstitution::apply(result, prob);
        }

        return result;
    }

    /**
     * Apply only tritone substitutions
     */
    static std::vector<ChordSymbol> applyTritone(
        const std::vector<ChordSymbol>& progression,
        float probability)
    {
        return TritoneSubstitution::apply(progression, probability);
    }

    /**
     * Apply only backdoor substitutions
     */
    static std::vector<ChordSymbol> applyBackdoor(
        const std::vector<ChordSymbol>& progression,
        float probability)
    {
        return BackdoorSubstitution::apply(progression, probability);
    }

    /**
     * Apply only Coltrane substitutions
     */
    static std::vector<ChordSymbol> applyColtrane(
        const std::vector<ChordSymbol>& progression,
        float probability)
    {
        return ColtraneSubstitution::apply(progression, probability);
    }
};

} // namespace JazzArchitect
