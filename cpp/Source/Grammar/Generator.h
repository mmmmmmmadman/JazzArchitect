#pragma once

#include "PCFG.h"
#include "../Core/ChordSymbol.h"
#include <memory>
#include <optional>

namespace JazzArchitect {

/**
 * Configuration for the harmony generator
 */
struct GeneratorConfig {
    int maxDepth = 6;
    int minChords = 4;
    int maxChords = 32;
    int key = 0;  // Key in pitch class (C=0)
    std::optional<unsigned int> seed = std::nullopt;
};

/**
 * Node in derivation tree
 */
struct DerivationNode {
    Symbol symbol;
    std::vector<std::shared_ptr<DerivationNode>> children;
    std::shared_ptr<GrammarRule> ruleUsed;
    int key = 0;

    DerivationNode(Symbol sym, int k = 0)
        : symbol(std::move(sym)), key(k) {}

    bool isTerminal() const {
        return std::holds_alternative<TerminalSymbol>(symbol);
    }

    /**
     * Get all terminal symbols with their key contexts
     */
    std::vector<std::pair<TerminalSymbol, int>> getTerminals() const {
        if (isTerminal()) {
            return {{std::get<TerminalSymbol>(symbol), key}};
        }
        std::vector<std::pair<TerminalSymbol, int>> result;
        for (const auto& child : children) {
            auto childTerminals = child->getTerminals();
            result.insert(result.end(), childTerminals.begin(), childTerminals.end());
        }
        return result;
    }

    /**
     * Get tree depth
     */
    int depth() const {
        if (children.empty()) {
            return 0;
        }
        int maxChildDepth = 0;
        for (const auto& child : children) {
            maxChildDepth = std::max(maxChildDepth, child->depth());
        }
        return 1 + maxChildDepth;
    }
};

/**
 * Top-down derivation generator for jazz progressions
 * Based on Rohrmeier (2020) recursive grammar framework
 */
class HarmonyGenerator {
public:
    HarmonyGenerator();
    explicit HarmonyGenerator(PCFG grammar, GeneratorConfig config = {});

    /**
     * Generate a chord progression
     */
    std::vector<ChordSymbol> generate();

    /**
     * Generate progression and return derivation tree
     */
    std::pair<std::vector<ChordSymbol>, std::shared_ptr<DerivationNode>> generateWithTree();

    /**
     * Set the key for generation
     */
    void setKey(int key);

    /**
     * Set random seed for reproducibility
     */
    void setSeed(unsigned int seed);

    /**
     * Set max derivation depth
     */
    void setMaxDepth(int depth);

    /**
     * Get the PCFG grammar
     */
    PCFG& getGrammar() { return grammar_; }
    const PCFG& getGrammar() const { return grammar_; }

private:
    PCFG grammar_;
    GeneratorConfig config_;

    /**
     * Recursively derive from a symbol
     */
    std::shared_ptr<DerivationNode> derive(const Symbol& symbol, int depth, int key);

    /**
     * Get default terminal for a non-terminal
     */
    TerminalSymbol getDefaultTerminal(NonTerminal nt) const;

    /**
     * Convert terminal symbols to ChordSymbol objects
     */
    std::vector<ChordSymbol> terminalsToChords(
        const std::vector<std::pair<TerminalSymbol, int>>& terminals) const;

    /**
     * Convert a terminal symbol to a ChordSymbol
     */
    ChordSymbol terminalToChord(const TerminalSymbol& terminal, int key) const;
};

/**
 * Convenience function to generate a progression
 */
std::vector<ChordSymbol> generateProgression(
    int length = 8,
    int keyPitchClass = 0,
    std::optional<unsigned int> seed = std::nullopt);

/**
 * Format chord progression for display
 */
std::string formatProgression(
    const std::vector<ChordSymbol>& chords,
    int barsPerLine = 4);

} // namespace JazzArchitect
