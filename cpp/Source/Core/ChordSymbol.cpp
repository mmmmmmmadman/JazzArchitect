#include "ChordSymbol.h"
#include <regex>
#include <algorithm>

namespace JazzArchitect {

ChordSymbol ChordSymbol::fromString(const std::string& symbol) {
    if (symbol.empty()) {
        return ChordSymbol();
    }

    // 解析根音
    std::regex rootRegex("^([A-G][#b]?)");
    std::smatch rootMatch;
    if (!std::regex_search(symbol, rootMatch, rootRegex)) {
        return ChordSymbol();
    }

    std::string rootStr = rootMatch[1];
    PitchClass root = PitchClass::fromName(rootStr);
    std::string remainder = symbol.substr(rootStr.length());

    // 檢查斜線和弦
    std::optional<PitchClass> bass;
    size_t slashPos = remainder.find('/');
    if (slashPos != std::string::npos) {
        std::string bassStr = remainder.substr(slashPos + 1);
        bass = PitchClass::fromName(bassStr);
        remainder = remainder.substr(0, slashPos);
    }

    // 解析品質
    ChordQuality quality = ChordQuality::DOM7;

    struct QualityPattern {
        std::string pattern;
        ChordQuality quality;
    };

    std::vector<QualityPattern> patterns = {
        {"maj7", ChordQuality::MAJ7},
        {"Maj7", ChordQuality::MAJ7},
        {"M7", ChordQuality::MAJ7},
        {"mMaj7", ChordQuality::MIN_MAJ7},
        {"m7b5", ChordQuality::HDIM7},
        {"m7-5", ChordQuality::HDIM7},
        {"dim7", ChordQuality::DIM7},
        {"m7", ChordQuality::MIN7},
        {"min7", ChordQuality::MIN7},
        {"-7", ChordQuality::MIN7},
        {"m6", ChordQuality::MIN6},
        {"6", ChordQuality::MAJ6},
        {"aug", ChordQuality::AUG},
        {"+", ChordQuality::AUG},
        {"sus4", ChordQuality::SUS4},
        {"sus2", ChordQuality::SUS2},
        {"7", ChordQuality::DOM7},
    };

    for (const auto& p : patterns) {
        if (remainder.find(p.pattern) == 0) {
            quality = p.quality;
            remainder = remainder.substr(p.pattern.length());
            break;
        }
    }

    ChordSymbol chord(root, quality);
    if (bass.has_value()) {
        chord.setBass(bass.value());
    }

    // 解析延伸音和變化音
    std::regex extRegex("([b#]?)(9|11|13)");
    std::sregex_iterator it(remainder.begin(), remainder.end(), extRegex);
    std::sregex_iterator end;

    while (it != end) {
        std::smatch match = *it;
        std::string alt = match[1];
        int degree = std::stoi(match[2]);

        chord.addExtension(degree);
        if (!alt.empty()) {
            chord.setAlteration(degree, alt == "b" ? -1 : 1);
        }
        ++it;
    }

    // 檢查 b5 或 #5
    if (remainder.find("b5") != std::string::npos) {
        chord.setAlteration(5, -1);
    } else if (remainder.find("#5") != std::string::npos) {
        chord.setAlteration(5, 1);
    }

    return chord;
}

PitchClass ChordSymbol::third() const {
    auto intervals = qualityIntervals(quality_);
    int thirdInterval = intervals.size() > 1 ? intervals[1] : 4;
    return root_.transpose(thirdInterval);
}

PitchClass ChordSymbol::seventh() const {
    auto intervals = qualityIntervals(quality_);
    if (intervals.size() < 4) {
        return root_.transpose(10); // Default to minor 7th
    }
    return root_.transpose(intervals[3]);
}

PitchClass ChordSymbol::fifth() const {
    auto intervals = qualityIntervals(quality_);
    int fifthInterval = intervals.size() > 2 ? intervals[2] : 7;

    // Apply alterations
    auto it = alterations_.find(5);
    if (it != alterations_.end()) {
        fifthInterval += it->second;
    }

    return root_.transpose(fifthInterval);
}

std::vector<PitchClass> ChordSymbol::getPitchClasses() const {
    std::vector<PitchClass> result;
    auto intervals = qualityIntervals(quality_);

    for (int interval : intervals) {
        result.push_back(root_.transpose(interval));
    }

    // Add extensions
    for (int ext : extensions_) {
        int interval;
        auto it = alterations_.find(ext);
        int alt = (it != alterations_.end()) ? it->second : 0;

        switch (ext) {
            case 9: interval = 2 + alt; break;
            case 11: interval = 5 + alt; break;
            case 13: interval = 9 + alt; break;
            default: continue;
        }
        result.push_back(root_.transpose(interval));
    }

    return result;
}

std::vector<int> ChordSymbol::getMIDINotes(int baseOctave) const {
    std::vector<int> notes;
    int baseMidi = (baseOctave + 1) * 12 + root_.value();

    auto intervals = qualityIntervals(quality_);
    for (int interval : intervals) {
        notes.push_back(baseMidi + interval);
    }

    return notes;
}

ChordSymbol ChordSymbol::transpose(int semitones) const {
    ChordSymbol result = *this;
    result.root_ = root_.transpose(semitones);
    if (bass_.has_value()) {
        result.bass_ = bass_->transpose(semitones);
    }
    return result;
}

ChordSymbol ChordSymbol::tritoneSubstitute() const {
    ChordSymbol result = *this;
    result.root_ = root_.transpose(6); // Tritone = 6 semitones
    return result;
}

std::string ChordSymbol::asRomanNumeral(PitchClass key) const {
    int interval = key.intervalTo(root_);

    static const std::string numerals[] = {
        "I", "bII", "II", "bIII", "III", "IV",
        "#IV", "V", "bVI", "VI", "bVII", "VII"
    };

    std::string base = numerals[interval];

    // Lowercase for minor qualities
    if (isMinorQuality(quality_)) {
        std::transform(base.begin(), base.end(), base.begin(), ::tolower);
    }

    return base + qualityToString(quality_);
}

std::string ChordSymbol::toString() const {
    std::string result = root_.name() + qualityToString(quality_);

    // Add alterations
    for (const auto& [degree, alt] : alterations_) {
        if (alt < 0) result += "b";
        else if (alt > 0) result += "#";
        result += std::to_string(degree);
    }

    // Add extensions
    for (int ext : extensions_) {
        if (alterations_.find(ext) == alterations_.end()) {
            result += "(" + std::to_string(ext) + ")";
        }
    }

    // Slash chord
    if (bass_.has_value() && bass_.value() != root_) {
        result += "/" + bass_->name();
    }

    return result;
}

} // namespace JazzArchitect
