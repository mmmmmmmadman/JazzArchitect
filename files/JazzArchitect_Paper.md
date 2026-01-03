# JazzArchitect: 基於遞歸機率文法的爵士和聲生成系統

**摘要**

本文介紹 JazzArchitect，一個基於 Rohrmeier (2020) 遞歸機率上下文無關文法 (PCFG) 框架的爵士和聲進行生成系統。系統以 C++ 實作，整合 JUCE 音訊框架，提供即時音訊合成與 MIDI 輸出功能。透過 15 參數風格向量控制，系統可生成 9 種不同爵士風格的和聲進行，包括 Bebop、Cool Jazz、Modal Jazz 等。系統實作三種主要替代規則（三全音替代、Backdoor dominant、Coltrane changes）及 Smither (2019) guide-tone 聲部導進優化。本文詳述系統架構、演算法設計與實作細節。

---

## 1. 緒論

### 1.1 研究動機

爵士和聲以其複雜的和弦進行與豐富的替代規則著稱。傳統的爵士即興演奏者透過長期學習掌握這些規則，但對於電腦音樂生成系統而言，如何模擬這種音樂知識仍是一大挑戰。

現有的音樂生成方法大致分為兩類：

1. **資料驅動方法**：使用深度學習（如 LSTM、Transformer）從語料庫學習和聲模式
2. **規則式方法**：基於音樂理論規則進行生成

資料驅動方法雖能產生流暢的結果，但缺乏可解釋性與可控性。規則式方法則能精確控制生成過程，但傳統實作往往過於僵化。

### 1.2 研究目標

本研究旨在結合兩種方法的優點：

1. 採用 Rohrmeier (2020) 提出的遞歸文法框架，提供理論基礎
2. 透過機率權重控制規則選擇，增加生成多樣性
3. 實作風格參數系統，允許跨時代爵士風格生成
4. 提供完整的音訊合成與 MIDI 輸出功能

### 1.3 貢獻

本研究主要貢獻如下：

- 完整實作 Rohrmeier (2020) PCFG 框架於 C++ 環境
- 設計 15 參數風格向量系統，支援 9 種爵士風格預設
- 整合三種主要替代規則與 guide-tone 聲部導進優化
- 開發具有圖形介面的獨立應用程式

---

## 2. 相關研究

### 2.1 爵士和聲的形式文法

Steedman (1984) 首先提出以形式文法分析爵士和弦序列，將爵士和聲視為一種語言結構。該研究定義了基本的生成規則，但未處理機率權重問題。

Rohrmeier (2020) 擴展此框架，提出包含以下核心概念的遞歸文法：

- **Prolongation（延長）**：同功能和弦的重複或展開
- **Preparation（準備）**：屬功能和弦對目標和弦的準備
- **Substitution（替代）**：功能等價和弦的替換

### 2.2 機率上下文無關文法

Harasim et al. (2018, 2020) 將 PCFG 應用於 Jazz Harmony Treebank (JHT)，從 150 首 jazz standards 萃取規則權重。此方法結合了形式文法的結構性與統計方法的靈活性。

### 2.3 聲部導進理論

Smither (2019) 針對爵士和聲提出 guide-tone 聲部導進理論。核心概念為：

- **Guide tones**：每個和弦的第三音與第七音
- **平滑導進**：相鄰和弦的 guide tones 應以最小音程移動
- **目標成本**：理想的聲部導進成本 ≤ 2 半音

### 2.4 替代規則

爵士和聲中常見的替代規則包括：

| 替代類型 | 原和弦 | 替代和弦 | 理論基礎 |
|----------|--------|----------|----------|
| 三全音替代 | V7 | bII7 | 共享三全音 (3rd-7th) |
| Backdoor dominant | V7 | bVII7 | 小調借用 |
| Coltrane changes | I | III7-bVI7-bII7-V7 | 大三度循環 |

---

## 3. 系統架構

### 3.1 整體架構

JazzArchitect 採用分層架構設計：

```
┌────────────────────────────────────────────────┐
│              User Interface Layer              │
│         (JUCE GUI, Parameter Controls)         │
├────────────────────────────────────────────────┤
│              Generation Layer                  │
│  ┌──────────────┐  ┌────────────────────────┐  │
│  │ PCFG Engine  │  │  StyleEngine           │  │
│  │              │──│  - Substitution        │  │
│  │ - Generator  │  │  - Voice Leading Opt   │  │
│  │ - Rules      │  │  - Style Parameters    │  │
│  └──────────────┘  └────────────────────────┘  │
├────────────────────────────────────────────────┤
│              Core Layer                        │
│  ┌──────────────┐  ┌────────────────────────┐  │
│  │ ChordSymbol  │  │  PitchClass            │  │
│  │ ChordQuality │  │  Interval              │  │
│  └──────────────┘  └────────────────────────┘  │
├────────────────────────────────────────────────┤
│              Output Layer                      │
│  ┌──────────────┐  ┌────────────────────────┐  │
│  │ ChordSynth   │  │  MIDIExporter          │  │
│  │ (8-voice)    │  │  (Standard MIDI File)  │  │
│  └──────────────┘  └────────────────────────┘  │
└────────────────────────────────────────────────┘
```

### 3.2 模組說明

#### Core 模組
- **PitchClass**: 音高類表示 (0-11, C=0)
- **ChordQuality**: 和弦品質列舉 (maj7, min7, dom7, hdim7, dim7, aug)
- **ChordSymbol**: 完整和弦符號，包含根音、品質、延伸音與變化音

#### Grammar 模組
- **NonTerminal**: 非終端符號 (S, T, D, SD, PREP, PROL)
- **GrammarRule**: 生成規則定義
- **PCFG**: 機率上下文無關文法引擎
- **Generator**: 自頂向下推導器

#### Style 模組
- **StyleVector**: 15 參數風格向量
- **StylePresets**: 9 種風格預設
- **StyleEngine**: 整合生成與後處理

#### Substitution 模組
- **TritoneSubstitution**: 三全音替代
- **BackdoorSubstitution**: Backdoor dominant
- **ColtraneSubstitution**: Coltrane changes
- **SubstitutionEngine**: 統一替代引擎

#### VoiceLeading 模組
- **GuideTone**: Guide-tone 分析
- **VoiceLeadingOptimizer**: 聲部導進優化

#### Synthesis 模組
- **ChordSynth**: 8 複音和弦合成器
- **ADSREnvelope**: ADSR 包絡
- **SynthVoice**: 單一合成聲部

#### MIDI 模組
- **MIDIExporter**: Standard MIDI File 匯出

---

## 4. 文法引擎

### 4.1 非終端符號

系統定義以下非終端符號：

| 符號 | 名稱 | 功能 |
|------|------|------|
| S | Start | 起始符號（整首曲子）|
| T | Tonic | 主和弦區域 |
| D | Dominant | 屬和弦區域 |
| SD | Subdominant | 下屬和弦區域 |
| PREP | Preparation | 準備區域 |
| PROL | Prolongation | 延長區域 |

### 4.2 核心生成規則

#### Prolongation 規則
延長規則透過重複或展開同功能和弦：

```
T → T T          (主和弦延長)
D → D D          (屬和弦延長)
T → T PROL       (右側延長)
T → PROL T       (左側延長)
```

#### Preparation 規則
準備規則插入屬功能和弦：

```
T → D T          (正格終止)
T → SD T         (變格終止)
D → PREP D       (ii-V 模式)
PREP → ii        (副屬和弦準備)
PREP → V/V       (重屬準備)
```

#### Substitution 規則
替代規則為單一生成規則：

```
V7 → bII7        (三全音替代)
V7 → bVII7       (Backdoor dominant)
I → III-VI-II-V  (Coltrane changes)
```

### 4.3 規則機率

規則機率由 StyleVector 控制。以 ii-V 進行為例：

```
P(D → PREP D) = styleVector.iiVPreference
P(D → D)      = 1 - styleVector.iiVPreference
```

### 4.4 推導演算法

系統採用自頂向下遞歸推導：

```
Algorithm: GenerateProgression
Input: style (StyleVector), length (int), key (PitchClass)
Output: progression (List<ChordSymbol>)

1. Initialize derivation tree with S
2. While tree contains non-terminals:
   a. Select leftmost non-terminal N
   b. Get applicable rules for N
   c. Weight rules by style parameters
   d. Sample rule r according to weights
   e. Apply r, replacing N with RHS symbols
3. Map terminals to ChordSymbols in key
4. Return progression
```

---

## 5. 風格系統

### 5.1 StyleVector 參數

StyleVector 包含 15 個參數，控制生成行為：

| 參數 | 範圍 | 說明 |
|------|------|------|
| tritoneSubProb | 0.0-1.0 | 三全音替代機率 |
| iiVPreference | 0.0-1.0 | ii-V 進行偏好 |
| modalInterchange | 0.0-1.0 | 調式互換頻率 |
| chromaticApproach | 0.0-1.0 | 半音趨近頻率 |
| dominantChainDepth | 1-5 | 屬和弦鏈最大深度 |
| rhythmDensity | 0.0-1.0 | 和弦密度 |
| extensionLevel | 0.0-1.0 | 延伸音使用程度 |
| backdoorProb | 0.0-1.0 | Backdoor dominant 機率 |
| coltraneProb | 0.0-1.0 | Coltrane changes 機率 |
| minorBorrowing | 0.0-1.0 | 小調借用頻率 |
| diminishedUsage | 0.0-1.0 | 減和弦使用頻率 |
| deceptiveResolution | 0.0-1.0 | 假終止機率 |
| plagalCadence | 0.0-1.0 | 變格終止機率 |
| suspensionUsage | 0.0-1.0 | 掛留音使用 |
| pedalPointProb | 0.0-1.0 | 持續低音機率 |

### 5.2 風格預設

系統提供 9 種風格預設：

| 風格 | tritone | iiV | modal | extension | 特徵 |
|------|---------|-----|-------|-----------|------|
| Bebop | 0.30 | 0.90 | 0.20 | 0.70 | 快速 ii-V，豐富延伸音 |
| Cool | 0.20 | 0.70 | 0.30 | 0.50 | 較慢節奏，空間感 |
| Modal | 0.10 | 0.30 | 0.70 | 0.30 | 少功能進行，調式色彩 |
| Hard Bop | 0.35 | 0.85 | 0.25 | 0.60 | Blues 影響，強烈節奏 |
| Post-Bop | 0.40 | 0.60 | 0.60 | 0.80 | 現代和聲，複雜結構 |
| Swing | 0.15 | 0.80 | 0.10 | 0.40 | 傳統進行，簡潔 |
| Fusion | 0.30 | 0.50 | 0.50 | 0.90 | 延伸音豐富，調式混合 |
| Contemporary | 0.35 | 0.40 | 0.70 | 0.85 | 現代技法，自由結構 |
| Blues | 0.20 | 0.60 | 0.40 | 0.50 | Blues 進行，平行和弦 |

### 5.3 風格引擎

StyleEngine 整合生成流程：

```
Algorithm: StyleEngine.generate
Input: style, length, key
Output: optimizedProgression

1. progression = PCFG.generate(style, length, key)
2. progression = SubstitutionEngine.apply(progression, style)
3. progression = VoiceLeadingOptimizer.optimize(progression)
4. Return progression
```

---

## 6. 替代規則

### 6.1 三全音替代

三全音替代 (Tritone Substitution) 基於 V7 與 bII7 共享相同的三全音音程：

- G7 的三全音：B-F (3rd-7th)
- Db7 的三全音：F-Cb (3rd-7th)

兩者共享 B/Cb 與 F，因此可互換。

實作時，系統以 `tritoneSubProb` 機率將 V7 替換為 bII7：

```
if (chord.quality == DOM7 && random() < style.tritoneSubProb):
    newRoot = (chord.root + 6) % 12  // 三全音 = 6 半音
    return ChordSymbol(newRoot, DOM7)
```

### 6.2 Backdoor Dominant

Backdoor dominant 來自小調借用，以 bVII7 取代 V7：

- 傳統：G7 → Cmaj7
- Backdoor：Bb7 → Cmaj7

實作以 `backdoorProb` 控制：

```
if (chord.quality == DOM7 && random() < style.backdoorProb):
    newRoot = (chord.root + 3) % 12  // 小三度上行
    return ChordSymbol(newRoot, DOM7)
```

### 6.3 Coltrane Changes

Coltrane changes 源自 John Coltrane 的 "Giant Steps"，以大三度循環取代簡單進行：

- 傳統：I
- Coltrane：III7 → bVI7 → bII7 → V7 → I

實作時將單一 I 和弦擴展為 5 個和弦的序列：

```
if (chord == I && random() < style.coltraneProb):
    return [
        ChordSymbol(root + 4, DOM7),   // III7
        ChordSymbol(root + 8, MAJ7),   // bVImaj7
        ChordSymbol(root + 1, DOM7),   // bII7
        ChordSymbol(root + 7, DOM7),   // V7
        chord                           // I
    ]
```

---

## 7. 聲部導進優化

### 7.1 Guide-Tone 分析

根據 Smither (2019)，guide tones 為和弦的第三音與第七音：

| 和弦類型 | 第三音 | 第七音 |
|----------|--------|--------|
| Cmaj7 | E (4) | B (11) |
| Dm7 | F (5) | C (0) |
| G7 | B (11) | F (5) |

### 7.2 聲部導進成本

兩和弦間的聲部導進成本定義為 guide tones 的最小移動量：

```
cost(chord1, chord2) = |third1 - third2| + |seventh1 - seventh2|
```

其中音程以最小值計算（考慮八度等價）。

### 7.3 優化演算法

VoiceLeadingOptimizer 檢查每對相鄰和弦，若成本過高則嘗試替代：

```
Algorithm: OptimizeVoiceLeading
Input: progression
Output: optimizedProgression

1. For i = 0 to length-2:
   a. cost = voiceLeadingCost(progression[i], progression[i+1])
   b. If cost > 2:
      - Try tritone substitution on progression[i]
      - Calculate new cost
      - If new cost < original: apply substitution
2. Return optimizedProgression
```

---

## 8. 音訊合成

### 8.1 合成器架構

ChordSynth 為 8 複音和弦合成器，支援三種音色：

| 音色 | 合成方式 | 特徵 |
|------|----------|------|
| Electric Piano | FM 合成 | Rhodes 風格，衰減調變 |
| Organ | 加法合成 | Hammond 風格，泛音疊加 |
| Pad | 減法合成 | 鋸齒波 + 低通濾波 |

### 8.2 ADSR 包絡

每個 SynthVoice 包含獨立 ADSR 包絡：

| 音色 | Attack | Decay | Sustain | Release |
|------|--------|-------|---------|---------|
| Electric Piano | 5ms | 300ms | 0.3 | 400ms |
| Organ | 10ms | 50ms | 0.9 | 100ms |
| Pad | 200ms | 500ms | 0.7 | 800ms |

### 8.3 和弦配置

ChordSymbol.getMIDINotes() 產生 MIDI 音符：

```
Algorithm: getMIDINotes
Input: baseOctave
Output: midiNotes[]

1. rootMidi = baseOctave * 12 + root
2. Add rootMidi to notes
3. For each interval in quality.intervals:
   - Add (rootMidi + interval) to notes
4. For each extension:
   - Add appropriate extension note
5. Return notes
```

---

## 9. 實作細節

### 9.1 開發環境

- **語言**: C++17
- **框架**: JUCE 7.x
- **建構**: CMake 3.22+
- **平台**: macOS (ARM64/x86_64)

### 9.2 檔案結構

```
cpp/
├── Source/
│   ├── Core/           # 核心類別
│   ├── Grammar/        # 文法引擎
│   ├── Style/          # 風格系統
│   ├── Substitution/   # 替代規則
│   ├── VoiceLeading/   # 聲部導進
│   ├── Synthesis/      # 音訊合成
│   ├── MIDI/           # MIDI 輸出
│   ├── Main.cpp
│   ├── MainComponent.h
│   └── MainComponent.cpp
├── JUCE/               # JUCE 子模組
├── CMakeLists.txt
└── build/
```

### 9.3 編譯方式

```bash
cd cpp
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### 9.4 應用程式規格

- **檔案大小**: 約 11MB
- **最低需求**: macOS 10.13+
- **音訊**: CoreAudio, 48kHz
- **CPU**: 極低（< 1%）

---

## 10. 使用者介面

### 10.1 介面配置

```
┌─────────────────────────────────────────────────────────────┐
│ Jazz Architect                                              │
├─────────────────────────────────────────────────────────────┤
│ Style: [Bebop   v]  Key: [C      v]  Sound: [E.Piano  v]   │
│ BPM: [===120===]    Length: [===8===]                      │
│                                                             │
│ [Generate]  [Play]  [Stop]  [Export MIDI]                  │
├─────────────────────────────────────────────────────────────┤
│                    CHORD PROGRESSION                        │
│  ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐     │
│  │Dm7   │ │G7    │ │Cmaj7 │ │A7    │ │Dm7   │ │G7    │     │
│  └──────┘ └──────┘ └──────┘ └──────┘ └──────┘ └──────┘     │
├─────────────────────────────────────────────────────────────┤
│ Tritone [═══○═══]  ii-V [═══════○]  Modal [═○═══════]      │
│ Ext     [═════○═]                                          │
└─────────────────────────────────────────────────────────────┘
```

### 10.2 操作流程

1. 選擇風格預設（或調整參數滑桿）
2. 選擇調性與長度
3. 點擊 Generate 生成新進行
4. 點擊 Play 播放（可選擇音色）
5. 點擊 Export MIDI 匯出檔案

---

## 11. 結論與未來工作

### 11.1 成果總結

本研究成功實作 JazzArchitect 系統，達成以下目標：

1. **理論整合**: 完整實作 Rohrmeier (2020) PCFG 框架
2. **風格控制**: 15 參數風格向量與 9 種預設
3. **替代規則**: 三全音、Backdoor、Coltrane 替代
4. **聲部優化**: Smither (2019) guide-tone 導進
5. **實用工具**: 圖形介面、音訊合成、MIDI 輸出

### 11.2 未來工作

1. **語料庫訓練**: 從 JHT 萃取規則機率
2. **評估整合**: 加入 MusDr 評估指標
3. **聽覺實驗**: 進行 A/B 比較測試
4. **樂譜顯示**: 加入五線譜視覺化
5. **即時 MIDI**: 連接外部合成器
6. **跨平台**: Windows/Linux 版本

---

## 參考文獻

1. Rohrmeier, M. (2020). Towards a Syntax of Jazz Harmony. *Music Theory and Analysis*, 7(1), 1-62.

2. Steedman, M. J. (1984). A generative grammar for jazz chord sequences. *Music Perception*, 2(1), 52-77.

3. Smither, S. (2019). Guide-tone voice leading in jazz. *Music Theory Online*, 25(2).

4. Harasim, D., Finkensiep, C., Ericson, P., O'Donnell, T. J., & Rohrmeier, M. (2020). The jazz harmony treebank. *Proceedings of ISMIR 2020*.

5. Tymoczko, D. (2006). The geometry of musical chords. *Science*, 313(5783), 72-74.

6. Wu, S. L., & Yang, Y. H. (2020). The jazz transformer on the front line: Exploring the shortcomings of AI-composed music through quantitative measures. *Proceedings of ISMIR 2020*.

7. Granroth-Wilding, M., & Steedman, M. (2014). A robust parser-interpreter for jazz chord sequences. *Journal of New Music Research*, 43(4), 355-374.

---

## 附錄 A: 和弦品質音程表

| ChordQuality | 音程 (半音) |
|--------------|-------------|
| MAJ7 | 0, 4, 7, 11 |
| MIN7 | 0, 3, 7, 10 |
| DOM7 | 0, 4, 7, 10 |
| HDIM7 | 0, 3, 6, 10 |
| DIM7 | 0, 3, 6, 9 |
| AUG | 0, 4, 8 |
| MAJ6 | 0, 4, 7, 9 |
| MIN6 | 0, 3, 7, 9 |

## 附錄 B: 非終端符號終端映射

| 非終端 | 功能 | 常見終端 |
|--------|------|----------|
| T | Tonic | Imaj7, vi7, iii7 |
| D | Dominant | V7, vii-7 |
| SD | Subdominant | IV, ii7 |
| PREP | Preparation | ii7, IV |

---

*JazzArchitect v1.0 - 2026-01-02*
