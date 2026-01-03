#pragma once

#include "NonTerminal.h"
#include "../Core/ChordQuality.h"
#include <vector>
#include <string>
#include <optional>
#include <variant>
#include <memory>
#include <unordered_map>

namespace JazzArchitect {

/**
 * Non-terminal symbol with optional key context
 */
struct NTSymbol {
    NonTerminal nt;
    std::optional<int> key;  // Key context (0-11), nullopt = inherit

    NTSymbol(NonTerminal n, std::optional<int> k = std::nullopt)
        : nt(n), key(k) {}

    std::string toString() const {
        if (key.has_value()) {
            return nonTerminalToString(nt) + "[" + std::to_string(key.value()) + "]";
        }
        return nonTerminalToString(nt);
    }
};

/**
 * Terminal symbol representing a chord function
 */
struct TerminalSymbol {
    std::string degree;      // Roman numeral (I, ii, V, bII, etc.)
    std::string quality;     // maj7, min7, 7, m7b5, etc.
    bool keyRelative = true; // Is degree relative to current key?

    TerminalSymbol(const std::string& deg, const std::string& qual, bool relative = true)
        : degree(deg), quality(qual), keyRelative(relative) {}

    std::string toString() const {
        return degree + quality;
    }
};

/**
 * A symbol can be either non-terminal or terminal
 */
using Symbol = std::variant<NTSymbol, TerminalSymbol>;

// Helper to check if symbol is terminal
inline bool isTerminal(const Symbol& sym) {
    return std::holds_alternative<TerminalSymbol>(sym);
}

// Helper to get string representation
inline std::string symbolToString(const Symbol& sym) {
    if (std::holds_alternative<NTSymbol>(sym)) {
        return std::get<NTSymbol>(sym).toString();
    }
    return std::get<TerminalSymbol>(sym).toString();
}

/**
 * A single PCFG production rule
 */
class GrammarRule {
public:
    GrammarRule(NonTerminal lhs,
                std::vector<Symbol> rhs,
                float prob = 1.0f,
                RuleType type = RuleType::STRUCTURAL,
                const std::string& name = "")
        : lhs_(lhs)
        , rhs_(std::move(rhs))
        , prob_(prob)
        , ruleType_(type)
        , name_(name)
    {
        // Clamp probability
        if (prob_ < 0.0f) prob_ = 0.0f;
        if (prob_ > 1.0f) prob_ = 1.0f;
    }

    // Getters
    NonTerminal getLhs() const { return lhs_; }
    const std::vector<Symbol>& getRhs() const { return rhs_; }
    float getProb() const { return prob_; }
    RuleType getRuleType() const { return ruleType_; }
    const std::string& getName() const { return name_; }

    // Setters
    void setProb(float p) { prob_ = std::max(0.0f, std::min(1.0f, p)); }

    /**
     * Apply rule, propagating key context
     */
    std::vector<Symbol> apply(int key = 0) const {
        std::vector<Symbol> result;
        for (const auto& sym : rhs_) {
            if (std::holds_alternative<NTSymbol>(sym)) {
                const auto& nts = std::get<NTSymbol>(sym);
                int newKey = nts.key.value_or(key);
                result.push_back(NTSymbol(nts.nt, newKey));
            } else {
                result.push_back(sym);
            }
        }
        return result;
    }

    std::string toString() const {
        std::string s = nonTerminalToString(lhs_) + " -> ";
        for (size_t i = 0; i < rhs_.size(); ++i) {
            if (i > 0) s += " ";
            s += symbolToString(rhs_[i]);
        }
        s += " [" + std::to_string(prob_) + "]";
        return s;
    }

private:
    NonTerminal lhs_;
    std::vector<Symbol> rhs_;
    float prob_;
    RuleType ruleType_;
    std::string name_;
};

/**
 * Degree to semitone mapping
 */
inline int degreeToSemitones(const std::string& degree) {
    static const std::unordered_map<std::string, int> mapping = {
        {"I", 0}, {"bII", 1}, {"II", 2}, {"bIII", 3}, {"III", 4},
        {"IV", 5}, {"#IV", 6}, {"V", 7}, {"bVI", 8}, {"VI", 9},
        {"bVII", 10}, {"VII", 11},
        {"i", 0}, {"ii", 2}, {"iii", 4}, {"iv", 5}, {"v", 7}, {"vi", 9}, {"vii", 11},
        {"V/V", 2}, {"V/ii", 9}, {"V/IV", 0}
    };

    auto it = mapping.find(degree);
    if (it != mapping.end()) {
        return it->second;
    }
    return 0;
}

/**
 * Quality string to ChordQuality enum mapping
 */
inline ChordQuality stringToChordQuality(const std::string& qual) {
    if (qual == "maj7") return ChordQuality::MAJ7;
    if (qual == "min7") return ChordQuality::MIN7;
    if (qual == "7") return ChordQuality::DOM7;
    if (qual == "m7b5") return ChordQuality::HDIM7;
    if (qual == "dim7") return ChordQuality::DIM7;
    if (qual == "maj6") return ChordQuality::MAJ6;
    if (qual == "min6") return ChordQuality::MIN6;
    return ChordQuality::MAJ7;  // default
}

} // namespace JazzArchitect
