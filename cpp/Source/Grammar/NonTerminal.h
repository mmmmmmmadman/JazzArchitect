#pragma once

#include <string>

namespace JazzArchitect {

/**
 * Non-terminal symbols in the jazz harmony grammar
 * Based on Rohrmeier (2020) recursive grammar framework
 */
enum class NonTerminal {
    S,      // Start symbol (entire piece)
    T,      // Tonic constituent
    D,      // Dominant constituent
    SD,     // Subdominant constituent
    PROL,   // Prolongation
    PREP,   // Preparation
    PHRASE  // Phrase boundary
};

/**
 * Types of grammar rules in jazz harmony
 */
enum class RuleType {
    PROLONGATION,   // X -> X X (extension)
    PREPARATION,    // X -> Prep X (approach)
    SUBSTITUTION,   // X -> Y (chord substitution)
    TERMINAL,       // X -> chord (leaf node)
    STRUCTURAL      // S -> T D T (top-level structure)
};

/**
 * Functional categories for chord classification
 */
enum class FunctionalCategory {
    TONIC,
    DOMINANT,
    SUBDOMINANT,
    APPLIED     // Applied/secondary dominant
};

// String conversion utilities
inline std::string nonTerminalToString(NonTerminal nt) {
    switch (nt) {
        case NonTerminal::S:      return "S";
        case NonTerminal::T:      return "T";
        case NonTerminal::D:      return "D";
        case NonTerminal::SD:     return "SD";
        case NonTerminal::PROL:   return "Prol";
        case NonTerminal::PREP:   return "Prep";
        case NonTerminal::PHRASE: return "Ph";
        default: return "?";
    }
}

inline std::string ruleTypeToString(RuleType type) {
    switch (type) {
        case RuleType::PROLONGATION: return "prolongation";
        case RuleType::PREPARATION:  return "preparation";
        case RuleType::SUBSTITUTION: return "substitution";
        case RuleType::TERMINAL:     return "terminal";
        case RuleType::STRUCTURAL:   return "structural";
        default: return "unknown";
    }
}

} // namespace JazzArchitect
