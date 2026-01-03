#pragma once

#include "StyleVector.h"
#include "StylePresets.h"
#include "../Grammar/PCFG.h"
#include "../Grammar/Generator.h"
#include "../Core/ChordSymbol.h"
#include <vector>
#include <memory>

namespace JazzArchitect {

/**
 * Engine that generates progressions according to style parameters.
 * Integrates StyleVector with PCFG grammar generation.
 */
class StyleEngine {
public:
    StyleEngine();
    explicit StyleEngine(const StyleVector& style);

    /**
     * Set the style and update grammar probabilities
     */
    void setStyle(const StyleVector& style);

    /**
     * Set style by preset ID
     */
    void setStyleById(int id);

    /**
     * Get current style
     */
    const StyleVector& getStyle() const { return style_; }

    /**
     * Generate a chord progression
     * @param length Number of chords
     * @param key Key pitch class (0-11)
     * @return Vector of ChordSymbols
     */
    std::vector<ChordSymbol> generate(int length = 8, int key = 0);

    /**
     * Get the styled grammar
     */
    PCFG& getGrammar() { return grammar_; }

    /**
     * Update individual style parameters (for real-time control)
     */
    void setTritoneSubProb(float value);
    void setIiVPreference(float value);
    void setModalInterchange(float value);
    void setExtensionLevel(float value);

private:
    StyleVector style_;
    PCFG grammar_;
    std::unique_ptr<HarmonyGenerator> generator_;

    /**
     * Update grammar probabilities based on current style
     */
    void updateGrammar();

    /**
     * Adjust progression to target length
     */
    std::vector<ChordSymbol> adjustLength(
        std::vector<ChordSymbol>& chords,
        int target,
        int key);
};

/**
 * Convert a style vector to PCFG rule probabilities
 */
PCFG styleToPCFG(const StyleVector& style);

} // namespace JazzArchitect
