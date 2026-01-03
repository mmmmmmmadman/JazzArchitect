#pragma once

#include <string>
#include <vector>

namespace JazzArchitect {

/**
 * ChordQuality - 和弦品質枚舉
 */
enum class ChordQuality {
    MAJ7,       // Major 7th (Imaj7)
    MIN7,       // Minor 7th (ii7, iii7, vi7)
    DOM7,       // Dominant 7th (V7)
    HDIM7,      // Half-diminished (m7b5)
    DIM7,       // Fully diminished
    AUG,        // Augmented
    MIN_MAJ7,   // Minor-major 7th
    MAJ6,       // Major 6th
    MIN6,       // Minor 6th
    SUS4,       // Suspended 4th
    SUS2        // Suspended 2nd
};

/**
 * 取得品質的符號字串
 */
inline std::string qualityToString(ChordQuality q) {
    switch (q) {
        case ChordQuality::MAJ7: return "maj7";
        case ChordQuality::MIN7: return "m7";
        case ChordQuality::DOM7: return "7";
        case ChordQuality::HDIM7: return "m7b5";
        case ChordQuality::DIM7: return "dim7";
        case ChordQuality::AUG: return "aug";
        case ChordQuality::MIN_MAJ7: return "mMaj7";
        case ChordQuality::MAJ6: return "6";
        case ChordQuality::MIN6: return "m6";
        case ChordQuality::SUS4: return "sus4";
        case ChordQuality::SUS2: return "sus2";
        default: return "";
    }
}

/**
 * 取得品質的音程組成 (半音數)
 */
inline std::vector<int> qualityIntervals(ChordQuality q) {
    switch (q) {
        case ChordQuality::MAJ7: return {0, 4, 7, 11};
        case ChordQuality::MIN7: return {0, 3, 7, 10};
        case ChordQuality::DOM7: return {0, 4, 7, 10};
        case ChordQuality::HDIM7: return {0, 3, 6, 10};
        case ChordQuality::DIM7: return {0, 3, 6, 9};
        case ChordQuality::AUG: return {0, 4, 8};
        case ChordQuality::MIN_MAJ7: return {0, 3, 7, 11};
        case ChordQuality::MAJ6: return {0, 4, 7, 9};
        case ChordQuality::MIN6: return {0, 3, 7, 9};
        case ChordQuality::SUS4: return {0, 5, 7, 10};
        case ChordQuality::SUS2: return {0, 2, 7, 10};
        default: return {0, 4, 7};
    }
}

/**
 * 判斷是否為 minor 類品質
 */
inline bool isMinorQuality(ChordQuality q) {
    return q == ChordQuality::MIN7 ||
           q == ChordQuality::HDIM7 ||
           q == ChordQuality::MIN6 ||
           q == ChordQuality::MIN_MAJ7;
}

} // namespace JazzArchitect
