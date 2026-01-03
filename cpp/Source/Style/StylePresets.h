#pragma once

#include "StyleVector.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace JazzArchitect {

/**
 * Predefined style presets for different jazz eras
 */
namespace StylePresets {

// Classic Era (1930s-1940s)
inline StyleVector SWING() {
    StyleVector s;
    s.tritoneSubProb = 0.1f;
    s.backdoorProb = 0.1f;
    s.coltraneProb = 0.0f;
    s.iiVPreference = 0.7f;
    s.secondaryDomProb = 0.2f;
    s.modalInterchange = 0.1f;
    s.minorIvProb = 0.1f;
    s.chromaticApproach = 0.1f;
    s.diminishedApproach = 0.15f;
    s.dominantChainDepth = 2;
    s.prolongationDepth = 1;
    s.rhythmDensity = 0.4f;
    s.turnaroundProb = 0.5f;
    s.extensionLevel = 0.3f;
    s.alterationProb = 0.1f;
    return s;
}

// Bebop Era (1940s-1950s)
inline StyleVector BEBOP() {
    StyleVector s;
    s.tritoneSubProb = 0.3f;
    s.backdoorProb = 0.15f;
    s.coltraneProb = 0.05f;
    s.iiVPreference = 0.9f;
    s.secondaryDomProb = 0.4f;
    s.modalInterchange = 0.2f;
    s.minorIvProb = 0.15f;
    s.chromaticApproach = 0.4f;
    s.diminishedApproach = 0.2f;
    s.dominantChainDepth = 4;
    s.prolongationDepth = 2;
    s.rhythmDensity = 0.8f;
    s.turnaroundProb = 0.6f;
    s.extensionLevel = 0.5f;
    s.alterationProb = 0.3f;
    return s;
}

// Cool Jazz (1950s)
inline StyleVector COOL() {
    StyleVector s;
    s.tritoneSubProb = 0.2f;
    s.backdoorProb = 0.1f;
    s.coltraneProb = 0.0f;
    s.iiVPreference = 0.7f;
    s.secondaryDomProb = 0.25f;
    s.modalInterchange = 0.3f;
    s.minorIvProb = 0.2f;
    s.chromaticApproach = 0.2f;
    s.diminishedApproach = 0.1f;
    s.dominantChainDepth = 3;
    s.prolongationDepth = 2;
    s.rhythmDensity = 0.5f;
    s.turnaroundProb = 0.4f;
    s.extensionLevel = 0.4f;
    s.alterationProb = 0.15f;
    return s;
}

// Hard Bop (1950s-1960s)
inline StyleVector HARDBOP() {
    StyleVector s;
    s.tritoneSubProb = 0.25f;
    s.backdoorProb = 0.2f;
    s.coltraneProb = 0.1f;
    s.iiVPreference = 0.85f;
    s.secondaryDomProb = 0.35f;
    s.modalInterchange = 0.25f;
    s.minorIvProb = 0.2f;
    s.chromaticApproach = 0.35f;
    s.diminishedApproach = 0.15f;
    s.dominantChainDepth = 4;
    s.prolongationDepth = 2;
    s.rhythmDensity = 0.7f;
    s.turnaroundProb = 0.5f;
    s.extensionLevel = 0.55f;
    s.alterationProb = 0.25f;
    return s;
}

// Modal Jazz (1960s)
inline StyleVector MODAL() {
    StyleVector s;
    s.tritoneSubProb = 0.1f;
    s.backdoorProb = 0.05f;
    s.coltraneProb = 0.15f;
    s.iiVPreference = 0.3f;
    s.secondaryDomProb = 0.1f;
    s.modalInterchange = 0.6f;
    s.minorIvProb = 0.3f;
    s.chromaticApproach = 0.1f;
    s.diminishedApproach = 0.05f;
    s.dominantChainDepth = 2;
    s.prolongationDepth = 3;
    s.rhythmDensity = 0.3f;
    s.turnaroundProb = 0.2f;
    s.extensionLevel = 0.6f;
    s.alterationProb = 0.1f;
    return s;
}

// Post-Bop (1960s-1970s)
inline StyleVector POSTBOP() {
    StyleVector s;
    s.tritoneSubProb = 0.4f;
    s.backdoorProb = 0.25f;
    s.coltraneProb = 0.25f;
    s.iiVPreference = 0.6f;
    s.secondaryDomProb = 0.45f;
    s.modalInterchange = 0.5f;
    s.minorIvProb = 0.3f;
    s.chromaticApproach = 0.5f;
    s.diminishedApproach = 0.2f;
    s.dominantChainDepth = 5;
    s.prolongationDepth = 3;
    s.rhythmDensity = 0.6f;
    s.turnaroundProb = 0.4f;
    s.extensionLevel = 0.7f;
    s.alterationProb = 0.4f;
    return s;
}

// Fusion (1970s)
inline StyleVector FUSION() {
    StyleVector s;
    s.tritoneSubProb = 0.35f;
    s.backdoorProb = 0.2f;
    s.coltraneProb = 0.1f;
    s.iiVPreference = 0.5f;
    s.secondaryDomProb = 0.3f;
    s.modalInterchange = 0.6f;
    s.minorIvProb = 0.35f;
    s.chromaticApproach = 0.4f;
    s.diminishedApproach = 0.15f;
    s.dominantChainDepth = 3;
    s.prolongationDepth = 2;
    s.rhythmDensity = 0.65f;
    s.turnaroundProb = 0.3f;
    s.extensionLevel = 0.75f;
    s.alterationProb = 0.35f;
    return s;
}

// Contemporary/Modern (1980s-present)
inline StyleVector CONTEMPORARY() {
    StyleVector s;
    s.tritoneSubProb = 0.35f;
    s.backdoorProb = 0.2f;
    s.coltraneProb = 0.15f;
    s.iiVPreference = 0.65f;
    s.secondaryDomProb = 0.35f;
    s.modalInterchange = 0.45f;
    s.minorIvProb = 0.25f;
    s.chromaticApproach = 0.35f;
    s.diminishedApproach = 0.15f;
    s.dominantChainDepth = 4;
    s.prolongationDepth = 2;
    s.rhythmDensity = 0.55f;
    s.turnaroundProb = 0.35f;
    s.extensionLevel = 0.65f;
    s.alterationProb = 0.3f;
    return s;
}

// Blues-influenced
inline StyleVector BLUES() {
    StyleVector s;
    s.tritoneSubProb = 0.15f;
    s.backdoorProb = 0.25f;
    s.coltraneProb = 0.0f;
    s.iiVPreference = 0.5f;
    s.secondaryDomProb = 0.2f;
    s.modalInterchange = 0.4f;
    s.minorIvProb = 0.4f;
    s.chromaticApproach = 0.2f;
    s.diminishedApproach = 0.1f;
    s.dominantChainDepth = 2;
    s.prolongationDepth = 1;
    s.rhythmDensity = 0.4f;
    s.turnaroundProb = 0.6f;
    s.extensionLevel = 0.4f;
    s.alterationProb = 0.2f;
    return s;
}

/**
 * Get a style preset by ID (1-9 matching UI selector)
 */
inline StyleVector getStyleById(int id) {
    switch (id) {
        case 1: return BEBOP();
        case 2: return COOL();
        case 3: return MODAL();
        case 4: return HARDBOP();
        case 5: return POSTBOP();
        case 6: return SWING();
        case 7: return FUSION();
        case 8: return CONTEMPORARY();
        case 9: return BLUES();
        default: return BEBOP();
    }
}

/**
 * Get style name by ID
 */
inline std::string getStyleName(int id) {
    switch (id) {
        case 1: return "Bebop";
        case 2: return "Cool";
        case 3: return "Modal";
        case 4: return "Hard Bop";
        case 5: return "Post-Bop";
        case 6: return "Swing";
        case 7: return "Fusion";
        case 8: return "Contemporary";
        case 9: return "Blues";
        default: return "Bebop";
    }
}

/**
 * List all available style IDs
 */
inline std::vector<int> listStyles() {
    return {1, 2, 3, 4, 5, 6, 7, 8, 9};
}

} // namespace StylePresets

} // namespace JazzArchitect
