# Phase 3: 規則系統設計 — 研究報告

## 1. 核心架構

### 1.1 系統模組

```
┌─────────────────────────────────────────────────────────┐
│                  Jazz Harmony Generator                  │
├─────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐ │
│  │  Functional │  │ Substitution│  │  Voice Leading  │ │
│  │   Grammar   │  │    Rules    │  │   Constraints   │ │
│  │   Engine    │  │    Module   │  │    (Smither)    │ │
│  └──────┬──────┘  └──────┬──────┘  └────────┬────────┘ │
│         │                │                   │          │
│         └────────────────┼───────────────────┘          │
│                          ▼                              │
│              ┌─────────────────────┐                    │
│              │   Style Parameter   │                    │
│              │      Controller     │                    │
│              └──────────┬──────────┘                    │
│                         ▼                               │
│              ┌─────────────────────┐                    │
│              │  Rhythm/Meter       │                    │
│              │  Integration        │                    │
│              │  (Harasim 2019)     │                    │
│              └─────────────────────┘                    │
└─────────────────────────────────────────────────────────┘
```

---

## 2. 功能文法引擎 (PCFG/PACFG)

### 2.1 非終端符號

```
N = {S, T, D, SD, Phrase, Prol, Prep}

S   : Start symbol (entire piece)
T   : Tonic constituent  
D   : Dominant constituent
SD  : Subdominant constituent
Prol: Prolongation
Prep: Preparation
```

### 2.2 核心產生規則

**Top-level structure**
```
S → T D T                    # AABA form: I - V - I
S → T                        # Single-part form
```

**Tonic rules**
```
T → I                        # Terminal
T → T Prol                   # Right prolongation
T → Prol T                   # Left prolongation  
T → SD T                     # Plagal preparation (Blues)
T → D T                      # Authentic preparation
```

**Dominant rules**
```
D → V                        # Terminal (V7, V9, etc.)
D → Prep D                   # Preparation chain
D → D Prol                   # Dominant prolongation
```

**Preparation rules**
```
Prep → ii                    # Diatonic supertonic
Prep → V/V                   # Secondary dominant
Prep → IV                    # Subdominant approach
Prep → Prep Prep             # Chained preparations
```

**Substitution (unary)**
```
V → ♭II7                     # Tritone substitution
V → ♭VII7                    # Backdoor dominant
I → ♭III                     # Relative major (minor context)
```

### 2.3 Key Feature 處理

依照 Rohrmeier (2020) 規範:
```
V/X_key=Y → V_key=X/Y        # Applied dominant key scope
V/X_key=Y → V_key=♭V/X/Y     # Tritone sub with key change
X_key=Y → X_key=inv(Y)       # Borrowing (mode change)
```

---

## 3. 替代規則庫

### 3.1 Tritone Substitutions

| Original | Substitution | Context |
|----------|--------------|---------|
| V7 → I   | ♭II7 → I    | Root ↓m2 |
| ii7 → V7 | ♭vi7 → ♭II7 | ii-V both |
| V/V → V  | ♭VI7 → V    | Secondary |

### 3.2 Backdoor Resolution
```
♭VII7 → I     # iv借用的屬七 (F7 → Cmaj)
iv → ♭VII7 → I  # 完整 backdoor
```

### 3.3 Coltrane Changes
```
I → III7 → VI7 → II7 → V7 → I
# 大三度循環: C → E7 → A♭7 → D♭7(B7) → G7 → C
```

### 3.4 Deceptive Resolutions
```
V7 → vi      # Standard deceptive
V7 → ♭VI     # Modal deceptive
V7 → IV      # Plagal avoidance
```

---

## 4. 聲部導進約束 (Smither 2019)

### 4.1 Guide-Tone Space

**Guide tones** = 和弦的 3rd 和 7th
- 定義和弦品質的最小集合
- 聲部導進的核心線

### 4.2 Voice Leading Transformations

| Progression | 3rd movement | 7th movement |
|-------------|--------------|--------------|
| ii7 → V7    | stays (→3rd) | ↓m2 (→7th)   |
| V7 → Imaj7  | ↓m2 (→7th)   | stays (→3rd) |
| V7 → ♭II7   | ↑m2          | stays        |

### 4.3 約束規則
```python
def voice_leading_cost(chord1, chord2):
    gt1 = (chord1.third, chord1.seventh)
    gt2 = (chord2.third, chord2.seventh)
    return sum(min_semitone_distance(a, b) 
               for a, b in zip(gt1, gt2))

# 優先選擇 cost ≤ 2 的進行
```

---

## 5. 風格參數化

### 5.1 風格向量定義

```python
StyleVector = {
    'tritone_sub_prob': float,      # 0.0-1.0
    'ii_v_preference': float,       # 偏好 ii-V vs V
    'modal_interchange': float,     # 借用頻率
    'chromatic_approach': float,    # 半音鄰接頻率
    'dominant_chain_depth': int,    # 最大遞歸深度
    'rhythm_density': float         # 和弦變換頻率
}

# 預設風格
BEBOP = StyleVector(0.3, 0.9, 0.2, 0.4, 4, 0.8)
COOL = StyleVector(0.2, 0.7, 0.3, 0.2, 3, 0.5)
MODAL = StyleVector(0.1, 0.3, 0.5, 0.1, 2, 0.3)
POSTBOP = StyleVector(0.4, 0.6, 0.6, 0.5, 5, 0.6)
```

### 5.2 參數到規則概率映射

```python
def style_to_pcfg(style: StyleVector) -> PCFG:
    """將風格向量轉換為 PCFG 規則概率"""
    rules = base_grammar.copy()
    
    # 調整替代規則概率
    rules['V → ♭II7'].prob = style.tritone_sub_prob
    rules['V → V'].prob = 1 - style.tritone_sub_prob
    
    # 調整 preparation 偏好
    rules['Prep → ii'].prob = style.ii_v_preference * 0.5
    rules['Prep → IV'].prob = (1 - style.ii_v_preference) * 0.3
    
    return normalize_pcfg(rules)
```

---

## 6. 學術可信度驗證

### 6.1 Phase 3 新增文獻驗證

| 文獻 | 出版資訊 | 驗證狀態 |
|------|----------|----------|
| Smither (2019) | Music Theory Online 25(2) | ✓ 確認 |
| Harasim (2020) | EPFL PhD Thesis | ✓ 確認 |
| McClimon (2017) | Music Theory Online 23(1) | ✓ 已驗證 (Phase 1) |
| Waters & Williams (2010) | Music Theory Online 16(3) | ✓ 已驗證 (prior) |

### 6.2 實作參考

| 資源 | 說明 |
|------|------|
| PACFG implementation | Harasim et al., Zenodo DOI:10.5281/zenodo.1492367 |
| Tree annotation app | https://dcmlab.github.io/tree-annotation-code/ |
| MeloSpySuite | jazzomat.hfm-weimar.de (Python) |

---

## Phase 3 完成狀態: ✓

### 待解決問題 (Phase 4 驗證)
1. PCFG 權重最佳化方法選擇 (MLE vs. Variational Bayes)
2. Post-bop 非功能和聲的處理策略
3. 節奏整合的時間粒度選擇

**下一步**: Phase 4 驗證與評估
