#include "StyleEngine.h"
#include "../Substitution/SubstitutionEngine.h"
#include "../VoiceLeading/VoiceLeadingOptimizer.h"
#include <random>
#include <algorithm>

namespace JazzArchitect {

PCFG styleToPCFG(const StyleVector& style) {
    PCFG grammar = createBaseGrammar();

    // Adjust Dominant rules based on style
    auto dRules = grammar.getRules(NonTerminal::D);
    for (auto& rule : dRules) {
        if (rule->getName() == "ii_v") {
            rule->setProb(0.4f * style.iiVPreference);
        } else if (rule->getName() == "d_terminal") {
            rule->setProb(0.4f * (1.0f - style.iiVPreference * 0.3f));
        } else if (rule->getName() == "tritone_sub") {
            rule->setProb(0.15f * style.tritoneSubProb);
        }
    }

    // Adjust Tonic rules based on style
    auto tRules = grammar.getRules(NonTerminal::T);
    for (auto& rule : tRules) {
        std::string name = rule->getName();
        if (name.find("prolong") != std::string::npos) {
            rule->setProb(0.1f * (1.0f + style.prolongationDepth * 0.2f));
        } else if (name == "authentic_cadence") {
            rule->setProb(0.35f * style.iiVPreference);
        } else if (name == "plagal_cadence") {
            // Plagal cadence more common in blues/modal styles
            rule->setProb(0.15f * (1.0f + style.modalInterchange * 0.5f));
        }
    }

    // Adjust Preparation rules
    auto prepRules = grammar.getRules(NonTerminal::PREP);
    for (auto& rule : prepRules) {
        if (rule->getName() == "prep_chain") {
            rule->setProb(0.15f * std::min(1.0f, static_cast<float>(style.dominantChainDepth) / 4.0f));
        } else if (rule->getName() == "prep_secondary_dom") {
            rule->setProb(0.15f * style.secondaryDomProb);
        }
    }

    // Adjust Subdominant rules (modal interchange affects borrowed chords)
    auto sdRules = grammar.getRules(NonTerminal::SD);
    for (auto& rule : sdRules) {
        if (rule->getName() == "sd_borrowed_iv") {
            rule->setProb(0.2f * (1.0f + style.modalInterchange));
        }
    }

    grammar.normalize();
    return grammar;
}

StyleEngine::StyleEngine()
    : style_(StylePresets::BEBOP())
{
    updateGrammar();
}

StyleEngine::StyleEngine(const StyleVector& style)
    : style_(style)
{
    updateGrammar();
}

void StyleEngine::setStyle(const StyleVector& style) {
    style_ = style;
    style_.validate();
    updateGrammar();
}

void StyleEngine::setStyleById(int id) {
    setStyle(StylePresets::getStyleById(id));
}

void StyleEngine::updateGrammar() {
    grammar_ = styleToPCFG(style_);
    generator_ = std::make_unique<HarmonyGenerator>(grammar_);
}

std::vector<ChordSymbol> StyleEngine::generate(int length, int key) {
    generator_->setKey(key);
    generator_->setMaxDepth(std::min(6, length / 2 + 2));

    auto chords = generator_->generate();
    chords = adjustLength(chords, length, key);

    // Apply post-generation substitutions based on style
    chords = SubstitutionEngine::apply(chords, style_);

    // Optimize for voice leading if style calls for it
    // Higher extension level = more concern for smooth voice leading
    if (style_.extensionLevel > 0.5f) {
        chords = VoiceLeadingOptimizer::optimize(chords, 50);
    }

    // Ensure length is still correct after substitutions
    if (static_cast<int>(chords.size()) > length) {
        chords.resize(length);
    }

    return chords;
}

std::vector<ChordSymbol> StyleEngine::adjustLength(
    std::vector<ChordSymbol>& chords,
    int target,
    int key)
{
    if (static_cast<int>(chords.size()) >= target) {
        chords.resize(target);
        return chords;
    }

    // Random generator for turnarounds
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Pad with appropriate chords
    ChordSymbol tonic(PitchClass(key), ChordQuality::MAJ7);

    while (static_cast<int>(chords.size()) < target) {
        // Add turnaround or repeat tonic based on style
        if (dist(rng) < style_.turnaroundProb && static_cast<int>(chords.size()) < target - 1) {
            // Add simple ii-V turnaround
            PitchClass iiRoot = PitchClass(key).transpose(2);
            PitchClass vRoot = PitchClass(key).transpose(7);
            chords.push_back(ChordSymbol(iiRoot, ChordQuality::MIN7));
            if (static_cast<int>(chords.size()) < target) {
                chords.push_back(ChordSymbol(vRoot, ChordQuality::DOM7));
            }
        } else {
            chords.push_back(tonic);
        }
    }

    // Ensure exact length
    if (static_cast<int>(chords.size()) > target) {
        chords.resize(target);
    }

    return chords;
}

void StyleEngine::setTritoneSubProb(float value) {
    style_.tritoneSubProb = std::clamp(value, 0.0f, 1.0f);
    updateGrammar();
}

void StyleEngine::setIiVPreference(float value) {
    style_.iiVPreference = std::clamp(value, 0.0f, 1.0f);
    updateGrammar();
}

void StyleEngine::setModalInterchange(float value) {
    style_.modalInterchange = std::clamp(value, 0.0f, 1.0f);
    updateGrammar();
}

void StyleEngine::setExtensionLevel(float value) {
    style_.extensionLevel = std::clamp(value, 0.0f, 1.0f);
    // Extension level doesn't affect grammar, but will affect chord voicing later
}

} // namespace JazzArchitect
