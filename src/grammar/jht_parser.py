"""Jazz Harmony Treebank Parser - Extract rule statistics from JHT"""

from __future__ import annotations
import json
from collections import Counter, defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Any
import re


@dataclass
class TreeNode:
    """Node in a harmonic analysis tree"""
    label: str
    children: List[TreeNode]
    is_implicit: bool = False  # Marked with * in JHT

    @classmethod
    def from_dict(cls, d: Dict) -> TreeNode:
        """Create TreeNode from JHT JSON structure"""
        label = d.get('label', '')
        is_implicit = label.endswith('*')
        if is_implicit:
            label = label[:-1]

        children = [cls.from_dict(c) for c in d.get('children', [])]
        return cls(label=label, children=children, is_implicit=is_implicit)

    def is_leaf(self) -> bool:
        return len(self.children) == 0

    def depth(self) -> int:
        if self.is_leaf():
            return 0
        return 1 + max(c.depth() for c in self.children)

    def leaf_sequence(self) -> List[str]:
        """Get chord sequence from leaves"""
        if self.is_leaf():
            return [self.label]
        result = []
        for c in self.children:
            result.extend(c.leaf_sequence())
        return result


@dataclass
class RuleStats:
    """Statistics about grammar rules extracted from JHT"""
    binary_rules: Counter  # (parent_type, left_type, right_type) -> count
    unary_rules: Counter   # (parent_type, child_type) -> count
    terminal_rules: Counter  # chord_type -> count
    progressions: Counter  # (chord1, chord2) -> count
    cadences: Counter      # cadence pattern -> count


def parse_chord_type(chord: str) -> str:
    """Extract chord type/quality from chord symbol"""
    # Remove root note
    match = re.match(r'^[A-G][#b]?(.*)$', chord)
    if not match:
        return chord
    quality = match.group(1)

    # Normalize quality
    quality_map = {
        '': 'maj',
        '^7': 'maj7',
        '7': 'dom7',
        'm7': 'min7',
        'm': 'min',
        'm7b5': 'hdim7',
        '%7': 'hdim7',
        '%': 'hdim7',
        'o7': 'dim7',
        'o': 'dim',
        '+': 'aug',
        'sus': 'sus4',
        '6': 'maj6',
        'm6': 'min6',
    }

    return quality_map.get(quality, quality)


def extract_root(chord: str) -> Optional[str]:
    """Extract root note from chord symbol"""
    match = re.match(r'^([A-G][#b]?)', chord)
    return match.group(1) if match else None


def classify_function(chord: str, key: str) -> str:
    """Classify chord function relative to key"""
    root = extract_root(chord)
    key_root = extract_root(key) or 'C'
    quality = parse_chord_type(chord)

    if not root:
        return 'unknown'

    # Simple pitch class mapping
    pc_map = {'C': 0, 'D': 2, 'E': 4, 'F': 5, 'G': 7, 'A': 9, 'B': 11}

    def to_pc(note: str) -> int:
        base = pc_map.get(note[0], 0)
        if len(note) > 1:
            if note[1] == '#':
                base += 1
            elif note[1] == 'b':
                base -= 1
        return base % 12

    root_pc = to_pc(root)
    key_pc = to_pc(key_root)
    interval = (root_pc - key_pc) % 12

    # Functional classification
    if interval == 0 and quality in ('maj', 'maj7', 'maj6'):
        return 'T'  # Tonic
    elif interval == 7 and quality in ('dom7', '7', 'sus4'):
        return 'D'  # Dominant
    elif interval == 2 and quality in ('min7', 'min'):
        return 'SD'  # Subdominant (ii)
    elif interval == 5 and quality in ('maj', 'maj7'):
        return 'SD'  # Subdominant (IV)
    elif interval == 9 and quality in ('min7', 'min'):
        return 'T'  # Tonic substitute (vi)
    elif interval == 4 and quality in ('min7', 'min'):
        return 'T'  # Tonic substitute (iii)
    elif interval == 11 and quality in ('hdim7', 'dim7'):
        return 'D'  # Dominant function (vii)
    elif quality in ('dom7', '7'):
        return 'D'  # Secondary dominant
    else:
        return 'other'


def extract_rules_from_tree(node: TreeNode, key: str, stats: RuleStats):
    """Recursively extract rules from a tree"""
    if node.is_leaf():
        # Terminal rule
        chord_type = parse_chord_type(node.label)
        stats.terminal_rules[chord_type] += 1
        return

    if len(node.children) == 2:
        # Binary rule
        left = node.children[0]
        right = node.children[1]

        parent_func = classify_function(node.label, key)
        left_func = classify_function(left.label, key)
        right_func = classify_function(right.label, key)

        stats.binary_rules[(parent_func, left_func, right_func)] += 1

        # Track progressions between leaves
        left_leaves = left.leaf_sequence()
        right_leaves = right.leaf_sequence()
        if left_leaves and right_leaves:
            last_left = parse_chord_type(left_leaves[-1])
            first_right = parse_chord_type(right_leaves[0])
            stats.progressions[(last_left, first_right)] += 1

        # Recurse
        extract_rules_from_tree(left, key, stats)
        extract_rules_from_tree(right, key, stats)

    elif len(node.children) == 1:
        # Unary rule (substitution)
        child = node.children[0]
        parent_func = classify_function(node.label, key)
        child_func = classify_function(child.label, key)
        stats.unary_rules[(parent_func, child_func)] += 1
        extract_rules_from_tree(child, key, stats)


def detect_cadences(chords: List[str], key: str) -> List[str]:
    """Detect cadence patterns in chord sequence"""
    cadences = []
    n = len(chords)

    for i in range(n - 1):
        func1 = classify_function(chords[i], key)
        func2 = classify_function(chords[i + 1] if i + 1 < n else '', key)

        if func1 == 'D' and func2 == 'T':
            # Check for ii-V-I
            if i > 0 and classify_function(chords[i - 1], key) == 'SD':
                cadences.append('ii-V-I')
            else:
                cadences.append('V-I')
        elif func1 == 'SD' and func2 == 'T':
            cadences.append('IV-I')

    return cadences


def load_jht(path: str = 'treebank.json') -> List[Dict]:
    """Load Jazz Harmony Treebank"""
    with open(path, 'r') as f:
        return json.load(f)


def analyze_treebank(treebank_path: str) -> RuleStats:
    """Analyze entire treebank and extract rule statistics"""
    data = load_jht(treebank_path)

    stats = RuleStats(
        binary_rules=Counter(),
        unary_rules=Counter(),
        terminal_rules=Counter(),
        progressions=Counter(),
        cadences=Counter()
    )

    analyzed = 0
    for item in data:
        if 'trees' not in item or not item['trees']:
            continue

        key = item.get('key', 'C')
        chords = item.get('chords', [])

        # Analyze tree structure
        for tree_data in item['trees']:
            if 'open_constituent_tree' in tree_data:
                tree = TreeNode.from_dict(tree_data['open_constituent_tree'])
                extract_rules_from_tree(tree, key, stats)
                analyzed += 1

        # Detect cadences
        for cadence in detect_cadences(chords, key):
            stats.cadences[cadence] += 1

    print(f"Analyzed {analyzed} trees from {len(data)} items")
    return stats


def compute_rule_probabilities(stats: RuleStats) -> Dict[str, float]:
    """Convert counts to probabilities"""
    probs = {}

    # Binary rules
    binary_total = sum(stats.binary_rules.values())
    if binary_total > 0:
        for rule, count in stats.binary_rules.most_common():
            probs[f"binary:{rule}"] = count / binary_total

    # Terminal rules
    term_total = sum(stats.terminal_rules.values())
    if term_total > 0:
        for chord_type, count in stats.terminal_rules.most_common():
            probs[f"terminal:{chord_type}"] = count / term_total

    # Progressions
    prog_total = sum(stats.progressions.values())
    if prog_total > 0:
        for prog, count in stats.progressions.most_common(20):
            probs[f"progression:{prog}"] = count / prog_total

    # Cadences
    cad_total = sum(stats.cadences.values())
    if cad_total > 0:
        for cad, count in stats.cadences.most_common():
            probs[f"cadence:{cad}"] = count / cad_total

    return probs


def print_statistics(stats: RuleStats):
    """Print analysis statistics"""
    print("\n=== Binary Rules (top 15) ===")
    for rule, count in stats.binary_rules.most_common(15):
        print(f"  {rule}: {count}")

    print("\n=== Terminal Rules (top 10) ===")
    for chord, count in stats.terminal_rules.most_common(10):
        print(f"  {chord}: {count}")

    print("\n=== Progressions (top 15) ===")
    for prog, count in stats.progressions.most_common(15):
        print(f"  {prog[0]} -> {prog[1]}: {count}")

    print("\n=== Cadences ===")
    for cad, count in stats.cadences.most_common():
        print(f"  {cad}: {count}")


if __name__ == '__main__':
    import sys

    path = sys.argv[1] if len(sys.argv) > 1 else 'treebank.json'
    stats = analyze_treebank(path)
    print_statistics(stats)

    print("\n=== Rule Probabilities (sample) ===")
    probs = compute_rule_probabilities(stats)
    for k, v in list(probs.items())[:20]:
        print(f"  {k}: {v:.4f}")
