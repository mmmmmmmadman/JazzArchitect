#include "Generator.h"
#include <algorithm>
#include <sstream>

namespace JazzArchitect {

HarmonyGenerator::HarmonyGenerator()
    : grammar_(createBaseGrammar())
    , config_{}
{
}

HarmonyGenerator::HarmonyGenerator(PCFG grammar, GeneratorConfig config)
    : grammar_(std::move(grammar))
    , config_(std::move(config))
{
    if (config_.seed.has_value()) {
        grammar_.setSeed(config_.seed.value());
    }
}

std::vector<ChordSymbol> HarmonyGenerator::generate() {
    auto [chords, tree] = generateWithTree();
    return chords;
}

std::pair<std::vector<ChordSymbol>, std::shared_ptr<DerivationNode>>
HarmonyGenerator::generateWithTree() {
    auto tree = derive(
        NTSymbol(grammar_.getStartSymbol(), config_.key),
        0,
        config_.key
    );

    auto terminals = tree->getTerminals();
    auto chords = terminalsToChords(terminals);

    return {chords, tree};
}

std::shared_ptr<DerivationNode> HarmonyGenerator::derive(
    const Symbol& symbol,
    int depth,
    int key)
{
    // Handle terminal symbols
    if (std::holds_alternative<TerminalSymbol>(symbol)) {
        return std::make_shared<DerivationNode>(symbol, key);
    }

    // Handle non-terminal symbols
    const auto& nts = std::get<NTSymbol>(symbol);
    NonTerminal nt = nts.nt;
    int currentKey = nts.key.value_or(key);

    auto node = std::make_shared<DerivationNode>(symbol, currentKey);

    // Check depth limit
    std::shared_ptr<GrammarRule> rule;

    if (depth >= config_.maxDepth) {
        // Force terminal rule
        auto rules = grammar_.getRules(nt);
        std::vector<std::shared_ptr<GrammarRule>> terminalRules;
        for (const auto& r : rules) {
            if (r->getRuleType() == RuleType::TERMINAL) {
                terminalRules.push_back(r);
            }
        }

        if (!terminalRules.empty()) {
            // Choose random terminal rule
            static std::mt19937 rng(std::random_device{}());
            std::uniform_int_distribution<size_t> dist(0, terminalRules.size() - 1);
            rule = terminalRules[dist(rng)];
        } else {
            // Fallback: create default terminal
            auto defaultTerm = getDefaultTerminal(nt);
            auto termNode = std::make_shared<DerivationNode>(defaultTerm, currentKey);
            node->children.push_back(termNode);
            return node;
        }
    } else {
        rule = grammar_.sampleRule(nt);
    }

    if (!rule) {
        // No rules available, use default terminal
        auto defaultTerm = getDefaultTerminal(nt);
        auto termNode = std::make_shared<DerivationNode>(defaultTerm, currentKey);
        node->children.push_back(termNode);
        return node;
    }

    node->ruleUsed = rule;

    // Apply rule and recursively derive
    for (const auto& rhsSym : rule->getRhs()) {
        int childKey = currentKey;

        // Handle key context for NTSymbol
        if (std::holds_alternative<NTSymbol>(rhsSym)) {
            const auto& childNts = std::get<NTSymbol>(rhsSym);
            if (childNts.key.has_value()) {
                childKey = childNts.key.value();
            }
        }

        // Handle key modulation for secondary dominants
        if (std::holds_alternative<TerminalSymbol>(rhsSym)) {
            const auto& term = std::get<TerminalSymbol>(rhsSym);
            if (term.degree.find("V/") != std::string::npos) {
                // Secondary dominant: adjust key
                size_t slashPos = term.degree.find('/');
                if (slashPos != std::string::npos) {
                    std::string target = term.degree.substr(slashPos + 1);
                    childKey = (currentKey + degreeToSemitones(target)) % 12;
                }
            }
        }

        Symbol childSym = rhsSym;
        if (std::holds_alternative<NTSymbol>(rhsSym)) {
            const auto& childNts = std::get<NTSymbol>(rhsSym);
            childSym = NTSymbol(childNts.nt, childKey);
        }

        auto childNode = derive(childSym, depth + 1, childKey);
        node->children.push_back(childNode);
    }

    return node;
}

TerminalSymbol HarmonyGenerator::getDefaultTerminal(NonTerminal nt) const {
    switch (nt) {
        case NonTerminal::T:
            return TerminalSymbol("I", "maj7");
        case NonTerminal::D:
            return TerminalSymbol("V", "7");
        case NonTerminal::SD:
            return TerminalSymbol("IV", "maj7");
        case NonTerminal::PREP:
            return TerminalSymbol("ii", "min7");
        case NonTerminal::PROL:
            return TerminalSymbol("vi", "min7");
        case NonTerminal::S:
        case NonTerminal::PHRASE:
        default:
            return TerminalSymbol("I", "maj7");
    }
}

std::vector<ChordSymbol> HarmonyGenerator::terminalsToChords(
    const std::vector<std::pair<TerminalSymbol, int>>& terminals) const
{
    std::vector<ChordSymbol> chords;
    chords.reserve(terminals.size());

    for (const auto& [term, key] : terminals) {
        chords.push_back(terminalToChord(term, key));
    }

    return chords;
}

ChordSymbol HarmonyGenerator::terminalToChord(
    const TerminalSymbol& terminal,
    int key) const
{
    std::string degree = terminal.degree;

    // Handle secondary dominants
    if (degree.find('/') != std::string::npos) {
        degree = "V";  // Simplify to V for now
    }

    // Get interval from degree
    int interval = degreeToSemitones(degree);

    // Calculate root
    int rootPc = (key + interval) % 12;
    if (rootPc < 0) rootPc += 12;

    PitchClass root(rootPc);
    ChordQuality quality = stringToChordQuality(terminal.quality);

    return ChordSymbol(root, quality);
}

void HarmonyGenerator::setKey(int key) {
    config_.key = key % 12;
    if (config_.key < 0) config_.key += 12;
}

void HarmonyGenerator::setSeed(unsigned int seed) {
    config_.seed = seed;
    grammar_.setSeed(seed);
}

void HarmonyGenerator::setMaxDepth(int depth) {
    config_.maxDepth = std::max(1, depth);
}

// Convenience function
std::vector<ChordSymbol> generateProgression(
    int length,
    int keyPitchClass,
    std::optional<unsigned int> seed)
{
    GeneratorConfig config;
    config.maxDepth = std::min(6, length / 2 + 2);
    config.minChords = length;
    config.maxChords = length * 2;
    config.key = keyPitchClass % 12;
    config.seed = seed;

    HarmonyGenerator generator(createBaseGrammar(), config);
    auto chords = generator.generate();

    // Trim or pad to desired length
    if (static_cast<int>(chords.size()) > length) {
        chords.resize(length);
    } else {
        // Pad with tonic
        ChordSymbol tonic(PitchClass(keyPitchClass % 12), ChordQuality::MAJ7);
        while (static_cast<int>(chords.size()) < length) {
            chords.push_back(tonic);
        }
    }

    return chords;
}

std::string formatProgression(
    const std::vector<ChordSymbol>& chords,
    int barsPerLine)
{
    std::ostringstream oss;
    std::vector<std::string> currentLine;

    for (size_t i = 0; i < chords.size(); ++i) {
        currentLine.push_back(chords[i].toString());

        if (static_cast<int>(currentLine.size()) >= barsPerLine) {
            for (size_t j = 0; j < currentLine.size(); ++j) {
                if (j > 0) oss << " | ";
                oss << currentLine[j];
            }
            oss << "\n";
            currentLine.clear();
        }
    }

    // Output remaining
    if (!currentLine.empty()) {
        for (size_t j = 0; j < currentLine.size(); ++j) {
            if (j > 0) oss << " | ";
            oss << currentLine[j];
        }
        oss << "\n";
    }

    return oss.str();
}

} // namespace JazzArchitect
