#include "PCFG.h"
#include <algorithm>
#include <numeric>
#include <sstream>

namespace JazzArchitect {

PCFG::PCFG(NonTerminal startSymbol)
    : startSymbol_(startSymbol)
    , rng_(std::random_device{}())
{
}

void PCFG::addRule(std::shared_ptr<GrammarRule> rule) {
    NonTerminal lhs = rule->getLhs();
    rules_[lhs].push_back(std::move(rule));
}

void PCFG::addRule(NonTerminal lhs,
                   std::vector<Symbol> rhs,
                   float prob,
                   RuleType type,
                   const std::string& name) {
    auto rule = std::make_shared<GrammarRule>(lhs, std::move(rhs), prob, type, name);
    addRule(std::move(rule));
}

std::vector<std::shared_ptr<GrammarRule>> PCFG::getRules(NonTerminal nt) const {
    auto it = rules_.find(nt);
    if (it != rules_.end()) {
        return it->second;
    }
    return {};
}

void PCFG::normalize() {
    for (auto& [nt, ruleList] : rules_) {
        float total = 0.0f;
        for (const auto& rule : ruleList) {
            total += rule->getProb();
        }
        if (total > 0.0f) {
            for (auto& rule : ruleList) {
                rule->setProb(rule->getProb() / total);
            }
        }
    }
}

std::shared_ptr<GrammarRule> PCFG::sampleRule(NonTerminal nt) {
    auto ruleList = getRules(nt);
    if (ruleList.empty()) {
        return nullptr;
    }

    // Calculate total probability
    float total = 0.0f;
    for (const auto& rule : ruleList) {
        total += rule->getProb();
    }

    if (total == 0.0f) {
        // Uniform random choice
        std::uniform_int_distribution<size_t> dist(0, ruleList.size() - 1);
        return ruleList[dist(rng_)];
    }

    // Weighted random choice
    std::uniform_real_distribution<float> dist(0.0f, total);
    float r = dist(rng_);
    float cumsum = 0.0f;

    for (const auto& rule : ruleList) {
        cumsum += rule->getProb();
        if (r <= cumsum) {
            return rule;
        }
    }

    return ruleList.back();
}

std::string PCFG::toString() const {
    std::ostringstream oss;
    for (const auto& [nt, ruleList] : rules_) {
        for (const auto& rule : ruleList) {
            oss << rule->toString() << "\n";
        }
    }
    return oss.str();
}

size_t PCFG::getRuleCount() const {
    size_t count = 0;
    for (const auto& [nt, ruleList] : rules_) {
        count += ruleList.size();
    }
    return count;
}

void PCFG::setSeed(unsigned int seed) {
    rng_.seed(seed);
}

// Create the base jazz harmony grammar based on Rohrmeier (2020)
PCFG createBaseGrammar() {
    PCFG grammar(NonTerminal::S);

    // Structural rules (S)
    grammar.addRule(NonTerminal::S,
        {NTSymbol(NonTerminal::T)},
        0.3f, RuleType::STRUCTURAL, "single_phrase");

    grammar.addRule(NonTerminal::S,
        {NTSymbol(NonTerminal::T), NTSymbol(NonTerminal::D), NTSymbol(NonTerminal::T)},
        0.5f, RuleType::STRUCTURAL, "tdt_form");

    grammar.addRule(NonTerminal::S,
        {NTSymbol(NonTerminal::T), NTSymbol(NonTerminal::T)},
        0.2f, RuleType::STRUCTURAL, "tt_form");

    // Tonic rules (T)
    grammar.addRule(NonTerminal::T,
        {TerminalSymbol("I", "maj7")},
        0.3f, RuleType::TERMINAL, "t_terminal");

    grammar.addRule(NonTerminal::T,
        {NTSymbol(NonTerminal::D), NTSymbol(NonTerminal::T)},
        0.35f, RuleType::PREPARATION, "authentic_cadence");

    grammar.addRule(NonTerminal::T,
        {NTSymbol(NonTerminal::SD), NTSymbol(NonTerminal::T)},
        0.15f, RuleType::PREPARATION, "plagal_cadence");

    grammar.addRule(NonTerminal::T,
        {NTSymbol(NonTerminal::T), NTSymbol(NonTerminal::PROL)},
        0.1f, RuleType::PROLONGATION, "t_right_prolong");

    grammar.addRule(NonTerminal::T,
        {NTSymbol(NonTerminal::PROL), NTSymbol(NonTerminal::T)},
        0.1f, RuleType::PROLONGATION, "t_left_prolong");

    // Dominant rules (D)
    grammar.addRule(NonTerminal::D,
        {TerminalSymbol("V", "7")},
        0.4f, RuleType::TERMINAL, "d_terminal");

    grammar.addRule(NonTerminal::D,
        {NTSymbol(NonTerminal::PREP), NTSymbol(NonTerminal::D)},
        0.4f, RuleType::PREPARATION, "ii_v");

    grammar.addRule(NonTerminal::D,
        {NTSymbol(NonTerminal::D), NTSymbol(NonTerminal::PROL)},
        0.1f, RuleType::PROLONGATION, "d_prolong");

    grammar.addRule(NonTerminal::D,
        {TerminalSymbol("bII", "7")},
        0.1f, RuleType::SUBSTITUTION, "tritone_sub");

    // Subdominant rules (SD)
    grammar.addRule(NonTerminal::SD,
        {TerminalSymbol("IV", "maj7")},
        0.5f, RuleType::TERMINAL, "sd_iv");

    grammar.addRule(NonTerminal::SD,
        {TerminalSymbol("ii", "min7")},
        0.3f, RuleType::TERMINAL, "sd_ii");

    grammar.addRule(NonTerminal::SD,
        {TerminalSymbol("iv", "min7")},
        0.2f, RuleType::TERMINAL, "sd_borrowed_iv");

    // Preparation rules (Prep)
    grammar.addRule(NonTerminal::PREP,
        {TerminalSymbol("ii", "min7")},
        0.5f, RuleType::TERMINAL, "prep_ii");

    grammar.addRule(NonTerminal::PREP,
        {TerminalSymbol("IV", "maj7")},
        0.2f, RuleType::TERMINAL, "prep_iv");

    grammar.addRule(NonTerminal::PREP,
        {TerminalSymbol("V/V", "7")},
        0.15f, RuleType::TERMINAL, "prep_secondary_dom");

    grammar.addRule(NonTerminal::PREP,
        {NTSymbol(NonTerminal::PREP), NTSymbol(NonTerminal::PREP)},
        0.15f, RuleType::PROLONGATION, "prep_chain");

    // Prolongation rules (Prol)
    grammar.addRule(NonTerminal::PROL,
        {TerminalSymbol("iii", "min7")},
        0.3f, RuleType::TERMINAL, "prol_iii");

    grammar.addRule(NonTerminal::PROL,
        {TerminalSymbol("vi", "min7")},
        0.4f, RuleType::TERMINAL, "prol_vi");

    grammar.addRule(NonTerminal::PROL,
        {TerminalSymbol("I", "maj7")},
        0.3f, RuleType::TERMINAL, "prol_i");

    grammar.normalize();
    return grammar;
}

} // namespace JazzArchitect
