#pragma once

#include <string>
#include <array>
#include <cmath>

namespace JazzArchitect {

/**
 * PitchClass - 音高類 (0-11)
 * C=0, C#/Db=1, D=2, ... B=11
 */
class PitchClass {
public:
    PitchClass() : value_(0) {}
    explicit PitchClass(int value) : value_(value % 12) {
        if (value_ < 0) value_ += 12;
    }

    int value() const { return value_; }

    // 從音名建立
    static PitchClass fromName(const std::string& name) {
        static const std::array<std::pair<std::string, int>, 21> nameMap = {{
            {"C", 0}, {"B#", 0},
            {"C#", 1}, {"Db", 1},
            {"D", 2},
            {"D#", 3}, {"Eb", 3},
            {"E", 4}, {"Fb", 4},
            {"F", 5}, {"E#", 5},
            {"F#", 6}, {"Gb", 6},
            {"G", 7},
            {"G#", 8}, {"Ab", 8},
            {"A", 9},
            {"A#", 10}, {"Bb", 10},
            {"B", 11}, {"Cb", 11}
        }};

        for (const auto& pair : nameMap) {
            if (pair.first == name) {
                return PitchClass(pair.second);
            }
        }
        return PitchClass(0);
    }

    // 計算到另一個音高類的上行音程
    int intervalTo(const PitchClass& other) const {
        return (other.value_ - value_ + 12) % 12;
    }

    // 移調
    PitchClass transpose(int semitones) const {
        return PitchClass(value_ + semitones);
    }

    // 取得音名
    std::string name(bool preferFlat = false) const {
        static const std::array<std::string, 12> sharpNames = {
            "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
        };
        static const std::array<std::string, 12> flatNames = {
            "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"
        };
        return preferFlat ? flatNames[value_] : sharpNames[value_];
    }

    // 計算頻率 (A4 = 440Hz)
    float frequency(int octave = 4) const {
        int midiNote = (octave + 1) * 12 + value_;
        return 440.0f * std::pow(2.0f, (midiNote - 69) / 12.0f);
    }

    bool operator==(const PitchClass& other) const { return value_ == other.value_; }
    bool operator!=(const PitchClass& other) const { return value_ != other.value_; }

private:
    int value_;
};

// 常用音高類常數
namespace Pitch {
    static const PitchClass C(0);
    static const PitchClass Db(1);
    static const PitchClass D(2);
    static const PitchClass Eb(3);
    static const PitchClass E(4);
    static const PitchClass F(5);
    static const PitchClass Gb(6);
    static const PitchClass G(7);
    static const PitchClass Ab(8);
    static const PitchClass A(9);
    static const PitchClass Bb(10);
    static const PitchClass B(11);
}

} // namespace JazzArchitect
