# Jazz Architect 開發進度規劃

## 已完成功能 (v1.0)

### 核心系統
- [x] PCFG 遞迴文法引擎 (Rohrmeier 2020)
- [x] 9 種爵士風格預設 (Bebop, Cool, Modal, Hard Bop, Post-Bop, Swing, Fusion, Contemporary, Blues)
- [x] 15 參數風格向量
- [x] 和弦進行生成

### 替代規則
- [x] 三全音替代 (Tritone Substitution)
- [x] Backdoor Dominant
- [x] Coltrane Changes

### 聲部導進
- [x] Guide-tone 優化 (Smither 2019)
- [x] Voice Leading Optimizer

### 音訊與 MIDI
- [x] ChordSynth 合成器 (Electric Piano, Organ, Pad)
- [x] 8 複音支援
- [x] MIDI 檔案匯出
- [x] MIDI 檔案匯入 + 自動和弦辨識
- [x] 樂譜影像匯入 (OMR) + 自動和弦辨識

### UI 功能
- [x] 和弦進行視覺化 (五線譜 + 和弦符號)
- [x] 五線譜升降記號顯示
- [x] 和弦長度自訂 (拖曳調整)
- [x] 音符拖曳編輯 + 和弦名稱自動更新
- [x] 即時播放與停止
- [x] 風格參數滑桿 (Tritone, ii-V, Modal, Extension)
- [x] 音訊輸出裝置選擇

---

## 計畫中功能

### Phase 2: Bassline 生成系統

**狀態**: 研究完成，待實作
**研究文件**: `/files/JazzBassline_Research.md`

#### 功能規劃

| 功能 | 說明 | 優先級 |
|------|------|--------|
| BasslineGenerator | 主生成器類別 | 高 |
| BassStyleVector | 10 參數風格向量 | 高 |
| Walking Bass | 四拍行進低音 | 高 |
| Two-Feel | 二拍感 (抒情曲) | 中 |
| Bossa Nova | Latin 節奏型態 | 中 |
| Tumbao | Afro-Cuban 型態 | 低 |

#### 技術要點

1. **節拍功能分配**
   - Beat 1: ROOT (和聲確立)
   - Beat 2: CHORD_TONE / SCALE_TONE
   - Beat 3: CHORD_TONE (5th 優先)
   - Beat 4: APPROACH (接近音)

2. **接近音類型**
   - 半音接近 (Chromatic)
   - 全音接近 (Diatonic)
   - 五度接近 (Fifth)
   - 包圍接近 (Enclosure)

3. **風格預設**
   - SWING_WALKING
   - BEBOP
   - BOSSA_NOVA
   - BALLAD

#### 檔案結構

```
Source/Bassline/
├── BasslineGenerator.h
├── BassStyleVector.h
├── BassStylePresets.h
├── BeatAllocator.h
├── NoteSelector.h
├── ApproachNotes.h
├── LatinPatterns.h
└── BassVoiceLeader.h
```

#### 學術參考

- Dias et al. (2013) - Contour-Based Walking Bass Generator
- Shiga & Kitahara (2021) - HMM Walking Bass Generation
- FiloBass Dataset (2023) - 48 首爵士低音轉錄

---

### Phase 3: 鼓組生成系統 (規劃中)

**狀態**: 未開始

#### 預計功能
- Swing 鼓組型態
- Latin 鼓組型態
- Brush 技巧模擬
- 與 Bassline 的節奏互動

---

### Phase 4: 進階功能 (構想中)

| 功能 | 說明 |
|------|------|
| 即時 MIDI 輸入 | 接收外部 MIDI 鍵盤 |
| DAW 插件版本 | VST3/AU 格式 |
| 樂譜匯出 | MusicXML / PDF |
| 機器學習增強 | 學習真實演奏風格 |

---

## 版本紀錄

| 版本 | 日期 | 內容 |
|------|------|------|
| v1.0 | 2026-01-02 | 初始發布：和弦生成、合成器、MIDI 匯出 |
| v1.1 | 2026-01-03 | 和弦長度自訂、MIDI 匯入、升降記號、音符編輯 |
| v1.2 | 2026-01-03 | 樂譜影像匯入 (OMR)，使用 oemer 開源引擎 + music21 |

### OMR 技術細節

- **oemer**: MIT 授權 OMR 引擎 (需修補 NumPy 2.x 相容性)
- **music21**: MIT 授權，MusicXML → MIDI 轉換
- **Python 環境**: `/omr_venv/` (Python 3.11)
- **依賴版本**: numpy 1.24.4, scipy 1.11.4, scikit-learn 1.3.2

---

## GitHub Repository

https://github.com/mmmmmmmadman/JazzArchitect

---

*最後更新: 2026-01-03*
