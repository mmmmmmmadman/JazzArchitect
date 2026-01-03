# Phase 2: 語料庫分析 — 研究報告

## 1. 可用語料庫總覽

### 1.1 Jazz Harmony Treebank (JHT)
- **來源**: DCML/EPFL (Harasim et al., 2020)
- **規模**: 150 Jazz Standards 完整和聲樹分析
- **格式**: JSON (open_constituent_tree + complete_constituent_tree)
- **標註**: prolongation/preparation 依賴關係, 形式結構
- **取得**: https://github.com/DCMLab/JazzHarmonyTreebank
- **授權**: CC BY 4.0

### 1.2 iRealPro Corpus (Broze & Shanahan)
- **來源**: Shanahan et al., Zenodo (2019)
- **規模**: 1,086 Jazz compositions (1924-1968)
- **格式**: 和弦符號序列
- **用途**: 風格歷時變化分析
- **發現**: P4 根音移動 1950s 驟降, 調性和聲逐漸減少

### 1.3 Weimar Jazz Database (WJD)
- **來源**: Jazzomat Project, Hochschule für Musik "Franz Liszt" Weimar
- **規模**: 456 solo transcriptions (1925-2009)
- **內容**: 289,000+ 音符事件, 含節拍/和弦/樂句標註
- **格式**: Sonic Visualiser 專案 + PostgreSQL
- **取得**: http://jazzomat.hfm-weimar.de
- **參考**: Pfleiderer et al. (2017) *Inside the Jazzomat*

### 1.4 Audio-Aligned Jazz Harmony Dataset
- **來源**: Eremenko et al., ISMIR 2018
- **規模**: 113 tracks (Smithsonian Collections)
- **格式**: .lab files (Harte notation)
- **特色**: 時間對齊的和弦標註

---

## 2. 語料庫分析目標與方法

### 2.1 風格特定進行模式萃取

| 風格 | 時期 | 特徵進行 |
|------|------|----------|
| Bebop | 1945-1955 | ii-V chains, tritone subs, bird changes |
| Cool | 1950s | Modal interchange, quartal voicings |
| Modal | 1959+ | Static harmony, sus4 structures |
| Post-bop | 1960s+ | Non-functional progressions, side-slipping |

### 2.2 統計分析指標

**和弦層級**
- 和弦品質分布 (maj7, min7, dom7, dim, aug, etc.)
- 根音移動類型頻率 (P4, P5, m2, M2, m3, M3, tritone)
- 和弦持續時間分布

**進行層級**
- ii-V-I 出現頻率與變體
- Tritone substitution 上下文
- 轉調模式 (相對頻率: ii, IV, V, ♭VII, ♭III)

**形式層級**
- AABA vs. ABAC 分布
- Turnaround 長度與類型
- Bridge 和聲特徵

### 2.3 調性模糊度量化

基於 Broze & Shanahan (2013) 的發現:
- 0階和弦品質分布熵
- 1階轉移概率矩陣稀疏度
- 功能和聲比例 (T-S-D 可解釋性)

---

## 3. 學術可信度驗證

### 3.1 語料庫相關文獻驗證

| 文獻 | 出版資訊 | 驗證狀態 |
|------|----------|----------|
| Harasim et al. (2020) | ISMIR 2020, pp. 207-215 | ✓ 確認 |
| Broze & Shanahan (2013) | Music Perception 31(1):32-45 | ✓ 確認 |
| Pfleiderer et al. (2017) | *Inside the Jazzomat*, Schott | ✓ 確認 |
| Eremenko et al. (2018) | ISMIR 2018, pp. 483-490 | ✓ 確認 |
| Shanahan & Broze (2012) | ICMPC 12, pp. 909-917 | ✓ 確認 |
| Frieler et al. (2018) | ISMIR 2018, pp. 777-783 | ✓ 確認 |
| Katz (2017) | Music Perception 35(2):165-192 | ✓ 確認 |

### 3.2 資料可取得性確認

| 語料庫 | URL | 狀態 |
|--------|-----|------|
| JHT | github.com/DCMLab/JazzHarmonyTreebank | ✓ 公開 |
| iRealPro | zenodo.org (Shanahan) | ✓ 公開 |
| WJD | jazzomat.hfm-weimar.de | ✓ 公開 |

---

## 4. 規則萃取策略

### 4.1 從 JHT 萃取文法規則

```python
# 偽代碼: 從樹分析萃取產生規則
def extract_rules(tree):
    if tree.is_leaf():
        return []
    
    rule = (tree.label, 
            [child.label for child in tree.children],
            tree.relation_type)  # prolongation or preparation
    
    return [rule] + sum([extract_rules(c) for c in tree.children], [])
```

### 4.2 規則概率估計

- 使用 MLE 從語料庫計算 PCFG 參數
- 參考 Harasim et al. (2018) PACFG 框架
- 考慮 key context 條件概率

---

## Phase 2 完成狀態: ✓

### 關鍵發現
1. JHT 是目前唯一含完整階層標註的爵士和聲語料庫
2. Broze & Shanahan 資料適合歷時風格分析
3. WJD 主要用於旋律研究，但含和弦序列可交叉驗證
4. 建議主要使用 JHT (150 standards) + iRealPro (1,086 pieces) 組合

**下一步**: Phase 3 規則系統設計
