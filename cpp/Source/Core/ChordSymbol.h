#pragma once

#include "PitchClass.h"
#include "ChordQuality.h"
#include <string>
#include <vector>
#include <map>
#include <optional>

namespace JazzArchitect {

/**
 * ChordSymbol - 和弦符號表示
 *
 * 包含根音、品質、延伸音、變化音、斜線和弦低音
 */
class ChordSymbol {
public:
    ChordSymbol() : root_(0), quality_(ChordQuality::MAJ7) {}

    ChordSymbol(PitchClass root, ChordQuality quality)
        : root_(root), quality_(quality) {}

    // 從字串解析
    static ChordSymbol fromString(const std::string& symbol);

    // Getters
    PitchClass root() const { return root_; }
    ChordQuality quality() const { return quality_; }
    const std::vector<int>& extensions() const { return extensions_; }
    const std::map<int, int>& alterations() const { return alterations_; }
    std::optional<PitchClass> bass() const { return bass_; }

    // Setters
    void setRoot(PitchClass root) { root_ = root; }
    void setQuality(ChordQuality q) { quality_ = q; }
    void addExtension(int ext) { extensions_.push_back(ext); }
    void setAlteration(int degree, int alteration) { alterations_[degree] = alteration; }
    void setBass(PitchClass bass) { bass_ = bass; }

    // Guide tones (3rd and 7th)
    PitchClass third() const;
    PitchClass seventh() const;
    PitchClass fifth() const;

    // 取得所有組成音
    std::vector<PitchClass> getPitchClasses() const;

    // 取得 MIDI 音高 (指定八度)
    std::vector<int> getMIDINotes(int baseOctave = 3) const;

    // 移調
    ChordSymbol transpose(int semitones) const;

    // 三全音替代
    ChordSymbol tritoneSubstitute() const;

    // 羅馬數字表示
    std::string asRomanNumeral(PitchClass key) const;

    // 轉換為字串
    std::string toString() const;

    bool operator==(const ChordSymbol& other) const {
        return root_ == other.root_ && quality_ == other.quality_;
    }

private:
    PitchClass root_;
    ChordQuality quality_;
    std::vector<int> extensions_;           // 9, 11, 13
    std::map<int, int> alterations_;        // degree -> alteration (-1=b, +1=#)
    std::optional<PitchClass> bass_;        // Slash chord bass
};

// 便捷工廠函數
inline ChordSymbol maj7(const std::string& root) {
    return ChordSymbol(PitchClass::fromName(root), ChordQuality::MAJ7);
}

inline ChordSymbol min7(const std::string& root) {
    return ChordSymbol(PitchClass::fromName(root), ChordQuality::MIN7);
}

inline ChordSymbol dom7(const std::string& root) {
    return ChordSymbol(PitchClass::fromName(root), ChordQuality::DOM7);
}

inline ChordSymbol hdim7(const std::string& root) {
    return ChordSymbol(PitchClass::fromName(root), ChordQuality::HDIM7);
}

} // namespace JazzArchitect
