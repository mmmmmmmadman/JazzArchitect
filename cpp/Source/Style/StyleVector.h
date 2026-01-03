#pragma once

#include <algorithm>
#include <string>

namespace JazzArchitect {

/**
 * Style parameters that control harmony generation.
 * Each parameter affects the probability of certain harmonic choices.
 */
struct StyleVector {
    // Substitution probabilities
    float tritoneSubProb = 0.3f;      // Probability of tritone substitution
    float backdoorProb = 0.15f;        // Probability of backdoor dominant
    float coltraneProb = 0.1f;         // Probability of Coltrane changes

    // Preparation preferences
    float iiVPreference = 0.8f;        // Preference for ii-V over direct V (0-1)
    float secondaryDomProb = 0.3f;     // Probability of secondary dominants

    // Modal/borrowed chord usage
    float modalInterchange = 0.2f;     // Frequency of borrowed chords
    float minorIvProb = 0.15f;         // Probability of minor iv in major

    // Approach patterns
    float chromaticApproach = 0.2f;    // Chromatic neighbor chord frequency
    float diminishedApproach = 0.1f;   // Diminished passing chord frequency

    // Structure parameters
    int dominantChainDepth = 3;        // Max depth of dominant chains
    int prolongationDepth = 2;         // Max prolongation recursion

    // Rhythmic density (harmonic rhythm)
    float rhythmDensity = 0.5f;        // Chord change frequency (0=sparse, 1=dense)
    float turnaroundProb = 0.4f;       // Probability of turnaround at phrase end

    // Tension parameters
    float extensionLevel = 0.5f;       // Average extension level (9ths, 11ths, 13ths)
    float alterationProb = 0.2f;       // Probability of altered dominants

    /**
     * Validate all parameters are in valid ranges
     */
    void validate() {
        tritoneSubProb = std::clamp(tritoneSubProb, 0.0f, 1.0f);
        backdoorProb = std::clamp(backdoorProb, 0.0f, 1.0f);
        coltraneProb = std::clamp(coltraneProb, 0.0f, 1.0f);
        iiVPreference = std::clamp(iiVPreference, 0.0f, 1.0f);
        secondaryDomProb = std::clamp(secondaryDomProb, 0.0f, 1.0f);
        modalInterchange = std::clamp(modalInterchange, 0.0f, 1.0f);
        minorIvProb = std::clamp(minorIvProb, 0.0f, 1.0f);
        chromaticApproach = std::clamp(chromaticApproach, 0.0f, 1.0f);
        diminishedApproach = std::clamp(diminishedApproach, 0.0f, 1.0f);
        rhythmDensity = std::clamp(rhythmDensity, 0.0f, 1.0f);
        turnaroundProb = std::clamp(turnaroundProb, 0.0f, 1.0f);
        extensionLevel = std::clamp(extensionLevel, 0.0f, 1.0f);
        alterationProb = std::clamp(alterationProb, 0.0f, 1.0f);
        dominantChainDepth = std::max(1, dominantChainDepth);
        prolongationDepth = std::max(0, prolongationDepth);
    }

    /**
     * Blend with another style vector
     * @param other Another StyleVector to blend with
     * @param weight Weight for the other vector (0 = self, 1 = other)
     * @return New StyleVector with blended values
     */
    StyleVector blend(const StyleVector& other, float weight = 0.5f) const {
        float w1 = 1.0f - weight;
        float w2 = weight;

        StyleVector result;
        result.tritoneSubProb = w1 * tritoneSubProb + w2 * other.tritoneSubProb;
        result.backdoorProb = w1 * backdoorProb + w2 * other.backdoorProb;
        result.coltraneProb = w1 * coltraneProb + w2 * other.coltraneProb;
        result.iiVPreference = w1 * iiVPreference + w2 * other.iiVPreference;
        result.secondaryDomProb = w1 * secondaryDomProb + w2 * other.secondaryDomProb;
        result.modalInterchange = w1 * modalInterchange + w2 * other.modalInterchange;
        result.minorIvProb = w1 * minorIvProb + w2 * other.minorIvProb;
        result.chromaticApproach = w1 * chromaticApproach + w2 * other.chromaticApproach;
        result.diminishedApproach = w1 * diminishedApproach + w2 * other.diminishedApproach;
        result.dominantChainDepth = static_cast<int>(w1 * dominantChainDepth + w2 * other.dominantChainDepth);
        result.prolongationDepth = static_cast<int>(w1 * prolongationDepth + w2 * other.prolongationDepth);
        result.rhythmDensity = w1 * rhythmDensity + w2 * other.rhythmDensity;
        result.turnaroundProb = w1 * turnaroundProb + w2 * other.turnaroundProb;
        result.extensionLevel = w1 * extensionLevel + w2 * other.extensionLevel;
        result.alterationProb = w1 * alterationProb + w2 * other.alterationProb;

        result.validate();
        return result;
    }

    /**
     * Generate human-readable description of the style
     */
    std::string describe() const {
        std::string parts;

        if (tritoneSubProb > 0.4f) {
            parts += "heavy tritone substitution, ";
        } else if (tritoneSubProb > 0.2f) {
            parts += "moderate tritone subs, ";
        }

        if (iiVPreference > 0.7f) {
            parts += "strong ii-V preference, ";
        }

        if (coltraneProb > 0.15f) {
            parts += "Coltrane-influenced, ";
        }

        if (modalInterchange > 0.4f) {
            parts += "modal borrowing, ";
        }

        if (rhythmDensity > 0.7f) {
            parts += "dense harmonic rhythm, ";
        } else if (rhythmDensity < 0.3f) {
            parts += "sparse changes, ";
        }

        if (extensionLevel > 0.6f) {
            parts += "extended harmonies, ";
        }

        if (alterationProb > 0.3f) {
            parts += "altered dominants, ";
        }

        if (parts.empty()) {
            return "standard jazz harmony";
        }

        // Remove trailing ", "
        if (parts.size() >= 2) {
            parts = parts.substr(0, parts.size() - 2);
        }

        return parts;
    }
};

} // namespace JazzArchitect
