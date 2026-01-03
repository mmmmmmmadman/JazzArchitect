# Phase 4: 驗證與評估 — 研究報告

## 1. 評估框架

### 1.1 客觀指標 (Wu & Yang 2020, MusDr)

| 指標 | 說明 | 目標 |
|------|------|------|
| **H** (Pitch-Class Histogram Entropy) | 音高使用多樣性 | 接近訓練集分布 |
| **GS** (Grooving Pattern Similarity) | 節奏一致性 | 高值 = 好 |
| **CPI** (Chord Progression Irregularity) | 和聲進行一致性 | 低值 = 好 |
| **SI** (Structureness Indicator) | 結構重複性 | 接近真實曲目 |

### 1.2 主觀評估維度

基於 Yang & Lerch (2020) 框架:
- **Stylistic success**: 風格辨識度
- **Harmonic consistency**: 和聲合理性
- **Melodic quality**: 旋律流暢度
- **Structural coherence**: 形式完整性
- **Overall preference**: 整體偏好

### 1.3 文法特定指標

| 指標 | 計算方式 |
|------|----------|
| **Grammar coverage** | 生成進行被文法接受的比例 |
| **Parse ambiguity** | 平均可能樹數量 |
| **Perplexity** | 負對數似然 (PCFG) |
| **ii-V-I accuracy** | 正確解析 ii-V-I 的比例 |

---

## 2. 基準比較

### 2.1 對比模型

| 模型 | 類型 | 參考 |
|------|------|------|
| Jazz Transformer | Neural (Transformer-XL) | Wu & Yang 2020 |
| Markov (n-gram) | Statistical | Baseline |
| LSTM | Neural (RNN) | Baseline |
| **Proposed PCFG** | Grammar-based | 本研究 |

### 2.2 預期優勢

| 維度 | Grammar-based | Neural |
|------|---------------|--------|
| 可解釋性 | ✓ 規則可檢視 | ✗ 黑盒 |
| 資料效率 | ✓ 小語料可行 | ✗ 需大量資料 |
| 長程結構 | ✓ 形式編碼 | △ 有限 |
| 風格控制 | ✓ 參數化 | △ 需微調 |
| 細節多樣性 | △ 有限 | ✓ 較佳 |

---

## 3. 驗證資料集

### 3.1 訓練/測試分割

| 語料庫 | 訓練 | 測試 | 用途 |
|--------|------|------|------|
| JHT | 120 | 30 | 樹結構學習 |
| iRealPro | 870 | 216 | 進行模式驗證 |

### 3.2 交叉驗證

- 5-fold cross-validation on JHT
- Leave-one-out for rare progressions

---

## 4. 學術可信度驗證

### 4.1 Phase 4 新增文獻

| 文獻 | 出版資訊 | 驗證狀態 |
|------|----------|----------|
| Wu & Yang (2020) | ISMIR 2020, pp. 142-149 | ✓ 確認 |
| Yang & Lerch (2020) | Neural Computing and Applications 32:4773-4784 | ✓ 確認 |
| MusDr toolkit | github.com/slSeanWU/MusDr | ✓ 公開 |

### 4.2 評估工具

| 工具 | 說明 | 取得 |
|------|------|------|
| MusDr | 符號音樂客觀評估 | GitHub |
| JHT utilities | 樹結構處理 | GitHub |
| music21 | 音樂分析 | pip install |

---

## Phase 4 完成狀態: ✓

### 建議驗證流程

1. 從 JHT 萃取規則並計算 PCFG 參數
2. 生成 100 首 32-bar 進行
3. 計算 H, GS, CPI, SI 指標
4. 與 Jazz Transformer 輸出比較
5. 小規模聽眾測試 (n=20, 音樂背景)
6. 分析失敗案例，迭代改進規則
