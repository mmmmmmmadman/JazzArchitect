# 爵士和聲規則系統：學術文獻研究報告

**完成日期**: 2026-01-01

---

## 執行摘要

本報告完成 4 階段研究計畫，為規則式爵士和聲進行生成系統建立學術基礎。所有核心文獻已驗證，發現 2 處需修正引用，並整理出完整的理論框架、語料庫資源、規則設計方案與評估方法。

---

## 已驗證核心文獻

### 理論框架
| 文獻 | 出版資訊 | 內容 |
|------|----------|------|
| Rohrmeier (2020) | Music Theory and Analysis 7(1):1-62 | 遞歸和聲文法 |
| Steedman (1984) | Music Perception 2(1):52-77 | 爵士和弦序列生成文法 |
| Tymoczko (2006) | Science 313:72-74 | 和弦幾何空間 |
| Cecchetti et al. **(2022)** | Musicae Scientiae, DOI:10.1177/10298649221122245 | OEC 感知實驗 |
| Smither (2019) | Music Theory Online 25(2) | Guide-tone 聲部導進 |

### 語料庫
| 資源 | 規模 | 格式 |
|------|------|------|
| Jazz Harmony Treebank | 150 standards | JSON (樹結構) |
| iRealPro Corpus | 1,086 pieces | 和弦符號 |
| Weimar Jazz Database | 456 solos | 音符事件 |

### 計算方法
| 文獻 | 內容 |
|------|------|
| Harasim et al. (2018, 2020) | PACFG 框架 + JHT |
| Wu & Yang (2020) | MusDr 評估指標 |
| Granroth-Wilding & Steedman (2014) | Parser 實作 |

---

## 需修正引用

1. **Cecchetti et al.**: 2023 → **2022** (線上發表日期)
2. 前次會話已確認: Yin et al. 作者順序錯誤

---

## 規則系統架構

### 核心組件
```
┌──────────────────────────────────────┐
│         Functional Grammar           │
│   (Prolongation + Preparation)       │
├──────────────────────────────────────┤
│      Substitution Rule Module        │
│  (Tritone, Backdoor, Coltrane, etc) │
├──────────────────────────────────────┤
│   Voice Leading Constraints          │
│      (Guide-tone optimization)       │
├──────────────────────────────────────┤
│      Style Parameter Controller      │
│   (Bebop, Cool, Modal, Post-bop)     │
└──────────────────────────────────────┘
```

### 文法核心規則
- **Prolongation**: X → X X (延長)
- **Preparation**: X → V/X X (應用屬和弦)
- **Substitution**: V → ♭II7 (三全音替代)

---

## 評估方案

### 客觀指標 (MusDr)
- H: Pitch-class entropy
- GS: Grooving similarity  
- CPI: Chord progression irregularity
- SI: Structureness indicator

### 基準比較
- Jazz Transformer (Wu & Yang 2020)
- Markov n-gram baseline
- LSTM baseline

---

## 關鍵資源連結

| 資源 | URL |
|------|-----|
| JHT | github.com/DCMLab/JazzHarmonyTreebank |
| MusDr | github.com/slSeanWU/MusDr |
| Jazzomat/WJD | jazzomat.hfm-weimar.de |
| PACFG code | zenodo.org/records/1492367 |
| Rohrmeier PDF | music.arts.uci.edu/abauer/4.3/readings/Rohrmeier_Syntax_Jazz_Harmony.pdf |

---

## 下一步建議

1. **實作**: 用 Python 實作 PCFG，從 JHT 萃取規則權重
2. **測試**: 生成進行並用 MusDr 評估
3. **比較**: 與 Jazz Transformer 輸出做 A/B 測試
4. **迭代**: 分析失敗案例，調整規則

---

## 附件

- phase1_theoretical_foundations.md
- phase2_corpus_analysis.md  
- phase3_rule_system_design.md
- phase4_validation.md
