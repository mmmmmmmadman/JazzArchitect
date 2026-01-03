# Jazz Harmony Generator — Development Guide

## Communication Preferences

- 請使用繁體中文
- 沒有表情符號的極簡模式
- 對話不要顯示程式碼內容

## Project Overview

Rule-based jazz harmony progression generator using Rohrmeier (2020) recursive grammar framework with style parameterization for cross-era jazz generation.

## Project Structure

```
JazzArchitect/
├── src/
│   ├── grammar/          # PCFG rule engine
│   │   ├── __init__.py
│   │   ├── rules.py      # Production rule definitions
│   │   ├── parser.py     # Tree structure parsing
│   │   └── generator.py  # Top-down derivation
│   ├── substitution/     # Substitution rule modules
│   │   ├── __init__.py
│   │   ├── tritone.py    # Tritone substitution
│   │   ├── backdoor.py   # Backdoor dominant
│   │   └── coltrane.py   # Coltrane changes
│   ├── voiceleading/     # Voice leading constraints
│   │   ├── __init__.py
│   │   └── guidetone.py  # Guide-tone optimization (Smither 2019)
│   └── style/            # Style parameter control
│       ├── __init__.py
│       ├── vectors.py    # StyleVector definitions
│       └── presets.py    # Bebop, Cool, Modal, Post-bop
├── data/
│   └── jht/              # Jazz Harmony Treebank data
├── tests/
├── examples/
├── files/                # Research documents
│   ├── FINAL_REPORT.md
│   ├── phase1_theoretical_foundations.md
│   ├── phase2_corpus_analysis.md
│   ├── phase3_rule_system_design.md
│   └── phase4_validation.md
└── CLAUDE.md
```

## Core Classes

### ChordSymbol
```python
class ChordSymbol:
    root: PitchClass        # 0-11 (C=0, C#=1, ..., B=11)
    quality: ChordQuality   # maj7, min7, dom7, hdim7, dim7, aug
    extensions: List[int]   # [9, 11, 13, ...]
    alterations: Dict       # {'5': 'b', '9': '#', ...}
```

### PitchClass
```python
class PitchClass:
    value: int              # 0-11

    @classmethod
    def from_name(cls, name: str) -> 'PitchClass'

    def interval_to(self, other: 'PitchClass') -> int
    def transpose(self, semitones: int) -> 'PitchClass'
```

### GrammarRule
```python
class GrammarRule:
    lhs: NonTerminal        # Left-hand side symbol
    rhs: List[Symbol]       # Right-hand side symbols
    prob: float             # Rule probability (0.0-1.0)
    rule_type: RuleType     # prolongation, preparation, substitution
```

### StyleVector
```python
@dataclass
class StyleVector:
    tritone_sub_prob: float       # 0.0-1.0
    ii_v_preference: float        # Preference for ii-V vs direct V
    modal_interchange: float      # Borrowing frequency
    chromatic_approach: float     # Chromatic neighbor frequency
    dominant_chain_depth: int     # Max recursion depth
    rhythm_density: float         # Chord change frequency
```

## Grammar Rules Reference

### Non-terminals
```
N = {S, T, D, SD, Phrase, Prol, Prep}

S   : Start symbol (entire piece)
T   : Tonic constituent
D   : Dominant constituent
SD  : Subdominant constituent
Prol: Prolongation
Prep: Preparation
```

### Core Production Rules

**Prolongation** (X → X X)
```python
"T -> T T"      # Tonic prolongation
"D -> D D"      # Dominant prolongation
"T -> T Prol"   # Right prolongation
"T -> Prol T"   # Left prolongation
```

**Preparation** (X → V/X X)
```python
"T -> D T"      # Authentic cadence
"T -> SD T"     # Plagal preparation (blues)
"D -> Prep D"   # ii-V pattern
"Prep -> ii"    # Diatonic supertonic
"Prep -> V/V"   # Secondary dominant
```

**Substitution** (unary)
```python
"V7 -> bII7"    # Tritone substitution
"V7 -> bVII7"   # Backdoor dominant
"I -> bIII"     # Relative major (minor context)
```

### Substitution Patterns

| Type | Original | Substitution | Voice Leading |
|------|----------|--------------|---------------|
| Tritone | V7 → I | bII7 → I | Root ↓m2 |
| Backdoor | V7 → I | bVII7 → I | iv borrowed dominant |
| Coltrane | I | III7 → bVI7 → bII7 → V7 → I | Major 3rd cycle |
| Deceptive | V7 → I | V7 → vi | Standard deceptive |

## Voice Leading (Smither 2019)

**Guide tones** = 3rd and 7th of each chord

```python
def voice_leading_cost(chord1, chord2):
    """Calculate guide-tone voice leading cost"""
    gt1 = (chord1.third, chord1.seventh)
    gt2 = (chord2.third, chord2.seventh)
    return sum(min_semitone_distance(a, b)
               for a, b in zip(gt1, gt2))

# Target: cost <= 2 semitones
```

| Progression | 3rd movement | 7th movement |
|-------------|--------------|--------------|
| ii7 → V7    | stays (→3rd) | ↓m2 (→7th)   |
| V7 → Imaj7  | ↓m2 (→7th)   | stays (→3rd) |
| V7 → bII7   | ↑m2          | stays        |

## Style Presets

```python
BEBOP    = StyleVector(0.3, 0.9, 0.2, 0.4, 4, 0.8)
COOL     = StyleVector(0.2, 0.7, 0.3, 0.2, 3, 0.5)
MODAL    = StyleVector(0.1, 0.3, 0.5, 0.1, 2, 0.3)
POSTBOP  = StyleVector(0.4, 0.6, 0.6, 0.5, 5, 0.6)
```

## Development Sequence

1. **Core Classes** - Implement ChordSymbol, PitchClass
2. **PCFG Structure** - Define grammar rule classes
3. **JHT Integration** - Extract rule weights from Jazz Harmony Treebank
4. **Generator** - Implement top-down derivation
5. **Substitution Module** - Add tritone, backdoor, Coltrane rules
6. **Voice Leading** - Guide-tone constraint optimization
7. **Style Integration** - Connect style vectors to PCFG probabilities
8. **Evaluation** - MusDr metrics integration

## Key Resources

| Resource | URL |
|----------|-----|
| JHT | https://github.com/DCMLab/JazzHarmonyTreebank |
| MusDr | https://github.com/slSeanWU/MusDr |
| PACFG Code | https://zenodo.org/records/1492367 |
| Rohrmeier PDF | https://music.arts.uci.edu/abauer/4.3/readings/Rohrmeier_Syntax_Jazz_Harmony.pdf |
| Jazzomat/WJD | https://jazzomat.hfm-weimar.de |
| Tree Annotation | https://dcmlab.github.io/tree-annotation-code/ |

## Testing Standards

1. **Grammar Validity** - Generated progressions parseable by JHT grammar
2. **Voice Leading** - Guide-tone cost ≤ 2 semitones
3. **Style Differentiation** - Parameter changes produce audible style differences
4. **MusDr Metrics**:
   - H: Pitch-class entropy
   - GS: Grooving similarity
   - CPI: Chord progression irregularity
   - SI: Structureness indicator

## Verified Literature

| Paper | Publication | Content |
|-------|-------------|---------|
| Rohrmeier (2020) | Music Theory and Analysis 7(1):1-62 | Recursive harmony grammar |
| Steedman (1984) | Music Perception 2(1):52-77 | Jazz chord sequence grammar |
| Smither (2019) | Music Theory Online 25(2) | Guide-tone voice leading |
| Harasim et al. (2020) | EPFL PhD Thesis | PACFG framework + JHT |
| Wu & Yang (2020) | - | MusDr evaluation metrics |

## Commands

```bash
# Run tests
pytest tests/

# Generate progression
python -m src.grammar.generator --style bebop --length 32

# Evaluate with MusDr
python -m src.evaluate --input output.json
```

---

## C++ JUCE 獨立應用程式 (2025-01 完成)

### 開發狀態: 完成

將 Python 實作重寫為 JUCE C++ 獨立應用程式，具備 GUI、內建合成器與 MIDI 輸出。

### C++ 專案結構

```
cpp/
├── Source/
│   ├── Core/
│   │   ├── PitchClass.h           # 音高類 (0-11)
│   │   ├── ChordSymbol.h/cpp      # 和弦符號
│   │   └── ChordQuality.h         # 和弦品質 enum
│   │
│   ├── Grammar/
│   │   ├── NonTerminal.h          # 非終端符號
│   │   ├── GrammarRule.h          # 文法規則
│   │   ├── PCFG.h/cpp             # 機率文法
│   │   └── Generator.h/cpp        # 和聲生成器
│   │
│   ├── Substitution/
│   │   ├── TritoneSubstitution.h  # 三全音替代
│   │   ├── BackdoorSubstitution.h # Backdoor dominant
│   │   ├── ColtraneSubstitution.h # Coltrane changes
│   │   └── SubstitutionEngine.h   # 統一替代引擎
│   │
│   ├── Style/
│   │   ├── StyleVector.h          # 15 參數風格向量
│   │   ├── StylePresets.h         # 9 種風格預設
│   │   └── StyleEngine.h/cpp      # 風格引擎
│   │
│   ├── VoiceLeading/
│   │   ├── GuideTone.h            # Guide-tone 分析
│   │   └── VoiceLeadingOptimizer.h
│   │
│   ├── Synthesis/
│   │   └── ChordSynth.h           # 和弦合成器 (8 複音)
│   │
│   ├── MIDI/
│   │   └── MIDIExporter.h         # MIDI 檔案匯出
│   │
│   ├── Main.cpp
│   ├── MainComponent.h
│   └── MainComponent.cpp
│
├── JUCE/                          # JUCE 子模組
├── CMakeLists.txt
└── build/
    └── JazzArchitect_artefacts/
        ├── Debug/Jazz Architect.app
        └── Release/Jazz Architect.app
```

### 開發階段進度

| 階段 | 狀態 | 說明 |
|------|------|------|
| Phase 1: 專案設置 | 完成 | CMake + JUCE 7.x |
| Phase 1: 核心類別 | 完成 | PitchClass, ChordQuality, ChordSymbol |
| Phase 1: GUI 框架 | 完成 | Techno Machine 風格配色 |
| Phase 2: 文法引擎 | 完成 | PCFG + HarmonyGenerator |
| Phase 3: 風格系統 | 完成 | 9 種預設 + StyleEngine |
| Phase 4: 替代規則 | 完成 | Tritone, Backdoor, Coltrane |
| Phase 5: 聲部導進 | 完成 | GuideTone + Optimizer |
| Phase 6: 音訊引擎 | 完成 | ChordSynth (3 音色) |
| Phase 7: MIDI 輸出 | 完成 | MIDIExporter |
| Phase 8: UI 完善 | 完成 | 合成器選擇器 |
| Phase 9: 打包發布 | 完成 | Release build (11MB) |

### 功能清單

- **和聲生成**: Rohrmeier (2020) PCFG 遞迴文法
- **9 種風格**: Bebop, Cool, Modal, Hard Bop, Post-Bop, Swing, Fusion, Contemporary, Blues
- **替代規則**: 三全音替代、Backdoor dominant、Coltrane changes
- **聲部導進**: Smither (2019) guide-tone 優化
- **音訊合成**: Electric Piano, Organ, Pad (8 複音)
- **MIDI 輸出**: Standard MIDI File 匯出
- **即時控制**: 風格參數滑桿、BPM、長度

### 編譯指令

```bash
cd cpp
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Release 位置

```
cpp/build/JazzArchitect_artefacts/Release/Jazz Architect.app
```

---

## 文件進度 (2026-01-02)

### 學術論文

已完成三語版本論文：

| 版本 | 檔案 | 狀態 |
|------|------|------|
| 繁體中文 | `files/JazzArchitect_Paper.md` | 完成 |
| English | `files/JazzArchitect_Paper_en.md` | 完成 |
| 日本語 | `files/JazzArchitect_Paper_ja.md` | 完成 |

論文內容涵蓋：
- 系統架構與模組設計
- PCFG 文法引擎
- 15 參數風格向量
- 替代規則實作
- 聲部導進優化
- 音訊合成與 MIDI 輸出

---

## 下一階段：UI 改進與測試

### 待處理項目

| 項目 | 狀態 | 說明 |
|------|------|------|
| Layout 修改 | 完成 | 改善 UI 配置 |
| 功能測試 | 待進行 | 測試生成、播放、MIDI 匯出 |
| Bug 修正 | 待進行 | 修正發現的問題 |

---

## UI Layout 偏好設定

### 垂直間距原則

| 區域 | 位置 | 說明 |
|------|------|------|
| 標題 (JAZZ ARCHITECT) | 頂部 | 粉紅色大標題 |
| 第一排控制項 | ~55px | Style, Key, BPM, Length, Sound |
| 第二排按鈕 | ~95px | Generate, Play, Stop, Export MIDI |
| 分隔線 | 165px | 控制區與顯示區的分界 |
| 標籤 (CHORD PROGRESSION / STYLE PARAMETERS) | 180px | 分隔線下方 15px |
| 內容起始 (和弦盒子 / 滑桿) | 205-210px | 標籤下方 25-30px |
| 狀態列 | 對齊滑桿數字 | Y 軸與右側滑桿下方數字對齊 |

### 水平間距原則

- 控制項間距：12px
- 按鈕間距：10px
- 標籤寬度需足夠容納文字，避免截斷

### 重要注意事項

1. 分隔線以下內容需與分隔線保持 15-20px 間距
2. 狀態列使用計算方式對齊，而非固定底部距離
3. 和弦進行顯示區域需檢查右側邊界，避免與滑桿重疊
4. 所有標籤文字需有足夠空間，避免重疊
