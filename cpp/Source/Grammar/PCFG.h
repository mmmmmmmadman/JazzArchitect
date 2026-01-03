#pragma once

#include "GrammarRule.h"
#include <unordered_map>
#include <random>
#include <memory>

namespace JazzArchitect {

/**
 * Probabilistic Context-Free Grammar for jazz harmony
 * Based on Rohrmeier (2020) recursive grammar framework
 */
class PCFG {
public:
    PCFG(NonTerminal startSymbol = NonTerminal::S);

    /**
     * Add a rule to the grammar
     */
    void addRule(std::shared_ptr<GrammarRule> rule);

    /**
     * Add a rule with parameters (convenience method)
     */
    void addRule(NonTerminal lhs,
                 std::vector<Symbol> rhs,
                 float prob = 1.0f,
                 RuleType type = RuleType::STRUCTURAL,
                 const std::string& name = "");

    /**
     * Get all rules for a non-terminal
     */
    std::vector<std::shared_ptr<GrammarRule>> getRules(NonTerminal nt) const;

    /**
     * Normalize probabilities for each non-terminal
     */
    void normalize();

    /**
     * Sample a rule according to probabilities
     */
    std::shared_ptr<GrammarRule> sampleRule(NonTerminal nt);

    /**
     * Get start symbol
     */
    NonTerminal getStartSymbol() const { return startSymbol_; }

    /**
     * Get string representation
     */
    std::string toString() const;

    /**
     * Get number of rules
     */
    size_t getRuleCount() const;

    /**
     * Set random seed
     */
    void setSeed(unsigned int seed);

private:
    NonTerminal startSymbol_;
    std::unordered_map<NonTerminal, std::vector<std::shared_ptr<GrammarRule>>> rules_;
    std::mt19937 rng_;
};

/**
 * Create the base jazz harmony grammar based on Rohrmeier (2020)
 */
PCFG createBaseGrammar();

} // namespace JazzArchitect
