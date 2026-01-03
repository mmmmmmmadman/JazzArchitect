#pragma once

#include <JuceHeader.h>
#include "../Core/ChordSymbol.h"
#include <vector>

namespace JazzArchitect {

class StaffNotation : public juce::Component {
public:
    StaffNotation() {
        bgDark_ = juce::Colour(0xff0e0c0c);
        bgMid_ = juce::Colour(0xff201a1a);
        accent_ = juce::Colour(0xffff9eb0);
        accentDim_ = juce::Colour(0xffc08090);
        textLight_ = juce::Colour(0xffffffff);
        textDim_ = juce::Colour(0xffc8b8b8);
    }

    void setChords(const std::vector<ChordSymbol>& chords, int startX = 20, int boxWidth = 85, int boxSpacing = 8) {
        chords_ = chords;
        chordStartX_ = startX;
        chordBoxWidth_ = boxWidth;
        chordBoxSpacing_ = boxSpacing;
        repaint();
    }

    void setCurrentIndex(int index) {
        currentIndex_ = index;
        repaint();
    }

    void clearChords() {
        chords_.clear();
        currentIndex_ = -1;
        repaint();
    }

    void paint(juce::Graphics& g) override {
        if (chords_.empty()) return;

        const int lineSpacing = 7;
        const int staffHeight = 4 * lineSpacing;
        const int trebleTopY = 8;
        const int bassTopY = trebleTopY + staffHeight + 20;
        const int clefWidth = 25;

        drawGrandStaff(g, trebleTopY, bassTopY, lineSpacing, clefWidth);
        drawClefs(g, trebleTopY, bassTopY, lineSpacing);

        for (size_t i = 0; i < chords_.size(); ++i) {
            // Align with chord box center
            int x = chordStartX_ + static_cast<int>(i) * (chordBoxWidth_ + chordBoxSpacing_) + chordBoxWidth_ / 2;
            if (x > getWidth() - 20) break;

            bool isPlaying = (static_cast<int>(i) == currentIndex_);
            drawChordNotes(g, chords_[i], x, trebleTopY, bassTopY, lineSpacing, isPlaying);
        }
    }

private:
    std::vector<ChordSymbol> chords_;
    int currentIndex_ = -1;
    int chordStartX_ = 20;
    int chordBoxWidth_ = 85;
    int chordBoxSpacing_ = 8;

    juce::Colour bgDark_, bgMid_, accent_, accentDim_, textLight_, textDim_;

    void drawGrandStaff(juce::Graphics& g, int trebleY, int bassY, int spacing, int clefWidth) {
        g.setColour(textDim_);

        for (int i = 0; i < 5; ++i) {
            int y = trebleY + i * spacing;
            g.drawLine(static_cast<float>(clefWidth), static_cast<float>(y),
                      static_cast<float>(getWidth() - 5), static_cast<float>(y), 1.0f);
        }

        for (int i = 0; i < 5; ++i) {
            int y = bassY + i * spacing;
            g.drawLine(static_cast<float>(clefWidth), static_cast<float>(y),
                      static_cast<float>(getWidth() - 5), static_cast<float>(y), 1.0f);
        }
    }

    void drawClefs(juce::Graphics& g, int trebleY, int bassY, int spacing) {
        g.setColour(textLight_);
        g.setFont(juce::FontOptions(28.0f));
        g.drawText(juce::String::charToString(0x1D11E), 2, trebleY - 8, 28, 40,
                   juce::Justification::centredLeft);

        g.setFont(juce::FontOptions(24.0f));
        g.drawText(juce::String::charToString(0x1D122), 2, bassY - 2, 28, 35,
                   juce::Justification::centredLeft);
    }

    void drawChordNotes(juce::Graphics& g, const ChordSymbol& chord, int x,
                        int trebleY, int bassY, int spacing, bool isPlaying) {
        auto midiNotes = chord.getMIDINotes(4);

        juce::Colour noteColor = isPlaying ? accent_ : accentDim_;
        g.setColour(noteColor);

        const int noteWidth = 8;
        const int noteHeight = 6;

        for (int midi : midiNotes) {
            int y = midiToStaffY(midi, trebleY, bassY, spacing);

            g.drawEllipse(static_cast<float>(x - noteWidth / 2),
                         static_cast<float>(y - noteHeight / 2),
                         static_cast<float>(noteWidth),
                         static_cast<float>(noteHeight), 1.5f);

            drawLedgerLines(g, midi, x, y, trebleY, bassY, spacing);
            drawAccidental(g, midi, x, y, chord.root());
        }
    }

    int midiToStaffY(int midiNote, int trebleY, int bassY, int spacing) {
        static const int notePositions[] = {0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6};

        int octave = midiNote / 12 - 1;
        int pitchClass = midiNote % 12;
        int notePos = notePositions[pitchClass];

        int b4Position = trebleY + 4 * spacing;
        int stepsFromB4 = (octave - 4) * 7 + notePos - 6;
        int y = b4Position - stepsFromB4 * spacing / 2;

        return y;
    }

    void drawLedgerLines(juce::Graphics& g, int midiNote, int x, int y,
                         int trebleY, int bassY, int spacing) {
        g.setColour(textDim_);
        int lineWidth = 16;

        int trebleBottom = trebleY + 4 * spacing;
        int bassTop = bassY;
        int bassBottom = bassY + 4 * spacing;

        if (midiNote == 60) {
            g.drawLine(static_cast<float>(x - lineWidth / 2), static_cast<float>(y),
                      static_cast<float>(x + lineWidth / 2), static_cast<float>(y), 1.0f);
        }

        if (y > trebleBottom && y < bassTop) {
            int numLines = (y - trebleBottom) / spacing;
            for (int i = 1; i <= numLines; ++i) {
                int lineY = trebleBottom + i * spacing;
                if (std::abs(lineY - y) <= spacing / 2) {
                    g.drawLine(static_cast<float>(x - lineWidth / 2), static_cast<float>(lineY),
                              static_cast<float>(x + lineWidth / 2), static_cast<float>(lineY), 1.0f);
                }
            }
        }

        if (y < trebleY) {
            for (int lineY = trebleY - spacing; lineY >= y - spacing / 2; lineY -= spacing) {
                g.drawLine(static_cast<float>(x - lineWidth / 2), static_cast<float>(lineY),
                          static_cast<float>(x + lineWidth / 2), static_cast<float>(lineY), 1.0f);
            }
        }

        if (y > bassBottom) {
            for (int lineY = bassBottom + spacing; lineY <= y + spacing / 2; lineY += spacing) {
                g.drawLine(static_cast<float>(x - lineWidth / 2), static_cast<float>(lineY),
                          static_cast<float>(x + lineWidth / 2), static_cast<float>(lineY), 1.0f);
            }
        }
    }

    void drawAccidental(juce::Graphics& g, int midiNote, int x, int y, const PitchClass& root) {
        int pitchClass = midiNote % 12;

        static const bool needsAccidental[] = {
            false, true, false, true, false, false, true, false, true, false, true, false
        };

        if (needsAccidental[pitchClass]) {
            g.setColour(textLight_);
            g.setFont(juce::FontOptions(12.0f));

            bool useFlat = (root.value() == 1 || root.value() == 3 ||
                           root.value() == 6 || root.value() == 8 || root.value() == 10);

            juce::String accidental = useFlat ? "b" : "#";
            g.drawText(accidental, x - 18, y - 6, 12, 12, juce::Justification::centred);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StaffNotation)
};

}
