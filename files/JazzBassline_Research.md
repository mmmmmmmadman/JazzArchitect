# Jazz Bassline 自動生成系統研究

## 摘要

本研究探討爵士音樂中低音線 (Bassline) 的自動生成技術，涵蓋 Walking Bass、Two-Feel、Latin/Bossa Nova 等多種風格。透過分析現有學術文獻與實務演奏規則，設計一套基於規則與機率模型的生成演算法，並規劃其在 Jazz Architect 系統中的實作架構。

---

## 1. 緒論

### 1.1 研究背景

爵士音樂中的低音線扮演三重角色：
1. **和聲基礎** - 標示和弦根音與和聲進行
2. **節奏驅動** - 與鼓手共同建立律動感 (Groove)
3. **旋律對位** - 創造與主旋律互動的低音旋律

自動生成高品質的爵士低音線是一項具挑戰性的任務，需同時考量和聲規則、風格特徵、聲部導進 (Voice Leading) 與節奏變化。

### 1.2 研究目標

1. 系統性整理爵士低音線的構成規則
2. 分析不同爵士風格的低音線特徵差異
3. 設計可參數化的低音線生成演算法
4. 規劃與現有 Jazz Architect 系統的整合架構

---

## 2. 文獻回顧

### 2.1 核心學術研究

#### 2.1.1 輪廓導向生成法 (Contour-Based Approach)

**Dias, Guedes & Marques (2013)** - "A Contour-Based Jazz Walking Bass Generator"

此研究提出基於旋律輪廓的即時 Walking Bass 生成演算法：
- 使用預定義的輪廓模式 (上行、下行、拱形、波形)
- 根據和聲進行選擇適當的輪廓
- 在 Max/MSP 環境中實作

**核心概念**：Walking Bass 的基本思想是從一個和弦走向下一個和弦，用和弦音或音階音填充中間拍點，描繪出平滑的旋律線或模式。

#### 2.1.2 隱藏馬可夫模型法 (HMM Approach)

**Shiga & Kitahara (2021)** - "Generating Walking Bass Lines with HMM"

此研究使用隱藏馬可夫模型生成 Walking Bass：
- 隱藏狀態結合音高類別 (Pitch Class) 與節拍位置 (Metric Position)
- 學習不同節拍位置的音符分布差異
- 可生成包含經過音的流暢低音線

**關鍵發現**：模型能學習到不同節拍位置的音符選擇傾向，生成具音樂性的低音線。

#### 2.1.3 深度學習法

**Abeßer et al. (2017)** - "Deep Learning for Jazz Walking Bass Transcription"

使用深度神經網路 (DNN) 進行低音線轉錄：
- 從混合頻譜圖映射到強調低音線的顯著性表示
- 為理解實際演奏和弦提供線索

**Riley et al. (2023)** - FiloBass 資料集

提供 48 首手動驗證的爵士低音轉錄，包含音訊、MIDI 與 MusicXML：
- 揭示 Walking Bass 通常在穩定的四分音符脈動上演奏和弦根音
- 半音接近音 (Semitone Approach) 主導和弦過渡

### 2.2 相關系統

| 系統 | 方法 | 特點 |
|------|------|------|
| T2G | 規則導向 | 和聲分析 + 節奏組伴奏生成 |
| MuseGAN | GAN | 多軌生成含低音軌 |
| Hung et al. | BGRU-VAE | 遷移學習處理爵士資料不足 |

---

## 3. 低音線類型與風格特徵

### 3.1 Walking Bass (四拍行進低音)

**定義**：連續的四分音符序列，驅動音樂向前推進，通常與鼓手的疊鈸配合。

#### 構成規則

| 拍點 | 功能 | 音符選擇 |
|------|------|---------|
| Beat 1 | 和聲確立 | 根音 (最常見)、三音、五音 |
| Beat 2 | 節奏強調 | 和弦音、音階音、經過音 |
| Beat 3 | 和聲支撐 | 五音、三音、七音 |
| Beat 4 | 接近音 | 半音接近、全音接近、五度接近 |

#### 接近音技巧 (Approach Notes)

1. **半音接近** (Chromatic Approach)
   - 從目標音上方或下方半音進入
   - 例：進入 F 和弦時，Beat 4 可彈 E 或 Gb

2. **全音接近** (Diatonic Approach)
   - 使用音階內音符接近
   - 更平順但張力較低

3. **五度接近** (Fifth Approach)
   - 從目標根音的五度上方進入
   - 強烈的和聲解決感

4. **包圍接近** (Enclosure)
   - 從上下兩方包圍目標音
   - 例：E-Gb-F 或 Gb-E-F

### 3.2 Two-Feel (二拍感)

**定義**：主要使用二分音符，每小節兩個主音，常用於樂曲開頭 (Head) 或慢速曲目。

#### 常見節奏模式

| 模式 | 節奏型態 | 使用情境 |
|------|---------|---------|
| 純二拍 | ♩. ♩. | 極慢速抒情曲 |
| 標準 | ♩. ♩. ♩ ♩ | 標準抒情曲 |
| 過渡型 | ♩. ♩ ♩ ♩ ♩ | 準備進入 Walking |

#### Sam Jones 範例 ("Autumn Leaves")

在 Head 段落使用 Two-Feel：節奏型態為 ♩.-♩.-♩.-♩-♩，保持穩定的二拍感但有適度裝飾。

### 3.3 Latin Bass

#### 3.3.1 Bossa Nova

**特徵**：
- 如同半速演奏的 Samba
- 以八分音符書寫
- Beat 1 短促，Beat 3 強調且延長

**基本模式**：
```
|: R . . 5 . . R . :|
   1   2   3   4
```

**進階模式**：
```
|: R . . 5 . (5) R (R) :|
   1   2   3    4
(括號表示切分音前置)
```

#### 3.3.2 Tumbao (Afro-Cuban)

**特徵**：
- 強調特定節奏型態
- 主要使用根音與五音
- 與 Montuno 鋼琴型態配合

**典型節奏**：
```
|: . R . . 5 . R . :|
   1   2   3   4
(anticipation on "and" of 2 and 4)
```

### 3.4 風格比較表

| 風格 | 節奏密度 | 主要音符 | 半音使用 | 切分音 |
|------|---------|---------|---------|--------|
| Swing Walking | 高 (4/bar) | R-3-5-7 | 多 (接近音) | 少 |
| Bebop | 高 | R-3-5-7 + 延伸 | 極多 | 中 |
| Modal | 中-低 | R-5 為主 | 少 | 少 |
| Cool | 中 | R-3-5-7 | 中 | 少 |
| Bossa Nova | 低 (2/bar) | R-5 | 少 | 多 |
| Latin Swing | 中 | R-5-7 | 中 | 多 |
| Ballad | 極低 | R-5 | 少 | 無 |

---

## 4. 生成演算法設計

### 4.1 系統架構

```
┌─────────────────────────────────────────────────────────┐
│                    BasslineGenerator                     │
├─────────────────────────────────────────────────────────┤
│  Input:                                                  │
│  - ChordProgression (和弦進行)                           │
│  - StyleVector (風格參數)                                │
│  - Tempo (速度)                                          │
│  - Key (調性)                                            │
├─────────────────────────────────────────────────────────┤
│  Process:                                                │
│  1. Style Analyzer → 決定節奏密度與模式                   │
│  2. Beat Allocator → 分配每拍的功能角色                   │
│  3. Note Selector  → 選擇音符 (和弦音/音階音/接近音)      │
│  4. Contour Shaper → 調整旋律輪廓                        │
│  5. Voice Leader   → 優化聲部導進                        │
├─────────────────────────────────────────────────────────┤
│  Output:                                                 │
│  - MIDI Note Sequence                                    │
│  - Velocity Contour                                      │
│  - Articulation Hints                                    │
└─────────────────────────────────────────────────────────┘
```

### 4.2 風格參數向量 (BassStyleVector)

```cpp
struct BassStyleVector {
    // 節奏參數
    float rhythmDensity;      // 0.0 = Two-Feel, 1.0 = Walking
    float syncopation;        // 0.0 = On-beat, 1.0 = Heavy syncopation
    float swingRatio;         // 0.5 = Straight, 0.67 = Swing

    // 音符選擇參數
    float chromaticism;       // 半音使用頻率
    float chordToneRatio;     // 和弦音 vs 音階音比例
    float rootEmphasis;       // 根音強調程度
    float approachVariety;    // 接近音變化程度

    // 輪廓參數
    float contourRange;       // 音域範圍 (八度)
    float contourSmoothness;  // 輪廓平滑度
    float repeatAvoidance;    // 避免重複傾向
};
```

### 4.3 風格預設

```cpp
// Walking Swing
BassStyleVector SWING_WALKING = {
    .rhythmDensity = 1.0,
    .syncopation = 0.1,
    .swingRatio = 0.67,
    .chromaticism = 0.4,
    .chordToneRatio = 0.7,
    .rootEmphasis = 0.8,
    .approachVariety = 0.6,
    .contourRange = 1.5,
    .contourSmoothness = 0.7,
    .repeatAvoidance = 0.8
};

// Bebop
BassStyleVector BEBOP = {
    .rhythmDensity = 1.0,
    .syncopation = 0.2,
    .swingRatio = 0.6,  // Lighter swing
    .chromaticism = 0.7,
    .chordToneRatio = 0.5,
    .rootEmphasis = 0.6,
    .approachVariety = 0.9,
    .contourRange = 2.0,
    .contourSmoothness = 0.5,
    .repeatAvoidance = 0.9
};

// Bossa Nova
BassStyleVector BOSSA_NOVA = {
    .rhythmDensity = 0.3,
    .syncopation = 0.6,
    .swingRatio = 0.5,  // Straight
    .chromaticism = 0.2,
    .chordToneRatio = 0.9,
    .rootEmphasis = 0.95,
    .approachVariety = 0.3,
    .contourRange = 1.0,
    .contourSmoothness = 0.9,
    .repeatAvoidance = 0.3
};

// Ballad Two-Feel
BassStyleVector BALLAD = {
    .rhythmDensity = 0.2,
    .syncopation = 0.0,
    .swingRatio = 0.55,
    .chromaticism = 0.1,
    .chordToneRatio = 0.95,
    .rootEmphasis = 0.98,
    .approachVariety = 0.2,
    .contourRange = 0.8,
    .contourSmoothness = 0.95,
    .repeatAvoidance = 0.2
};
```

### 4.4 核心演算法

#### 4.4.1 節拍分配器 (Beat Allocator)

```cpp
enum class BeatFunction {
    ROOT,           // 根音 (和聲確立)
    CHORD_TONE,     // 和弦音 (3, 5, 7)
    SCALE_TONE,     // 音階音
    APPROACH,       // 接近音
    CHROMATIC,      // 半音經過音
    REST            // 休止
};

std::vector<BeatFunction> allocateBeats(
    const ChordSymbol& chord,
    const ChordSymbol& nextChord,
    const BassStyleVector& style,
    int beatsPerBar = 4)
{
    std::vector<BeatFunction> beats(beatsPerBar);

    if (style.rhythmDensity >= 0.8) {
        // Walking: 四拍全填
        beats[0] = BeatFunction::ROOT;           // 或 CHORD_TONE
        beats[1] = BeatFunction::CHORD_TONE;     // 或 SCALE_TONE
        beats[2] = BeatFunction::CHORD_TONE;     // 強調 5 或 3
        beats[3] = BeatFunction::APPROACH;       // 接近下一和弦
    }
    else if (style.rhythmDensity >= 0.4) {
        // Half-walking
        beats[0] = BeatFunction::ROOT;
        beats[1] = BeatFunction::REST;
        beats[2] = BeatFunction::CHORD_TONE;
        beats[3] = random() < 0.5 ? BeatFunction::APPROACH : BeatFunction::REST;
    }
    else {
        // Two-feel
        beats[0] = BeatFunction::ROOT;
        beats[1] = BeatFunction::REST;
        beats[2] = BeatFunction::CHORD_TONE;  // 通常是 5
        beats[3] = BeatFunction::REST;
    }

    return beats;
}
```

#### 4.4.2 音符選擇器 (Note Selector)

```cpp
int selectNote(
    const ChordSymbol& chord,
    const ChordSymbol& nextChord,
    BeatFunction function,
    int previousNote,
    const BassStyleVector& style,
    int baseOctave = 2)
{
    std::vector<int> candidates;
    int root = chord.root().value() + (baseOctave * 12);

    switch (function) {
        case BeatFunction::ROOT:
            return root;

        case BeatFunction::CHORD_TONE: {
            // 收集和弦音 (3, 5, 7)
            auto pitches = chord.getPitchClasses();
            for (auto pc : pitches) {
                int note = pc.value() + (baseOctave * 12);
                // 調整八度使其在合理範圍
                while (note < root - 5) note += 12;
                while (note > root + 12) note -= 12;
                candidates.push_back(note);
            }
            break;
        }

        case BeatFunction::SCALE_TONE: {
            // 根據和弦品質選擇音階
            auto scale = getScaleForChord(chord);
            for (int degree : scale) {
                int note = chord.root().value() + degree + (baseOctave * 12);
                while (note < root - 5) note += 12;
                while (note > root + 12) note -= 12;
                candidates.push_back(note);
            }
            break;
        }

        case BeatFunction::APPROACH: {
            int target = nextChord.root().value() + (baseOctave * 12);
            while (target < root - 2) target += 12;
            while (target > root + 14) target -= 12;

            if (random() < style.chromaticism) {
                // 半音接近
                candidates.push_back(target - 1);  // 下方半音
                candidates.push_back(target + 1);  // 上方半音
            } else {
                // 全音或五度接近
                candidates.push_back(target - 2);  // 下方全音
                candidates.push_back(target + 7);  // 五度上方
            }
            break;
        }

        case BeatFunction::CHROMATIC: {
            // 半音經過
            candidates.push_back(previousNote + 1);
            candidates.push_back(previousNote - 1);
            break;
        }

        case BeatFunction::REST:
            return -1;  // 表示休止
    }

    // 根據聲部導進選擇最佳候選
    return selectBestCandidate(candidates, previousNote, style);
}
```

#### 4.4.3 聲部導進優化

```cpp
int selectBestCandidate(
    const std::vector<int>& candidates,
    int previousNote,
    const BassStyleVector& style)
{
    if (candidates.empty()) return previousNote;

    float bestScore = -999.0f;
    int bestNote = candidates[0];

    for (int note : candidates) {
        float score = 0.0f;
        int interval = std::abs(note - previousNote);

        // 平滑度評分：小音程得分高
        if (interval <= 2) score += 2.0f * style.contourSmoothness;
        else if (interval <= 4) score += 1.0f * style.contourSmoothness;
        else if (interval <= 7) score += 0.5f;
        else score -= 0.5f;

        // 避免重複
        if (note == previousNote) {
            score -= 2.0f * style.repeatAvoidance;
        }

        // 偏好向下移動 (低音線傳統)
        if (note < previousNote) score += 0.3f;

        // 隨機因素
        score += random() * 0.5f;

        if (score > bestScore) {
            bestScore = score;
            bestNote = note;
        }
    }

    return bestNote;
}
```

### 4.5 Latin 模式特殊處理

```cpp
struct LatinPattern {
    std::vector<float> timings;      // 相對於小節起點的時間 (0.0-4.0)
    std::vector<BeatFunction> functions;
    std::string name;
};

LatinPattern BOSSA_NOVA_BASIC = {
    .timings = {0.0, 2.0},
    .functions = {BeatFunction::ROOT, BeatFunction::CHORD_TONE},
    .name = "Bossa Basic"
};

LatinPattern BOSSA_NOVA_SYNCOPATED = {
    .timings = {0.0, 1.5, 2.0, 3.5},
    .functions = {BeatFunction::ROOT, BeatFunction::CHORD_TONE,
                  BeatFunction::CHORD_TONE, BeatFunction::APPROACH},
    .name = "Bossa Syncopated"
};

LatinPattern TUMBAO = {
    .timings = {0.5, 2.0, 2.5},
    .functions = {BeatFunction::ROOT, BeatFunction::CHORD_TONE, BeatFunction::ROOT},
    .name = "Tumbao"
};
```

---

## 5. 程式實作架構

### 5.1 類別設計

```
JazzArchitect/
├── Source/
│   ├── Bassline/
│   │   ├── BasslineGenerator.h     # 主生成器
│   │   ├── BassStyleVector.h       # 風格參數
│   │   ├── BassStylePresets.h      # 預設風格
│   │   ├── BeatAllocator.h         # 節拍分配
│   │   ├── NoteSelector.h          # 音符選擇
│   │   ├── ApproachNotes.h         # 接近音邏輯
│   │   ├── LatinPatterns.h         # Latin 節奏型態
│   │   └── BassVoiceLeader.h       # 聲部導進
│   └── ...
```

### 5.2 主要介面

```cpp
namespace JazzArchitect {

class BasslineGenerator {
public:
    struct BassNote {
        int midiNote;
        double startBeat;
        double duration;
        float velocity;
        bool isApproach;
    };

    struct GeneratorConfig {
        BassStyleVector style;
        int baseOctave = 2;
        float humanize = 0.1f;  // 時間/力度微調
    };

    // 生成完整低音線
    std::vector<BassNote> generate(
        const std::vector<ChordSymbol>& chords,
        const GeneratorConfig& config);

    // 即時生成 (逐小節)
    std::vector<BassNote> generateBar(
        const ChordSymbol& current,
        const ChordSymbol& next,
        const GeneratorConfig& config,
        int previousNote);

    // 風格預設
    static BassStyleVector getStylePreset(const std::string& styleName);

private:
    BeatAllocator beatAllocator_;
    NoteSelector noteSelector_;
    BassVoiceLeader voiceLeader_;
};

} // namespace JazzArchitect
```

### 5.3 與現有系統整合

```cpp
// MainComponent.h 新增
class MainComponent {
    // ...現有成員

    // 新增 Bassline 相關
    std::unique_ptr<JazzArchitect::BasslineGenerator> bassGenerator_;
    std::vector<JazzArchitect::BasslineGenerator::BassNote> currentBassline_;
    bool bassEnabled_ = true;

    juce::ToggleButton bassToggle_{"Bass"};
    juce::ComboBox bassStyleSelector_;

    void generateBassline();
    void playBassNote(const BasslineGenerator::BassNote& note);
};
```

### 5.4 合成器擴展

```cpp
// ChordSynth.h 擴展
class ChordSynth {
public:
    enum class VoiceType {
        CHORD,      // 現有和弦聲部
        BASS        // 新增低音聲部
    };

    void playBassNote(int midiNote, float velocity = 0.8f);
    void stopBassNote(int midiNote);

    // 低音音色設定
    void setBassTimbre(int timbreType);  // 0=Acoustic, 1=Electric, 2=Synth

private:
    // 低音專用 Voice 分配
    std::vector<Voice> bassVoices_;  // 單音或少量複音
};
```

---

## 6. 驗證與評估

### 6.1 評估指標

| 指標 | 說明 | 目標值 |
|------|------|--------|
| 和聲正確性 | Beat 1 為根音的比例 | > 90% |
| 接近音解決 | 接近音正確解決比例 | > 95% |
| 音程跳躍 | 連續音程 > 完全五度比例 | < 15% |
| 音符重複 | 連續相同音符比例 | < 10% |
| 風格區分度 | 不同風格的可辨識性 | 主觀評估 |

### 6.2 測試計畫

1. **規則驗證測試**
   - 使用標準 ii-V-I 進行測試各種風格輸出
   - 檢查 Beat 1 根音率、接近音正確性

2. **風格比較測試**
   - 相同和弦進行，不同風格參數
   - 聽覺比較與專家評估

3. **與真實演奏比較**
   - 使用 FiloBass 資料集比對
   - 統計特徵相似度分析

---

## 7. 結論與展望

本研究建立了爵士低音線自動生成的完整框架，涵蓋：

1. **理論基礎** - 整理 Walking Bass、Two-Feel、Latin 等風格的構成規則
2. **演算法設計** - 基於節拍功能分配與聲部導進的生成方法
3. **風格參數化** - 可調整的 BassStyleVector 支援多種風格
4. **實作規劃** - 與 Jazz Architect 系統的整合架構

### 未來工作

1. **機器學習增強** - 使用 HMM 或 RNN 學習真實演奏的細微變化
2. **互動式生成** - 即時響應和弦變化的低音線調整
3. **多樂器協調** - 低音與鼓組的節奏互動
4. **表情處理** - 力度變化、滑音、裝飾音等

---

## 參考文獻

1. Dias, R., Guedes, C., & Marques, T. (2013). A Contour-Based Jazz Walking Bass Generator. *Sound and Music Computing Conference*.

2. Shiga, A., & Kitahara, T. (2021). Generating Walking Bass Lines with HMM. *Creativity in Music Systems and Communication*. Springer.

3. Abeßer, J., Balke, S., Frieler, K., Pfleiderer, M., & Müller, M. (2017). Deep Learning for Jazz Walking Bass Transcription.

4. Riley, J., et al. (2023). FiloBass: A Dataset of Jazz Bass Transcriptions.

5. Smither, S. (2019). Guide-Tone Space: Navigating Voice-Leading Syntax in Tonal Jazz. *Music Theory Online*, 25(2).

---

## 附錄 A: 常用和弦的低音音階選擇

| 和弦品質 | 建議音階 | 可用音程 |
|---------|---------|---------|
| maj7 | Ionian / Lydian | 1-2-3-4-5-6-7 |
| min7 | Dorian / Aeolian | 1-2-b3-4-5-6-b7 |
| dom7 | Mixolydian | 1-2-3-4-5-6-b7 |
| dom7(alt) | Altered | 1-b2-b3-b4-b5-b6-b7 |
| m7b5 | Locrian | 1-b2-b3-4-b5-b6-b7 |
| dim7 | Whole-Half Dim | 1-2-b3-4-b5-b6-6-7 |

## 附錄 B: 節拍位置與音符功能對應表

### Walking Bass (Swing)

| 小節位置 | Beat 1 | Beat 2 | Beat 3 | Beat 4 |
|---------|--------|--------|--------|--------|
| 功能優先序 | R > 3 > 5 | 5 > 3 > Scale | 5 > 3 > 7 | Approach |
| 張力程度 | 低 | 中 | 低 | 高 |
| 音量 | mf | mp | mf | mp |

### Two-Feel (Ballad)

| 小節位置 | Beat 1 | Beat 2 | Beat 3 | Beat 4 |
|---------|--------|--------|--------|--------|
| 功能優先序 | R | (rest) | 5 > 3 | (rest) |
| 張力程度 | 低 | - | 低 | - |
| 音量 | mf | - | mp | - |

---

*文件版本: 1.0*
*最後更新: 2026-01-03*
